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
    
    QString modelPath = getModelPath(modelName);
    QDir().mkpath(QFileInfo(modelPath).absolutePath());
    
    // Create download info
    DownloadInfo info;
    info.modelName = modelName;
    info.totalBytes = 0;
    info.receivedBytes = 0;
    
    // For now, simulate download progress
    // In a real implementation, this would download from Hugging Face Hub
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, [this, modelName, timer]() {
        static int progress = 0;
        progress += 10;
        
        if (progress >= 100) {
            progress = 100;
            timer->stop();
            timer->deleteLater();
            emit downloadComplete(modelName);
        } else {
            emit downloadProgress(modelName, progress);
        }
    });
    
    info.progressTimer = timer;
    timer->start(100); // Update every 100ms
    
    m_activeDownloads[modelName] = info;
    
    return QString("Started downloading model: %1").arg(modelName);
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
    } else {
        progress["model_name"] = modelName;
        progress["progress"] = 0.0;
        progress["status"] = "not_started";
        progress["downloaded_bytes"] = 0;
        progress["total_bytes"] = 0;
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
