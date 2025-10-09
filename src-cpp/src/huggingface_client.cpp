#include "huggingface_client.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QUrl>
#include <QDebug>
#include <QTimer>
#include <QCoreApplication>

HuggingFaceClient::HuggingFaceClient(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
}

HuggingFaceClient::~HuggingFaceClient()
{
    // Clean up any active downloads
    for (auto it = m_activeDownloads.begin(); it != m_activeDownloads.end(); ++it) {
        if (it->reply) {
            it->reply->deleteLater();
        }
        if (it->progressTimer) {
            it->progressTimer->deleteLater();
        }
    }
}

QJsonArray HuggingFaceClient::getAvailableModels()
{
    QJsonArray models;
    
    // Determine candidate locations for llms.txt/models.txt
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString cwdDir = QDir::currentPath();
    const QStringList candidates = {
        appDir + "/llms.txt",
        appDir + "/models.txt",
        cwdDir + "/llms.txt",
        cwdDir + "/models.txt",
        appDir + "/dist/llms.txt",
        appDir + "/dist/models.txt"
    };

    QString pickedPath;
    for (const QString &p : candidates) {
        if (QFile::exists(p)) { pickedPath = p; break; }
    }

    QFile modelsFile(pickedPath);
    if (!modelsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open llms.txt or models.txt file from candidates" << candidates;
        return models;
    }
    
    QTextStream in(&modelsFile);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }
        // Format: name, size, task, url  OR  name, size, task, rating/downloads, url
        QStringList parts = line.split(',');
        for (int i = 0; i < parts.size(); ++i) parts[i] = parts[i].trimmed();
        
        // Need at least 4 fields: name, size, task, url
        if (parts.size() < 4) continue;

        const QString modelName = parts[0];
        const QString size = parts[1];
        QString taskType = parts[2];
        QString rating;
        QString url;

        // Determine if we have 4 or 5+ fields
        if (parts.size() == 4) {
            // Format: name, size, task, url (no rating)
            url = parts[3];
            rating = "N/A";
        } else {
            // Format: name, size, task, rating/downloads, url
            url = parts.last();
            rating = parts[3].trimmed();
        }

        // Normalize task type labels
        const QString t = taskType.toLower();
        if (t.contains("text-to-image") || t == "text to image" || t == "image" || t == "t2i") {
            taskType = "Text-to-Image";
        } else if (t.contains("text-to-video") || t == "text to video" || t == "video" || t == "t2v") {
            taskType = "Text-to-Video";
        } else if (t.contains("tts") || t.contains("text-to-speech") || t == "speech") {
            taskType = "Text-to-Speech";
        } else if (t.contains("text generation") || t == "text") {
            taskType = "Text Generation";
        } else {
            taskType = "Text Generation";  // Default
        }

        QJsonObject model = parseModelInfo(modelName, size, taskType, rating, url);
        models.append(model);
    }
    if (models.isEmpty()) {
        qWarning() << "Parsed 0 models from" << pickedPath;
    } else {
        qDebug() << "Parsed" << models.size() << "models from" << pickedPath;
    }
    
    return models;
}

