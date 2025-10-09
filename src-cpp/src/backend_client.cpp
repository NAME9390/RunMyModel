#include "backend_client.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QThread>
#include <QTimer>
#include <QEventLoop>
#include <QDebug>

BackendClient::BackendClient(QObject *parent)
    : QObject(parent)
    , m_backendProcess(new QProcess(this))
    , m_networkManager(new QNetworkAccessManager(this))
    , m_baseUrl("http://127.0.0.1:5000")
    , m_backendRunning(false)
{
    // Find Python executable
    m_pythonPath = findPythonExecutable();
    m_backendScriptPath = findBackendScript();

    // Connect process signals
    connect(m_backendProcess.get(), &QProcess::readyReadStandardOutput,
            this, &BackendClient::onBackendOutput);
    connect(m_backendProcess.get(), &QProcess::readyReadStandardError,
            this, &BackendClient::onBackendError);
    connect(m_backendProcess.get(), QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &BackendClient::onBackendFinished);
}

BackendClient::~BackendClient()
{
    stopBackend();
}

QString BackendClient::findPythonExecutable()
{
    // Try to find Python executable
    QStringList candidates = {
        "python3",
        "python",
        "/usr/bin/python3",
        "/usr/local/bin/python3",
        "C:/Python311/python.exe",
        "C:/Python310/python.exe",
    };

    for (const QString &candidate : candidates) {
        QProcess testProcess;
        testProcess.start(candidate, {"--version"});
        if (testProcess.waitForFinished(1000) && testProcess.exitCode() == 0) {
            qDebug() << "Found Python:" << candidate;
            return candidate;
        }
    }

    qWarning() << "⚠️ No Python executable found!";
    return "python3"; // Fallback
}

QString BackendClient::findBackendScript()
{
    // Look for backend-python/main.py
    QStringList searchPaths = {
        QCoreApplication::applicationDirPath() + "/backend-python/main.py",
        QDir::currentPath() + "/backend-python/main.py",
        QCoreApplication::applicationDirPath() + "/../backend-python/main.py",
    };

    for (const QString &path : searchPaths) {
        if (QFile::exists(path)) {
            qDebug() << "Found backend script:" << path;
            return path;
        }
    }

    qWarning() << "⚠️ Backend script not found!";
    return QCoreApplication::applicationDirPath() + "/backend-python/main.py";
}

bool BackendClient::startBackend()
{
    if (m_backendRunning) {
        qDebug() << "Backend already running";
        return true;
    }

    if (!QFile::exists(m_backendScriptPath)) {
        QString error = QString("Backend script not found: %1").arg(m_backendScriptPath);
        qWarning() << error;
        emit backendError(error);
        return false;
    }

    qDebug() << "🚀 Starting Python backend...";
    qDebug() << "   Python:" << m_pythonPath;
    qDebug() << "   Script:" << m_backendScriptPath;

    // Start Python backend process
    QStringList args = {m_backendScriptPath};
    m_backendProcess->setProgram(m_pythonPath);
    m_backendProcess->setArguments(args);
    m_backendProcess->setWorkingDirectory(QFileInfo(m_backendScriptPath).absolutePath());

    m_backendProcess->start();

    if (!m_backendProcess->waitForStarted(5000)) {
        QString error = "Failed to start backend process";
        qWarning() << error;
        emit backendError(error);
        return false;
    }

    qDebug() << "✅ Backend process started (PID:" << m_backendProcess->processId() << ")";
    m_backendRunning = true;
    emit backendStarted();

    return true;
}

void BackendClient::stopBackend()
{
    if (!m_backendRunning || !m_backendProcess) {
        return;
    }

    qDebug() << "🛑 Stopping Python backend...";
    m_backendProcess->terminate();

    if (!m_backendProcess->waitForFinished(3000)) {
        qWarning() << "Backend didn't stop gracefully, killing...";
        m_backendProcess->kill();
        m_backendProcess->waitForFinished(1000);
    }

    m_backendRunning = false;
    qDebug() << "✅ Backend stopped";
    emit backendStopped();
}

