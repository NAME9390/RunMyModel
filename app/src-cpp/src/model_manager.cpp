#include "model_manager.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QHash>
#include <QSet>
#include <QDateTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QProcess>
#include <QThread>
#include <QMutexLocker>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QTextStream>
#include <QDebug>
#include <QLoggingCategory>

ModelManager::ModelManager(QObject *parent)
    : QObject(parent)
    , m_modelDirectory(QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/models")
{
    initializeModelDirectories();
    refreshModelList();
    qCDebug(modelManager) << "Model Manager initialized";
}

ModelManager::~ModelManager()
{
    qCDebug(modelManager) << "Model Manager destroyed";
}

void ModelManager::initializeModelDirectories()
{
    m_supportedFormats = {"gguf", "bin", "safetensors", "pt", "pth"};
    
    // Add default model directories
    m_modelDirectories.append(m_modelDirectory);
    m_modelDirectories.append(QDir::homePath() + "/models");
    m_modelDirectories.append("/usr/local/share/models");
    
    // Create model directory if it doesn't exist
    QDir().mkpath(m_modelDirectory);
    
    qCDebug(modelManager) << "Model directories initialized:" << m_modelDirectories;
}

bool ModelManager::loadModel(const QString &modelPath)
{
    QMutexLocker locker(&m_mutex);
    
    if (!QFile::exists(modelPath)) {
        emit errorOccurred(QString("Model file does not exist: %1").arg(modelPath));
        return false;
    }
    
    if (!validateModel(modelPath)) {
        emit errorOccurred(QString("Invalid model file: %1").arg(modelPath));
        return false;
    }
    
    m_currentModelPath = modelPath;
    emit modelLoaded(modelPath);
    
    qCDebug(modelManager) << "Model loaded:" << modelPath;
    return true;
}

bool ModelManager::unloadModel()
{
    QMutexLocker locker(&m_mutex);
    
    if (m_currentModelPath.isEmpty()) {
        return false;
    }
    
    m_currentModelPath.clear();
    emit modelUnloaded();
    
    qCDebug(modelManager) << "Model unloaded";
    return true;
}

QString ModelManager::getCurrentModelPath() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentModelPath;
}

QString ModelManager::getCurrentModelName() const
{
    QMutexLocker locker(&m_mutex);
    if (m_currentModelPath.isEmpty()) {
        return QString();
    }
    return QFileInfo(m_currentModelPath).baseName();
}

bool ModelManager::isModelLoaded() const
{
    QMutexLocker locker(&m_mutex);
    return !m_currentModelPath.isEmpty();
}

QStringList ModelManager::discoverModels(const QString &directory)
{
    QString searchDir = directory.isEmpty() ? m_modelDirectory : directory;
    return scanDirectoryForModels(searchDir);
}

QStringList ModelManager::scanDirectoryForModels(const QString &directory)
{
    QStringList models;
    QDir dir(directory);
    
    if (!dir.exists()) {
        return models;
    }
    
    QStringList filters;
    for (const QString &format : m_supportedFormats) {
        filters << QString("*.%1").arg(format);
    }
    
    QFileInfoList files = dir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo &fileInfo : files) {
        QString modelPath = fileInfo.absoluteFilePath();
        if (validateModel(modelPath)) {
            models.append(modelPath);
        }
    }
    
    return models;
}

QStringList ModelManager::getAvailableModels() const
{
    QMutexLocker locker(&m_mutex);
    return m_models.keys();
}

bool ModelManager::addModelDirectory(const QString &directory)
{
    if (!QDir(directory).exists()) {
        emit errorOccurred(QString("Directory does not exist: %1").arg(directory));
        return false;
    }
    
    if (!m_modelDirectories.contains(directory)) {
        m_modelDirectories.append(directory);
        refreshModelList();
        qCDebug(modelManager) << "Added model directory:" << directory;
    }
    
    return true;
}

