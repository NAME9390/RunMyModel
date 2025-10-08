#include "model_manager.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QStandardPaths>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>

ModelManager::ModelManager(QObject *parent)
    : QObject(parent)
    , m_fileWatcher(new QFileSystemWatcher(this))
    , m_refreshTimer(new QTimer(this))
{
    m_cacheDir = getCacheDirectory();
    
    // Setup file watcher
    connect(m_fileWatcher, &QFileSystemWatcher::directoryChanged,
            this, &ModelManager::onDirectoryChanged);
    
    // Setup refresh timer
    m_refreshTimer->setSingleShot(true);
    m_refreshTimer->setInterval(1000); // 1 second delay
    connect(m_refreshTimer, &QTimer::timeout,
            this, &ModelManager::refreshModelList);
    
    // Load initial model list
    loadModelsFromFile();
    scanInstalledModels();
}

ModelManager::~ModelManager() = default;

QJsonArray ModelManager::getInstalledModels()
{
    QJsonArray installed;
    
    for (auto it = m_models.begin(); it != m_models.end(); ++it) {
        if (it->downloaded) {
            installed.append(modelInfoToJson(it.value()));
        }
    }
    
    return installed;
}

QJsonArray ModelManager::getAvailableModels()
{
    QJsonArray available;
    
    for (auto it = m_models.begin(); it != m_models.end(); ++it) {
        available.append(modelInfoToJson(it.value()));
    }
    
    return available;
}

bool ModelManager::isModelInstalled(const QString &modelName)
{
    return m_models.contains(modelName) && m_models[modelName].downloaded;
}

QString ModelManager::getModelPath(const QString &modelName)
{
    if (m_models.contains(modelName)) {
        return m_models[modelName].localPath;
    }
    return QString();
}

qint64 ModelManager::getModelSize(const QString &modelName)
{
    if (m_models.contains(modelName)) {
        return m_models[modelName].sizeBytes;
    }
    return 0;
}

bool ModelManager::installModel(const QString &modelName)
{
    if (!m_models.contains(modelName)) {
        return false;
    }
    
    // In a real implementation, this would trigger the download
    // For now, just mark as installed
    m_models[modelName].downloaded = true;
    m_models[modelName].localPath = m_cacheDir + "/" + modelName;
    
    emit modelInstalled(modelName);
    emit modelListChanged();
    
    return true;
}

bool ModelManager::removeModel(const QString &modelName)
{
    if (!m_models.contains(modelName) || !m_models[modelName].downloaded) {
        return false;
    }
    
    QString modelPath = m_models[modelName].localPath;
    if (QDir(modelPath).removeRecursively()) {
        m_models[modelName].downloaded = false;
        m_models[modelName].localPath.clear();
        
        emit modelRemoved(modelName);
        emit modelListChanged();
        
        return true;
    }
    
    return false;
}

QJsonObject ModelManager::getModelInfo(const QString &modelName)
{
    if (m_models.contains(modelName)) {
        return modelInfoToJson(m_models[modelName]);
    }
    return QJsonObject();
}

QString ModelManager::getModelDisplayName(const QString &modelName)
{
    if (m_models.contains(modelName)) {
        return m_models[modelName].name;
    }
    return modelName;
}

QString ModelManager::getModelAuthor(const QString &modelName)
{
    if (m_models.contains(modelName)) {
        // Extract author from model name (e.g., "author/model-name")
        QStringList parts = modelName.split('/');
        if (parts.size() >= 2) {
            return parts[0];
        }
    }
    return QString();
}

void ModelManager::onDirectoryChanged(const QString &path)
{
    Q_UNUSED(path)
    m_refreshTimer->start();
}

void ModelManager::refreshModelList()
{
    scanInstalledModels();
}

void ModelManager::loadModelsFromFile()
{
    // Look for llms.txt first in app dir, then models.txt
    const QString appDir = QCoreApplication::applicationDirPath();
    const QString llmsPath = appDir + "/llms.txt";
    const QString modelsPath = appDir + "/models.txt";
    QFile modelsFile(QFile::exists(llmsPath) ? llmsPath : modelsPath);
    if (!modelsFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open llms.txt or models.txt file";
        return;
    }
    
    QTextStream in(&modelsFile);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        
        // Skip empty lines and comments
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }
        
        // Parse CSV format: Model Name, Parameter Size, Task Type, Rating, Hugging Face URL
        QStringList parts = line.split(',');
        if (parts.size() >= 5) {
            ModelInfo info;
            info.name = parts[0].trimmed();
            info.size = parts[1].trimmed();
            info.taskType = parts[2].trimmed();
            info.rating = parts[3].trimmed().toInt();
            info.url = parts[4].trimmed();
            info.downloaded = false;
            info.localPath.clear();
            info.sizeBytes = 0;
            
            m_models[info.name] = info;
        }
    }
    
    // Watch the file for changes
    m_fileWatcher->addPath(modelsFile.fileName());
}

void ModelManager::scanInstalledModels()
{
    QDir cacheDir(m_cacheDir);
    if (!cacheDir.exists()) {
        return;
    }
    
    // Scan for installed models
    QStringList entries = cacheDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    for (const QString &entry : entries) {
        if (entry.startsWith("models--")) {
            QString modelName = entry.mid(8); // Remove "models--" prefix
            modelName.replace("--", "/");
            
            if (m_models.contains(modelName)) {
                updateModelDownloadStatus(modelName, true);
            }
        }
    }
}

QString ModelManager::getCacheDirectory() const
{
    QString cacheDir = QStandardPaths::writableLocation(QStandardPaths::CacheLocation);
    const QString path = cacheDir + "/huggingface/hub";
    QDir().mkpath(path);
    return path;
}

QJsonObject ModelManager::modelInfoToJson(const ModelInfo &info) const
{
    QJsonObject json;
    json["name"] = info.name;
    json["size"] = info.size;
    json["task_type"] = info.taskType;
    json["rating"] = info.rating;
    json["url"] = info.url;
    json["downloaded"] = info.downloaded;
    json["local_path"] = info.localPath;
    json["size_bytes"] = info.sizeBytes;
    return json;
}

void ModelManager::updateModelDownloadStatus(const QString &modelName, bool downloaded)
{
    if (m_models.contains(modelName)) {
        bool wasDownloaded = m_models[modelName].downloaded;
        m_models[modelName].downloaded = downloaded;
        
        if (downloaded && !wasDownloaded) {
            QString modelPath = modelName;
            modelPath.replace("/", "--");
            m_models[modelName].localPath = m_cacheDir + "/models--" + modelPath;
            emit modelInstalled(modelName);
        } else if (!downloaded && wasDownloaded) {
            m_models[modelName].localPath.clear();
            emit modelRemoved(modelName);
        }
        
        emit modelListChanged();
    }
}