QString HuggingFaceClient::downloadModel(const QString &modelName)
{
    if (m_activeDownloads.contains(modelName)) {
        return QString("Model %1 is already being downloaded").arg(modelName);
    }
    
    // Get model URL from available models
    QString modelUrl;
    QJsonArray models = getAvailableModels();
    for (const QJsonValue &value : models) {
        QJsonObject model = value.toObject();
        if (model["name"].toString() == modelName) {
            modelUrl = model["url"].toString();
            break;
        }
    }
    
    if (modelUrl.isEmpty()) {
        return QString("Error: Model %1 not found in available models").arg(modelName);
    }
    
    // Create cache directory  
    QString cachePath = QStandardPaths::writableLocation(QStandardPaths::CacheLocation) + "/RunMyModelDesktop/models";
    QDir cacheDir(cachePath);
    if (!cacheDir.exists()) {
        cacheDir.mkpath(".");
    }
    
    QString safeModelName = QString(modelName).replace("/", "_");
    QString modelPath = cachePath + "/" + safeModelName;
    QDir().mkpath(modelPath);
    
    qDebug() << "Starting REAL download for:" << modelName;
    qDebug() << "Hugging Face URL:" << modelUrl;
    qDebug() << "Save to:" << modelPath;
    
    // Create download info
    DownloadInfo info;
    info.modelName = modelName;
    info.receivedBytes = 0;
    info.lastReceivedBytes = 0;
    info.totalBytes = 0;
    info.progressTimer = nullptr;
    info.reply = nullptr;
    info.startTime = QDateTime::currentDateTime();
    info.lastUpdateTime = info.startTime;
    info.speed = 0;
    
    // REAL DOWNLOAD using QNetworkAccessManager
    // Hugging Face URLs are in format: https://huggingface.co/{author}/{model}
    // We need to construct the actual file download URL
    
    // Parse model URL to get author/model and construct proper download URL
    QString downloadUrl;
    
    // Check if URL already contains /resolve/ (direct file link)
    if (modelUrl.contains("/resolve/")) {
        downloadUrl = modelUrl;
    } else {
        // Try common GGUF file patterns
        // Most models have files like: model-name-Q4_K_M.gguf, model-name-Q5_K_M.gguf, etc.
        // For now, we'll need to query the API to find the actual file
        
        // Extract repo from URL: https://huggingface.co/author/model -> author/model
        QString repo = modelUrl;
        repo.remove("https://huggingface.co/");
        
        qDebug() << "Fetching file list from Hugging Face API for:" << repo;
        
        // Use Hugging Face API to list files
        downloadUrl = QString("https://huggingface.co/api/models/%1").arg(repo);
    }
    
    qDebug() << "Step 1: Fetching model info from:" << downloadUrl;
    
    QNetworkRequest request(downloadUrl);
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    request.setRawHeader("User-Agent", "RunMyModel-Desktop/0.2.0");
    
    QNetworkReply *reply = m_networkManager->get(request);
    
    // First, get the model info to find actual GGUF files
    connect(reply, &QNetworkReply::finished, [this, modelName, modelUrl, modelPath, reply]() {
        reply->deleteLater();
        
        if (reply->error() != QNetworkReply::NoError) {
            qWarning() << "❌ Failed to fetch model info:" << reply->errorString();
            m_activeDownloads.remove(modelName);
            emit downloadError(modelName, "Failed to fetch model info: " + reply->errorString());
            return;
        }
        
        // Parse JSON response to find GGUF files
        QByteArray response = reply->readAll();
        QJsonDocument doc = QJsonDocument::fromJson(response);
        QJsonObject obj = doc.object();
        
        QString actualDownloadUrl;
        qint64 fileSize = 0;
        
        // Look for siblings (files in the model)
        if (obj.contains("siblings")) {
            QJsonArray siblings = obj["siblings"].toArray();
            
            qDebug() << "Found" << siblings.size() << "files in model";
            
            // Prioritize Q4_K_M quantization (good balance of size/quality)
            QStringList preferred = {"Q4_K_M.gguf", "q4_k_m.gguf", "Q5_K_M.gguf", "q5_k_m.gguf", ".gguf"};
            
            for (const QString &pattern : preferred) {
                for (const QJsonValue &sibling : siblings) {
                    QJsonObject file = sibling.toObject();
                    QString filename = file["rfilename"].toString();
                    
                    if (filename.endsWith(pattern, Qt::CaseInsensitive)) {
                        actualDownloadUrl = modelUrl + "/resolve/main/" + filename;
                        fileSize = file["size"].toVariant().toLongLong();
                        
                        qDebug() << "Selected file:" << filename;
                        qDebug() << "Size:" << (fileSize / (1024*1024)) << "MB";
                        break;
                    }
                }
                if (!actualDownloadUrl.isEmpty()) break;
            }
        }
        
        if (actualDownloadUrl.isEmpty()) {
            qWarning() << "❌ No GGUF file found in model repository";
            m_activeDownloads.remove(modelName);
            emit downloadError(modelName, "No GGUF file found");
            return;
        }
        
        qDebug() << "Step 2: Starting REAL download from:" << actualDownloadUrl;
        
        // Now do the ACTUAL file download
        QNetworkRequest fileRequest(actualDownloadUrl);
        fileRequest.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
        fileRequest.setRawHeader("User-Agent", "RunMyModel-Desktop/0.2.0");
        
        QNetworkReply *fileReply = m_networkManager->get(fileRequest);
        
        // Update download info
        if (m_activeDownloads.contains(modelName)) {
            m_activeDownloads[modelName].reply = fileReply;
            m_activeDownloads[modelName].totalBytes = fileSize;
        }
        
        // Connect REAL download progress
        connect(fileReply, &QNetworkReply::downloadProgress, 
                [this, modelName](qint64 bytesReceived, qint64 bytesTotal) {
            if (m_activeDownloads.contains(modelName)) {
                DownloadInfo &info = m_activeDownloads[modelName];
                info.receivedBytes = bytesReceived;
                info.totalBytes = bytesTotal;
                
                // Calculate speed
                QDateTime now = QDateTime::currentDateTime();
                qint64 timeDiff = info.lastUpdateTime.msecsTo(now);
                
                if (timeDiff >= 500) { // Update speed every 500ms
                    qint64 bytesDiff = bytesReceived - info.lastReceivedBytes;
                    if (timeDiff > 0) {
                        info.speed = (bytesDiff * 1000.0) / timeDiff; // bytes per second
                    }
                    info.lastReceivedBytes = bytesReceived;
                    info.lastUpdateTime = now;
                }
                
                if (bytesTotal > 0) {
                    double progress = (double)bytesReceived / bytesTotal * 100.0;
                    emit downloadProgress(modelName, progress);
                    
                    if (info.speed > 0) {
                        qDebug() << "REAL Download:" << modelName 
                                 << QString::number(progress, 'f', 1) << "%" 
                                 << "(" << bytesReceived / (1024*1024) << "MB /"
                                 << bytesTotal / (1024*1024) << "MB)"
                                 << "@" << QString::number(info.speed / (1024*1024), 'f', 2) << "MB/s";
                    }
                }
            }
        });
        
        // Connect finished signal for ACTUAL download
        connect(fileReply, &QNetworkReply::finished, [this, modelName, modelPath, fileReply]() {
            fileReply->deleteLater();
            
            if (fileReply->error() == QNetworkReply::NoError) {
                // Save the REAL downloaded data
                QString filename = modelPath + "/model.gguf";
                QFile file(filename);
                
                if (file.open(QIODevice::WriteOnly)) {
                    // Write in chunks to avoid memory issues with large files
                    qint64 totalWritten = 0;
                    QByteArray chunk;
                    
                    // For finished reply, we can read all at once
                    QByteArray allData = fileReply->readAll();
                    file.write(allData);
                    file.close();
                    
                    qDebug() << "✅ REAL Download complete:" << modelName;
                    qDebug() << "   Saved to:" << file.fileName();
                    qDebug() << "   Size:" << (file.size() / (1024.0*1024)) << "MB";
                    
                    m_activeDownloads.remove(modelName);
                    emit downloadComplete(modelName);
                } else {
                    qWarning() << "❌ Failed to save file:" << file.fileName();
                    m_activeDownloads.remove(modelName);
                    emit downloadError(modelName, "Failed to save file");
                }
            } else {
                QString errorMsg = fileReply->errorString();
                qWarning() << "❌ REAL Download error for" << modelName << ":" << errorMsg;
                m_activeDownloads.remove(modelName);
                emit downloadError(modelName, errorMsg);
            }
        });
    });
    
    info.reply = reply;
    m_activeDownloads[modelName] = info;
    
    return QString("Started REAL download: %1\nQuerying Hugging Face API...").arg(modelName);
}

