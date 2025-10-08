#include "web_server.h"
#include "backend.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QHttpServer>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebChannel>
#include <QTcpServer>
#include <QDebug>

WebServer::WebServer(QObject *parent)
    : QObject(parent)
    , m_httpServer(new QHttpServer(this))
    , m_webView(nullptr)
    , m_webPage(nullptr)
    , m_webChannel(nullptr)
    , m_backend(nullptr)
    , m_port(8080)
    , m_running(false)
{
    setupRoutes();
}

WebServer::~WebServer()
{
    stopServer();
}

bool WebServer::startServer(quint16 port)
{
    if (m_running) {
        return true;
    }
    
    m_port = port;
    
    // Create TCP server and bind it
    QTcpServer *tcpServer = new QTcpServer(this);
    if (!tcpServer->listen(QHostAddress::LocalHost, m_port)) {
        qWarning() << "Failed to start TCP server on port" << m_port;
        delete tcpServer;
        return false;
    }
    
    // Bind the HTTP server to the TCP server
    if (!m_httpServer->bind(tcpServer)) {
        qWarning() << "Failed to bind HTTP server to TCP server";
        delete tcpServer;
        return false;
    }
    
    m_running = true;
    qDebug() << "Web server started on port" << m_port;
    
    return true;
}

void WebServer::stopServer()
{
    if (!m_running) {
        return;
    }
    
    m_httpServer->deleteLater();
    m_httpServer = new QHttpServer(this);
    
    m_running = false;
    qDebug() << "Web server stopped";
}

bool WebServer::isRunning() const
{
    return m_running;
}

void WebServer::setBackend(Backend *backend)
{
    m_backend = backend;
}

void WebServer::onWebChannelObjectReceived(const QJsonObject &message)
{
    Q_UNUSED(message)
    // Handle web channel messages if needed
}

void WebServer::setupRoutes()
{
    // API routes
    m_httpServer->route("/api/system-info", QHttpServerRequest::Method::Get, [this]() {
        return QJsonDocument(handleGetSystemInfo()).toJson();
    });
    
    m_httpServer->route("/api/models", QHttpServerRequest::Method::Get, [this]() {
        return QJsonDocument(handleGetModels()).toJson();
    });
    
    m_httpServer->route("/api/download-model", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        QJsonObject data = doc.object();
        return QJsonDocument(handleDownloadModel(data)).toJson();
    });
    
    m_httpServer->route("/api/remove-model", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        QJsonObject data = doc.object();
        return QJsonDocument(handleRemoveModel(data)).toJson();
    });
    
    m_httpServer->route("/api/chat", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        QJsonObject data = doc.object();
        return QJsonDocument(handleChat(data)).toJson();
    });
    
    m_httpServer->route("/api/download-progress", QHttpServerRequest::Method::Post, [this](const QHttpServerRequest &request) {
        QJsonDocument doc = QJsonDocument::fromJson(request.body());
        QJsonObject data = doc.object();
        return QJsonDocument(handleGetDownloadProgress(data)).toJson();
    });
    
    // Serve static files
    m_httpServer->route("/", QHttpServerRequest::Method::Get, [this]() {
        return QHttpServerResponse("text/html", R"(
<!DOCTYPE html>
<html>
<head>
    <title>RunMyModel Desktop</title>
    <meta charset="utf-8">
    <script src="qrc:///qtwebchannel/qwebchannel.js"></script>
</head>
<body>
    <h1>RunMyModel Desktop</h1>
    <p>Loading...</p>
    <script>
        new QWebChannel(qt.webChannelTransport, function (channel) {
            window.backend = channel.objects.backend;
            console.log('Backend connected:', window.backend);
            
            // Test backend connection
            if (window.backend) {
                window.backend.getSystemInfo().then(function(info) {
                    console.log('System info:', info);
                    document.body.innerHTML = '<h1>RunMyModel Desktop</h1><p>Backend connected successfully!</p><pre>' + JSON.stringify(info, null, 2) + '</pre>';
                }).catch(function(error) {
                    console.error('Error getting system info:', error);
                    document.body.innerHTML = '<h1>RunMyModel Desktop</h1><p>Error connecting to backend: ' + error + '</p>';
                });
            }
        });
    </script>
</body>
</html>
        )");
    });
}

void WebServer::setupWebChannel()
{
    if (!m_webChannel) {
        m_webChannel = new QWebChannel(this);
    }
    
    if (m_backend) {
        m_webChannel->registerObject("backend", m_backend);
    }
}

QJsonObject WebServer::handleApiRequest(const QString &endpoint, const QJsonObject &data)
{
    Q_UNUSED(endpoint)
    Q_UNUSED(data)
    
    QJsonObject response;
    response["error"] = "Unknown endpoint";
    return response;
}

QJsonObject WebServer::handleGetSystemInfo()
{
    if (!m_backend) {
        QJsonObject error;
        error["error"] = "Backend not available";
        return error;
    }
    
    return m_backend->getSystemInfo();
}

QJsonObject WebServer::handleGetModels()
{
    if (!m_backend) {
        QJsonObject error;
        error["error"] = "Backend not available";
        return error;
    }
    
    QJsonObject response;
    response["models"] = m_backend->getAllHuggingFaceModels();
    return response;
}

QJsonObject WebServer::handleDownloadModel(const QJsonObject &data)
{
    if (!m_backend) {
        QJsonObject error;
        error["error"] = "Backend not available";
        return error;
    }
    
    QString modelName = data["model_name"].toString();
    QString result = m_backend->downloadHuggingFaceModel(modelName);
    
    QJsonObject response;
    response["message"] = result;
    response["success"] = !result.contains("Failed") && !result.contains("Error");
    
    return response;
}

QJsonObject WebServer::handleRemoveModel(const QJsonObject &data)
{
    if (!m_backend) {
        QJsonObject error;
        error["error"] = "Backend not available";
        return error;
    }
    
    QString modelName = data["model_name"].toString();
    QString result = m_backend->removeHuggingFaceModel(modelName);
    
    QJsonObject response;
    response["message"] = result;
    response["success"] = !result.contains("Failed") && !result.contains("Error");
    
    return response;
}

QJsonObject WebServer::handleChat(const QJsonObject &data)
{
    if (!m_backend) {
        QJsonObject error;
        error["error"] = "Backend not available";
        return error;
    }
    
    return m_backend->chatWithHuggingFace(data);
}

QJsonObject WebServer::handleGetDownloadProgress(const QJsonObject &data)
{
    if (!m_backend) {
        QJsonObject error;
        error["error"] = "Backend not available";
        return error;
    }
    
    QString modelName = data["model_name"].toString();
    return m_backend->getModelDownloadProgress(modelName);
}
