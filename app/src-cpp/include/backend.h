#ifndef BACKEND_H
#define BACKEND_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>

// Legacy Backend - Now mostly replaced by Python backend
// Kept for backward compatibility with UI signals
class Backend : public QObject
{
    Q_OBJECT

public:
    explicit Backend(QObject *parent = nullptr);
    ~Backend();

    // Model management (legacy - now handled by Python backend)
    Q_INVOKABLE QJsonArray getAllHuggingFaceModels();
    Q_INVOKABLE QString downloadHuggingFaceModel(const QString &modelName);
    Q_INVOKABLE QString removeHuggingFaceModel(const QString &modelName);
    Q_INVOKABLE bool cancelModelDownload(const QString &modelName);
    Q_INVOKABLE QJsonObject getModelDownloadProgress(const QString &modelName);
    
    // Chat functionality (placeholder - real implementation in Python backend)
    Q_INVOKABLE QJsonObject chatWithHuggingFace(const QJsonObject &request);

signals:
    void modelDownloadProgress(const QString &modelName, double progress);
    void modelDownloadComplete(const QString &modelName);
    void modelDownloadError(const QString &modelName, const QString &error);

private:
    QNetworkAccessManager *m_networkManager;
};

#endif // BACKEND_H