void ModelManager::refreshModelList()
{
    QMutexLocker locker(&m_mutex);
    
    m_models.clear();
    
    for (const QString &dir : m_modelDirectories) {
        QStringList models = scanDirectoryForModels(dir);
        
        for (const QString &modelPath : models) {
            ModelInfo info;
            info.name = generateModelName(modelPath);
            info.path = modelPath;
            info.format = getModelFormatFromPath(modelPath);
            info.size = QFileInfo(modelPath).size();
            info.metadata = extractModelMetadata(modelPath);
            info.lastModified = QFileInfo(modelPath).lastModified();
            info.isValid = validateModel(modelPath);
            
            m_models[modelPath] = info;
        }
    }
    
    emit modelListUpdated();
    qCDebug(modelManager) << "Model list refreshed, found" << m_models.size() << "models";
}

QJsonObject ModelManager::getModelInfo(const QString &modelPath) const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_models.contains(modelPath)) {
        const ModelInfo &info = m_models[modelPath];
        
        QJsonObject json;
        json["name"] = info.name;
        json["path"] = info.path;
        json["format"] = info.format;
        json["size"] = static_cast<qint64>(info.size);
        json["lastModified"] = info.lastModified.toString(Qt::ISODate);
        json["isValid"] = info.isValid;
        json["metadata"] = info.metadata;
        
        return json;
    }
    
    return QJsonObject();
}

QStringList ModelManager::getModelCapabilities(const QString &modelPath) const
{
    QStringList capabilities;
    
    if (isGGUFModel(modelPath)) {
        capabilities << "GGUF" << "Quantized" << "Cross-platform";
    }
    
    QFileInfo fileInfo(modelPath);
    if (fileInfo.size() > 1024 * 1024 * 1024) { // > 1GB
        capabilities << "Large-model";
    }
    
    capabilities << "Local-inference" << "Offline-capable";
    
    return capabilities;
}

qint64 ModelManager::getModelSize(const QString &modelPath) const
{
    return QFileInfo(modelPath).size();
}

QString ModelManager::getModelFormat(const QString &modelPath) const
{
    return getModelFormatFromPath(modelPath);
}

bool ModelManager::createGGUFModel(const QString &sourcePath, const QString &outputPath, const QJsonObject &config)
{
    if (!QFile::exists(sourcePath)) {
        emit errorOccurred(QString("Source file does not exist: %1").arg(sourcePath));
        return false;
    }
    
    // Process asynchronously
    QFuture<void> future = QtConcurrent::run([this, sourcePath, outputPath, config]() {
        processModelCreation(sourcePath, outputPath, config);
    });
    
    return true;
}

bool ModelManager::convertToGGUF(const QString &inputPath, const QString &outputPath, const QJsonObject &config)
{
    return createGGUFModel(inputPath, outputPath, config);
}

bool ModelManager::quantizeModel(const QString &inputPath, const QString &outputPath, const QString &quantization)
{
    QJsonObject config;
    config["quantization"] = quantization;
    config["method"] = "quantize";
    
    return createGGUFModel(inputPath, outputPath, config);
}

void ModelManager::processModelCreation(const QString &sourcePath, const QString &outputPath, const QJsonObject &config)
{
    emit modelCreationProgress(10);
    
    // Create output directory if needed
    QFileInfo outputInfo(outputPath);
    QDir().mkpath(outputInfo.absolutePath());
    
    emit modelCreationProgress(30);
    
    // Simulate model conversion process
    QProcess process;
    QStringList arguments;
    
    if (config.contains("quantization")) {
        arguments << "quantize" << sourcePath << outputPath << config["quantization"].toString();
    } else {
        arguments << "convert" << sourcePath << outputPath;
    }
    
    emit modelCreationProgress(50);
    
    // For now, just copy the file as a placeholder
    // In a real implementation, this would call llama.cpp conversion tools
    if (QFile::copy(sourcePath, outputPath)) {
        emit modelCreationProgress(90);
        
        // Update model list
        refreshModelList();
        
        emit modelCreationProgress(100);
        emit modelCreationFinished(true, outputPath);
        
        qCDebug(modelManager) << "Model creation completed:" << outputPath;
    } else {
        emit modelCreationFinished(false, outputPath);
        emit errorOccurred(QString("Failed to create model: %1").arg(outputPath));
    }
}

