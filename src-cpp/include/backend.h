#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <memory>

class HuggingFaceClient;
class ModelManager;
class SystemInfo;
class WebServer;

class Backend : public QObject
{
    Q_OBJECT

public:
    explicit Backend(QObject *parent = nullptr);
    ~Backend();

    // System information
    Q_INVOKABLE QJsonObject getSystemInfo();
    
    // Model management
    Q_INVOKABLE QJsonArray getAllHuggingFaceModels();
    Q_INVOKABLE QString downloadHuggingFaceModel(const QString &modelName);
    Q_INVOKABLE QString removeHuggingFaceModel(const QString &modelName);
    Q_INVOKABLE QJsonObject getModelDownloadProgress(const QString &modelName);
    
    // Chat functionality
    Q_INVOKABLE QJsonObject chatWithHuggingFace(const QJsonObject &request);

public slots:
    void startWebServer();
    void stopWebServer();

signals:
    void modelDownloadProgress(const QString &modelName, double progress);
    void modelDownloadComplete(const QString &modelName);
    void modelDownloadError(const QString &modelName, const QString &error);

private slots:
    void onModelDownloadProgress(const QString &modelName, double progress);
    void onModelDownloadComplete(const QString &modelName);
    void onModelDownloadError(const QString &modelName, const QString &error);

private:
    std::unique_ptr<HuggingFaceClient> m_huggingFaceClient;
    std::unique_ptr<ModelManager> m_modelManager;
    std::unique_ptr<SystemInfo> m_systemInfo;
    std::unique_ptr<WebServer> m_webServer;
    QNetworkAccessManager *m_networkManager;
};

#endif // BACKEND_H