bool BackendClient::isBackendRunning() const
{
    return m_backendRunning && m_backendProcess->state() == QProcess::Running;
}

bool BackendClient::waitForBackendReady(int timeoutMs)
{
    qDebug() << "⏳ Waiting for backend to be ready...";

    int elapsed = 0;
    int checkInterval = 500; // Check every 500ms

    while (elapsed < timeoutMs) {
        // Try health check
        QEventLoop loop;
        QNetworkRequest request(m_baseUrl + "/health");
        QNetworkReply *reply = m_networkManager->get(request);

        connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
        QTimer::singleShot(1000, &loop, &QEventLoop::quit); // 1 second timeout per check

        loop.exec();

        if (reply->error() == QNetworkReply::NoError) {
            qDebug() << "✅ Backend is ready!";
            reply->deleteLater();
            emit backendReady();
            return true;
        }

        reply->deleteLater();

        // Wait before next check
        QThread::msleep(checkInterval);
        elapsed += checkInterval;
    }

    qWarning() << "❌ Backend failed to become ready within" << timeoutMs << "ms";
    return false;
}

void BackendClient::listModels()
{
    makeRequest("GET", "/api/models");
}

void BackendClient::listLoadedModels()
{
    makeRequest("GET", "/api/models/loaded");
}

void BackendClient::loadModel(const QString &modelName, int nCtx, int nGpuLayers)
{
    QJsonObject data;
    data["model_name"] = modelName;
    data["n_ctx"] = nCtx;
    data["n_gpu_layers"] = nGpuLayers;

    makeRequest("POST", "/api/models/load", data);
}

void BackendClient::unloadModel(const QString &modelName)
{
    QJsonObject data;
    data["model_name"] = modelName;

    makeRequest("POST", "/api/models/unload", data);
}

void BackendClient::chatCompletion(const QString &modelName, const QJsonArray &messages,
                                   float temperature, int maxTokens)
{
    QJsonObject data;
    data["model"] = modelName;
    data["messages"] = messages;
    data["temperature"] = temperature;
    data["max_tokens"] = maxTokens;
    data["stream"] = true;

    // This will use streaming, handled differently
    QNetworkRequest request(m_baseUrl + "/api/chat/completions");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(data).toJson());
    handleStreamingResponse(reply);
}

void BackendClient::completion(const QString &modelName, const QString &prompt,
                               float temperature, int maxTokens)
{
    QJsonObject data;
    data["model"] = modelName;
    data["prompt"] = prompt;
    data["temperature"] = temperature;
    data["max_tokens"] = maxTokens;
    data["stream"] = true;

    QNetworkRequest request(m_baseUrl + "/api/completions");
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = m_networkManager->post(request, QJsonDocument(data).toJson());
    handleStreamingResponse(reply);
}

void BackendClient::getSystemInfo()
{
    makeRequest("GET", "/api/system/info");
}

void BackendClient::checkHealth()
{
    makeRequest("GET", "/health");
}

void BackendClient::makeRequest(const QString &method, const QString &endpoint,
                                const QJsonObject &data)
{
    QNetworkRequest request(m_baseUrl + endpoint);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QNetworkReply *reply = nullptr;

    if (method == "GET") {
        reply = m_networkManager->get(request);
    } else if (method == "POST") {
        reply = m_networkManager->post(request, QJsonDocument(data).toJson());
    } else if (method == "DELETE") {
        reply = m_networkManager->deleteResource(request);
    }

    if (reply) {
        connect(reply, &QNetworkReply::finished, [this, reply, endpoint]() {
            handleResponse(reply, endpoint);
        });
    }
}