QString HuggingFaceClient::removeModel(const QString &modelName)
{
    QString modelPath = getModelPath(modelName);
    
    if (QFile::exists(modelPath)) {
        if (QDir(modelPath).removeRecursively()) {
            return QString("Successfully removed model: %1").arg(modelName);
        } else {
            return QString("Failed to remove model: %1").arg(modelName);
        }
    } else {
        return QString("Model %1 is not found in cache").arg(modelName);
    }
}

QJsonObject HuggingFaceClient::getModelInfo(const QString &modelName)
{
    QJsonArray models = getAvailableModels();
    for (const QJsonValue &value : models) {
        QJsonObject model = value.toObject();
        if (model["name"].toString() == modelName) {
            return model;
        }
    }
    
    return QJsonObject();
}

QJsonObject HuggingFaceClient::getDownloadProgress(const QString &modelName)
{
    QJsonObject progress;
    
    if (m_activeDownloads.contains(modelName)) {
        const DownloadInfo &info = m_activeDownloads[modelName];
        progress["model_name"] = modelName;
        progress["progress"] = info.totalBytes > 0 ? 
            (double)info.receivedBytes / info.totalBytes * 100.0 : 0.0;
        progress["status"] = "downloading";
        progress["downloaded_bytes"] = info.receivedBytes;
        progress["total_bytes"] = info.totalBytes;
        progress["speed"] = info.speed; // bytes per second
    } else {
        progress["model_name"] = modelName;
        progress["progress"] = 0.0;
        progress["status"] = "not_started";
        progress["downloaded_bytes"] = 0;
        progress["total_bytes"] = 0;
        progress["speed"] = 0.0;
    }
    
    return progress;
}

