#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QFuture>
#include <QtConcurrent>
#include <QProcess>
#include <QTimer>
#include <QElapsedTimer>
#include <QCoreApplication>
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

Q_LOGGING_CATEGORY(modelManager, "model.manager")

/**
 * @brief Advanced Model Manager for GGUF model support and creation
 * 
 * This class provides:
 * - Multiple GGUF model support
 * - Model switching and management
 * - GGUF model creation and conversion
 * - Model metadata and information
 * - Automatic model discovery
 */
class ModelManager : public QObject
{
    Q_OBJECT

public:
    explicit ModelManager(QObject *parent = nullptr);
    ~ModelManager();

    // Model Management
    bool loadModel(const QString &modelPath);
    bool unloadModel();
    QString getCurrentModelPath() const;
    QString getCurrentModelName() const;
    bool isModelLoaded() const;
    
    // Model Discovery
    QStringList discoverModels(const QString &directory = "");
    QStringList getAvailableModels() const;
    bool addModelDirectory(const QString &directory);
    void refreshModelList();
    
    // Model Information
    QJsonObject getModelInfo(const QString &modelPath) const;
    QStringList getModelCapabilities(const QString &modelPath) const;
    qint64 getModelSize(const QString &modelPath) const;
    QString getModelFormat(const QString &modelPath) const;
    
    // GGUF Model Creation
    bool createGGUFModel(const QString &sourcePath, const QString &outputPath, const QJsonObject &config);
    bool convertToGGUF(const QString &inputPath, const QString &outputPath, const QJsonObject &config);
    bool quantizeModel(const QString &inputPath, const QString &outputPath, const QString &quantization);
    
    // Model Configuration
    void setModelDirectory(const QString &directory);
    QString getModelDirectory() const;
    void setDefaultModel(const QString &modelPath);
    QString getDefaultModel() const;
    
    // Model Validation
    bool validateModel(const QString &modelPath) const;
    QStringList getModelErrors(const QString &modelPath) const;
    bool isGGUFModel(const QString &modelPath) const;

signals:
    void modelLoaded(const QString &modelPath);
    void modelUnloaded();
    void modelListUpdated();
    void modelCreationProgress(int percentage);
    void modelCreationFinished(bool success, const QString &outputPath);
    void errorOccurred(const QString &error);

private slots:
    void processModelCreation(const QString &sourcePath, const QString &outputPath, const QJsonObject &config);

private:
    struct ModelInfo {
        QString name;
        QString path;
        QString format;
        qint64 size;
        QJsonObject metadata;
        QDateTime lastModified;
        bool isValid;
    };
    
    // Core data
    QMap<QString, ModelInfo> m_models;
    QString m_currentModelPath;
    QString m_modelDirectory;
    QString m_defaultModel;
    QMutex m_mutex;
    
    // Configuration
    QStringList m_modelDirectories;
    QStringList m_supportedFormats;
    
    // Helper methods
    QStringList scanDirectoryForModels(const QString &directory);
    QJsonObject extractModelMetadata(const QString &modelPath);
    bool validateGGUFHeader(const QString &modelPath);
    QString generateModelName(const QString &path);
    void initializeModelDirectories();
    bool runConversionProcess(const QString &command, const QStringList &arguments);
    QStringList getConversionArguments(const QString &inputPath, const QString &outputPath, const QJsonObject &config);
    void updateModelList();
    bool isModelFile(const QString &filePath) const;
    QString getModelFormatFromPath(const QString &path) const;
    QJsonObject createDefaultConfig() const;
    void logModelInfo(const ModelInfo &info);
    bool createModelDirectory(const QString &path);
    QString sanitizeModelName(const QString &name) const;
    bool backupModel(const QString &modelPath) const;
    bool restoreModel(const QString &backupPath) const;
};

#endif // MODEL_MANAGER_H