void BackendClient::handleResponse(QNetworkReply *reply, const QString &requestType)
{
    reply->deleteLater();

    if (reply->error() != QNetworkReply::NoError) {
        QString error = QString("Network error: %1").arg(reply->errorString());
        qWarning() << error;
        emit backendError(error);
        return;
    }

    QByteArray responseData = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(responseData);

    if (doc.isNull()) {
        qWarning() << "Invalid JSON response";
        return;
    }

    QJsonObject obj = doc.object();

    // Route response based on endpoint
    if (requestType.contains("/models/loaded")) {
        emit loadedModelsListed(obj["loaded_models"].toArray());
    } else if (requestType.contains("/models/load")) {
        emit modelLoaded(obj["model"].toString());
    } else if (requestType.contains("/models/unload")) {
        emit modelUnloaded(obj["model_name"].toString());
    } else if (requestType.contains("/models")) {
        emit modelsListed(obj["models"].toArray());
    } else if (requestType.contains("/system/info")) {
        emit systemInfoReceived(obj);
    } else if (requestType.contains("/health")) {
        bool healthy = obj["status"].toString() == "healthy";
        emit healthCheckResult(healthy, obj);
    }
}

void BackendClient::handleStreamingResponse(QNetworkReply *reply)
{
    // Handle Server-Sent Events (SSE) streaming
    connect(reply, &QNetworkReply::readyRead, [this, reply]() {
        while (reply->canReadLine()) {
            QByteArray line = reply->readLine();
            QString lineStr = QString::fromUtf8(line).trimmed();

            if (lineStr.startsWith("data: ")) {
                QString jsonStr = lineStr.mid(6); // Remove "data: "

                if (jsonStr == "[DONE]") {
                    emit streamComplete();
                    continue;
                }

                QJsonDocument doc = QJsonDocument::fromJson(jsonStr.toUtf8());
                if (!doc.isNull()) {
                    QJsonObject obj = doc.object();

                    // Handle OpenAI-style streaming format
                    if (obj.contains("choices")) {
                        QJsonArray choices = obj["choices"].toArray();
                        if (!choices.isEmpty()) {
                            QJsonObject choice = choices[0].toObject();
                            QJsonObject delta = choice["delta"].toObject();
                            QString content = delta["content"].toString();

                            if (!content.isEmpty()) {
                                emit streamToken(content);
                            }
                        }
                    }

                    // Handle error
                    if (obj.contains("error")) {
                        emit streamError(obj["error"].toString());
                    }
                }
            }
        }
    });

    connect(reply, &QNetworkReply::finished, [reply]() {
        reply->deleteLater();
    });

    connect(reply, QOverload<QNetworkReply::NetworkError>::of(&QNetworkReply::errorOccurred),
            [this, reply](QNetworkReply::NetworkError error) {
        Q_UNUSED(error)
        emit streamError(reply->errorString());
    });
}

void BackendClient::onBackendOutput()
{
    QByteArray output = m_backendProcess->readAllStandardOutput();
    QString outputStr = QString::fromUtf8(output);

    // Log backend output
    for (const QString &line : outputStr.split('\n')) {
        if (!line.trimmed().isEmpty()) {
            qDebug() << "[Backend]" << line;
        }
    }
}

void BackendClient::onBackendError()
{
    QByteArray error = m_backendProcess->readAllStandardError();
    QString errorStr = QString::fromUtf8(error);

    // Log backend errors
    for (const QString &line : errorStr.split('\n')) {
        if (!line.trimmed().isEmpty()) {
            qWarning() << "[Backend Error]" << line;
        }
    }
}

void BackendClient::onBackendFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    m_backendRunning = false;

    if (exitStatus == QProcess::CrashExit) {
        QString error = QString("Backend crashed with exit code: %1").arg(exitCode);
        qWarning() << error;
        emit backendError(error);
    } else {
        qDebug() << "Backend exited normally with code:" << exitCode;
    }

    emit backendStopped();
}

