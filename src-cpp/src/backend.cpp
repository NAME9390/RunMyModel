#include "backend.h"
#include "huggingface_client.h"
#include "model_manager.h"
#include "system_info.h"
#include "web_server.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QDebug>

Backend::Backend(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    // Initialize components
    m_huggingFaceClient = std::make_unique<HuggingFaceClient>(this);
    m_modelManager = std::make_unique<ModelManager>(this);
    m_systemInfo = std::make_unique<SystemInfo>(this);
    m_webServer = std::make_unique<WebServer>(this);
    
    // Connect signals
    connect(m_huggingFaceClient.get(), &HuggingFaceClient::downloadProgress,
            this, &Backend::onModelDownloadProgress);
    connect(m_huggingFaceClient.get(), &HuggingFaceClient::downloadComplete,
            this, &Backend::onModelDownloadComplete);
    connect(m_huggingFaceClient.get(), &HuggingFaceClient::downloadError,
            this, &Backend::onModelDownloadError);
    
    // Set backend reference in web server
    m_webServer->setBackend(this);
}

Backend::~Backend() = default;

QJsonObject Backend::getSystemInfo()
{
    return m_systemInfo->getSystemInfo();
}

QJsonArray Backend::getAllHuggingFaceModels()
{
    return m_huggingFaceClient->getAvailableModels();
}

QString Backend::downloadHuggingFaceModel(const QString &modelName)
{
    return m_huggingFaceClient->downloadModel(modelName);
}

QString Backend::removeHuggingFaceModel(const QString &modelName)
{
    return m_huggingFaceClient->removeModel(modelName);
}

QJsonObject Backend::getModelDownloadProgress(const QString &modelName)
{
    return m_huggingFaceClient->getDownloadProgress(modelName);
}

QJsonObject Backend::chatWithHuggingFace(const QJsonObject &request)
{
    // For now, return a placeholder response
    // In a real implementation, this would load and run the model
    QJsonObject response;
    response["content"] = QString("This is a placeholder response from Hugging Face model: %1. The actual model inference would be implemented here.")
                         .arg(request["model"].toString());
    
    QJsonObject usage;
    usage["prompt_tokens"] = request["messages"].toArray().size();
    usage["completion_tokens"] = 50; // Placeholder
    usage["total_tokens"] = usage["prompt_tokens"].toInt() + usage["completion_tokens"].toInt();
    
    response["usage"] = usage;
    
    return response;
}

void Backend::startWebServer()
{
    m_webServer->startServer();
}

void Backend::stopWebServer()
{
    m_webServer->stopServer();
}

int Backend::getWebServerPort() const
{
    if (m_webServer) {
        return static_cast<int>(m_webServer->port());
    }
    return 8080;  // Default fallback
}

void Backend::onModelDownloadProgress(const QString &modelName, double progress)
{
    emit modelDownloadProgress(modelName, progress);
}

void Backend::onModelDownloadComplete(const QString &modelName)
{
    emit modelDownloadComplete(modelName);
}

void Backend::onModelDownloadError(const QString &modelName, const QString &error)
{
    emit modelDownloadError(modelName, error);
}