void HuggingFaceClient::onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal)
{
    // This would be called by QNetworkReply in a real implementation
    Q_UNUSED(bytesReceived)
    Q_UNUSED(bytesTotal)
}

void HuggingFaceClient::onDownloadFinished()
{
    // This would be called by QNetworkReply in a real implementation
}

void HuggingFaceClient::onDownloadError(QNetworkReply::NetworkError error)
{
    Q_UNUSED(error)
    // This would be called by QNetworkReply in a real implementation
}

QString HuggingFaceClient::getCacheDir() const
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    return cacheDir + "/huggingface/hub";
}

QString HuggingFaceClient::getModelPath(const QString &modelName) const
{
    QString cacheDir = getCacheDir();
    QString modelDir = modelName;
    modelDir.replace('/', "--");
    return cacheDir + "/models--" + modelDir;
}

QJsonObject HuggingFaceClient::parseModelInfo(const QString &modelName, const QString &size, 
                                             const QString &taskType, const QString &rating, const QString &url) const
{
    QJsonObject model;
    model["name"] = modelName;
    model["size"] = size;
    model["task_type"] = taskType;
    model["rating"] = rating;
    model["url"] = url;
    
    // Check if model is downloaded
    QString modelPath = getModelPath(modelName);
    bool downloaded = QDir(modelPath).exists();
    model["downloaded"] = downloaded;
    
    if (downloaded) {
        model["local_path"] = modelPath;
    }
    
    return model;
}

void HuggingFaceClient::startProgressTracking(const QString &modelName, QNetworkReply *reply)
{
    Q_UNUSED(modelName)
    Q_UNUSED(reply)
    // This would be implemented for real downloads
}

void HuggingFaceClient::stopProgressTracking(const QString &modelName)
{
    if (m_activeDownloads.contains(modelName)) {
        DownloadInfo &info = m_activeDownloads[modelName];
        if (info.progressTimer) {
            info.progressTimer->stop();
            info.progressTimer->deleteLater();
        }
        m_activeDownloads.remove(modelName);
    }
}
