#ifndef BACKEND_CLIENT_H
#define BACKEND_CLIENT_H

#include <QObject>
#include <QProcess>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonArray>
#include <QString>
#include <memory>

/**
 * BackendClient - C++ client for Python FastAPI backend
 * 
 * Manages:
 * - Python backend subprocess lifecycle
 * - REST API communication (localhost:5000)
 * - Model management requests
 * - Inference requests (streaming)
 */
class BackendClient : public QObject
{
    Q_OBJECT

public:
    explicit BackendClient(QObject *parent = nullptr);
    ~BackendClient();

    // Backend process management
    bool startBackend();
    void stopBackend();
    bool isBackendRunning() const;
    bool waitForBackendReady(int timeoutMs = 10000);

    // Model management
    void listModels();
    void listLoadedModels();
    void loadModel(const QString &modelName, int nCtx = 4096, int nGpuLayers = -1);
    void unloadModel(const QString &modelName);

    // Inference (streaming)
    void chatCompletion(const QString &message, float temperature = 0.7, int maxTokens = 512);
    void chatCompletion(const QString &modelName, const QJsonArray &messages,
                       float temperature = 0.7, int maxTokens = 512);
    void completion(const QString &modelName, const QString &prompt,
                   float temperature = 0.7, int maxTokens = 512);

    // Knowledge management
    void ingestKnowledge(const QString &sourceName, const QString &content);
    void listKnowledge();
    void searchKnowledge(const QString &query, int topK = 5);
    void deleteKnowledge(int sourceId);

    // System info
    void getSystemInfo();
    void getSystemStats();

    // Health check
    void checkHealth();

signals:
    // Backend status
    void backendStarted();
    void backendStopped();
    void backendError(const QString &error);
    void backendReady();

    // Model management responses
    void modelsListed(const QJsonArray &models);
    void loadedModelsListed(const QJsonArray &models);
    void modelLoaded(const QString &modelName);
    void modelUnloaded(const QString &modelName);
    void modelError(const QString &error);

    // Inference responses (streaming)
    void streamToken(const QString &token);
    void streamComplete();
    void streamError(const QString &error);

    // Knowledge management responses
    void knowledgeIngested(const QString &sourceName, int chunkCount);
    void knowledgeListed(const QJsonArray &sources);
    void knowledgeSearchResults(const QJsonArray &results);
    void knowledgeDeleted(int sourceId);
    void knowledgeError(const QString &error);

    // System info response
    void systemInfoReceived(const QJsonObject &info);
    void systemStatsReceived(const QJsonObject &stats);

    // Health check response
    void healthCheckResult(bool healthy, const QJsonObject &data);

private slots:
    void onBackendOutput();
    void onBackendError();
    void onBackendFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    // Helper methods
    void makeRequest(const QString &method, const QString &endpoint,
                    const QJsonObject &data = QJsonObject());
    void handleResponse(QNetworkReply *reply, const QString &requestType);
    void handleStreamingResponse(QNetworkReply *reply);
    QString findPythonExecutable();
    QString findBackendScript();

    // Members
    std::unique_ptr<QProcess> m_backendProcess;
    std::unique_ptr<QNetworkAccessManager> m_networkManager;
    QString m_baseUrl;
    bool m_backendRunning;
    QString m_pythonPath;
    QString m_backendScriptPath;
};

#endif // BACKEND_CLIENT_H

