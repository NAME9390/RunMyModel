#include "backend.h"
#include "huggingface_client.h"
#include "model_manager.h"
#include "system_info.h"
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
    
    // Connect signals
    connect(m_huggingFaceClient.get(), &HuggingFaceClient::downloadProgress,
            this, &Backend::onModelDownloadProgress);
    connect(m_huggingFaceClient.get(), &HuggingFaceClient::downloadComplete,
            this, &Backend::onModelDownloadComplete);
    connect(m_huggingFaceClient.get(), &HuggingFaceClient::downloadError,
            this, &Backend::onModelDownloadError);
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
    // Real model inference using llama.cpp would go here
    // For now, return an informative message that this requires model loading
    QJsonObject response;
    QString modelName = request["model"].toString();
    
    response["content"] = QString(
        "⚠️ Model inference not yet implemented.\n\n"
        "To enable chat with %1:\n"
        "1. Integrate llama.cpp library\n"
        "2. Load model from ~/Documents/rmm/%2/model.gguf\n"
        "3. Run inference with your message\n\n"
        "This would require linking against llama.cpp and implementing model loading/inference logic."
    ).arg(modelName, QString(modelName).replace("/", "_"));
    
    QJsonObject usage;
    usage["prompt_tokens"] = 0;
    usage["completion_tokens"] = 0;
    usage["total_tokens"] = 0;
    
    response["usage"] = usage;
    response["error"] = "Inference not implemented yet";
    
    return response;
}

// Web server methods removed - not needed for native UI

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

bool Backend::cancelModelDownload(const QString &modelName)
{
    return m_huggingFaceClient->cancelDownload(modelName);
}
