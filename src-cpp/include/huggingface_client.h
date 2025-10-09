#ifndef HUGGINGFACE_CLIENT_H
#define HUGGINGFACE_CLIENT_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QFile>
#include <memory>

class HuggingFaceClient : public QObject
{
    Q_OBJECT

public:
    explicit HuggingFaceClient(QObject *parent = nullptr);
    ~HuggingFaceClient();

    // Model operations
    QJsonArray getAvailableModels();
    QString downloadModel(const QString &modelName);
    bool cancelDownload(const QString &modelName);
    QString removeModel(const QString &modelName);
    QJsonObject getModelInfo(const QString &modelName);
    
    // Download progress tracking
    QJsonObject getDownloadProgress(const QString &modelName);

signals:
    void downloadProgress(const QString &modelName, double progress);
    void downloadComplete(const QString &modelName);
    void downloadError(const QString &modelName, const QString &error);

private slots:
    void onDownloadProgress(qint64 bytesReceived, qint64 bytesTotal);
    void onDownloadFinished();
    void onDownloadError(QNetworkReply::NetworkError error);

private:
    struct DownloadInfo {
        QString modelName;
        QNetworkReply *reply;
        QFile *outputFile;  // For streaming write
        qint64 totalBytes;
        qint64 receivedBytes;
        qint64 lastReceivedBytes;
        QTimer *progressTimer;
        QDateTime startTime;
        QDateTime lastUpdateTime;
        double speed; // bytes per second
    };

    QNetworkAccessManager *m_networkManager;
    QMap<QString, DownloadInfo> m_activeDownloads;
    
    QString getCacheDir() const;
    QString getModelPath(const QString &modelName) const;
    QJsonObject parseModelInfo(const QString &modelName, const QString &size, 
                              const QString &taskType, const QString &rating, const QString &url) const;
    void startProgressTracking(const QString &modelName, QNetworkReply *reply);
    void stopProgressTracking(const QString &modelName);
};

#endif // HUGGINGFACE_CLIENT_H