void ModelManager::setModelDirectory(const QString &directory)
{
    m_modelDirectory = directory;
    QDir().mkpath(directory);
    refreshModelList();
}

QString ModelManager::getModelDirectory() const
{
    return m_modelDirectory;
}

void ModelManager::setDefaultModel(const QString &modelPath)
{
    m_defaultModel = modelPath;
}

QString ModelManager::getDefaultModel() const
{
    return m_defaultModel;
}

bool ModelManager::validateModel(const QString &modelPath) const
{
    if (!QFile::exists(modelPath)) {
        return false;
    }
    
    QFileInfo fileInfo(modelPath);
    if (fileInfo.size() < 1024) { // Less than 1KB
        return false;
    }
    
    if (isGGUFModel(modelPath)) {
        return validateGGUFHeader(modelPath);
    }
    
    return true; // Assume other formats are valid
}

QStringList ModelManager::getModelErrors(const QString &modelPath) const
{
    QStringList errors;
    
    if (!QFile::exists(modelPath)) {
        errors << "File does not exist";
        return errors;
    }
    
    QFileInfo fileInfo(modelPath);
    if (fileInfo.size() < 1024) {
        errors << "File too small (less than 1KB)";
    }
    
    if (isGGUFModel(modelPath) && !validateGGUFHeader(modelPath)) {
        errors << "Invalid GGUF header";
    }
    
    return errors;
}

bool ModelManager::isGGUFModel(const QString &modelPath) const
{
    return modelPath.toLower().endsWith(".gguf");
}

QString ModelManager::generateModelName(const QString &path) const
{
    return QFileInfo(path).baseName();
}

QJsonObject ModelManager::extractModelMetadata(const QString &modelPath) const
{
    QJsonObject metadata;
    
    QFileInfo fileInfo(modelPath);
    metadata["fileName"] = fileInfo.fileName();
    metadata["baseName"] = fileInfo.baseName();
    metadata["size"] = static_cast<qint64>(fileInfo.size());
    metadata["lastModified"] = fileInfo.lastModified().toString(Qt::ISODate);
    metadata["format"] = getModelFormatFromPath(modelPath);
    
    if (isGGUFModel(modelPath)) {
        metadata["type"] = "GGUF";
        metadata["quantized"] = true;
    }
    
    return metadata;
}

bool ModelManager::validateGGUFHeader(const QString &modelPath) const
{
    QFile file(modelPath);
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    // Check for GGUF magic bytes
    QByteArray header = file.read(8);
    file.close();
    
    // GGUF files start with "GGUF" magic
    return header.startsWith("GGUF");
}

QString ModelManager::getModelFormatFromPath(const QString &path) const
{
    QString extension = QFileInfo(path).suffix().toLower();
    
    if (extension == "gguf") return "GGUF";
    if (extension == "bin") return "Binary";
    if (extension == "safetensors") return "SafeTensors";
    if (extension == "pt" || extension == "pth") return "PyTorch";
    
    return "Unknown";
}

QJsonObject ModelManager::createDefaultConfig() const
{
    QJsonObject config;
    config["quantization"] = "Q4_K_M";
    config["method"] = "convert";
    config["overwrite"] = false;
    config["threads"] = 4;
    
    return config;
}

void ModelManager::logModelInfo(const ModelInfo &info)
{
    qCDebug(modelManager) << "Model Info:"
                         << "Name:" << info.name
                         << "Path:" << info.path
                         << "Format:" << info.format
                         << "Size:" << info.size
                         << "Valid:" << info.isValid;
}

bool ModelManager::createModelDirectory(const QString &path) const
{
    return QDir().mkpath(path);
}

QString ModelManager::sanitizeModelName(const QString &name) const
{
    QString sanitized = name;
    sanitized.replace(QRegularExpression("[^a-zA-Z0-9_-]"), "_");
    return sanitized;
}

bool ModelManager::backupModel(const QString &modelPath) const
{
    QString backupPath = modelPath + ".backup";
    return QFile::copy(modelPath, backupPath);
}

bool ModelManager::restoreModel(const QString &backupPath) const
{
    QString originalPath = backupPath;
    originalPath.remove(".backup");
    return QFile::copy(backupPath, originalPath);
}