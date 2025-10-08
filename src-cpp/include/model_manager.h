#ifndef MODEL_MANAGER_H
#define MODEL_MANAGER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QMap>
#include <QFileSystemWatcher>
#include <QTimer>

class ModelManager : public QObject
{
    Q_OBJECT

public:
    explicit ModelManager(QObject *parent = nullptr);
    ~ModelManager();

    // Model management
    QJsonArray getInstalledModels();
    QJsonArray getAvailableModels();
    bool isModelInstalled(const QString &modelName);
    QString getModelPath(const QString &modelName);
    qint64 getModelSize(const QString &modelName);
    
    // Model operations
    bool installModel(const QString &modelName);
    bool removeModel(const QString &modelName);
    
    // Model information
    QJsonObject getModelInfo(const QString &modelName);
    QString getModelDisplayName(const QString &modelName);
    QString getModelAuthor(const QString &modelName);

signals:
    void modelInstalled(const QString &modelName);
    void modelRemoved(const QString &modelName);
    void modelListChanged();

private slots:
    void onDirectoryChanged(const QString &path);
    void refreshModelList();

private:
    struct ModelInfo {
        QString name;
        QString size;
        QString taskType;
        int rating;
        QString url;
        bool downloaded;
        QString localPath;
        qint64 sizeBytes;
    };

    QMap<QString, ModelInfo> m_models;
    QFileSystemWatcher *m_fileWatcher;
    QTimer *m_refreshTimer;
    QString m_cacheDir;
    
    void loadModelsFromFile();
    void scanInstalledModels();
    QString getCacheDirectory() const;
    QJsonObject modelInfoToJson(const ModelInfo &info) const;
    void updateModelDownloadStatus(const QString &modelName, bool downloaded);
};

#endif // MODEL_MANAGER_H
