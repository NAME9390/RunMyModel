#include "web_server.h"
#include "backend.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QTcpServer>
#include <QTcpSocket>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebChannel>
#include <QHostAddress>
#include <QDebug>
#include <QTextStream>

WebServer::WebServer(QObject *parent)
    : QObject(parent)
    , m_tcpServer(new QTcpServer(this))
    , m_webView(nullptr)
    , m_webPage(nullptr)
    , m_webChannel(nullptr)
    , m_backend(nullptr)
    , m_port(8080)
    , m_running(false)
{
    connect(m_tcpServer, &QTcpServer::newConnection, this, &WebServer::onNewConnection);
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
    
    // Try desired port; if busy, try ephemeral port 0
    if (!m_tcpServer->listen(QHostAddress::LocalHost, m_port)) {
        qWarning() << "Failed to start TCP server on port" << m_port << ", trying ephemeral port...";
        if (!m_tcpServer->listen(QHostAddress::LocalHost, 0)) {
            qWarning() << "Failed to start TCP server";
            return false;
        }
        m_port = m_tcpServer->serverPort();
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
    
    m_tcpServer->close();
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
    setupWebChannel();
}

void WebServer::onNewConnection()
{
    QTcpSocket *socket = m_tcpServer->nextPendingConnection();
    if (socket) {
        connect(socket, &QTcpSocket::readyRead, this, &WebServer::onSocketReadyRead);
        connect(socket, &QTcpSocket::disconnected, socket, &QTcpSocket::deleteLater);
    }
}

void WebServer::onSocketReadyRead()
{
    QTcpSocket *socket = qobject_cast<QTcpSocket*>(sender());
    if (!socket) return;
    
    QByteArray data = socket->readAll();
    QString request = QString::fromUtf8(data);
    
    handleHttpRequest(socket, request);
}

void WebServer::handleHttpRequest(QTcpSocket *socket, const QString &request)
{
    QStringList lines = request.split("\r\n");
    if (lines.isEmpty()) return;
    
    QString requestLine = lines.first();
    QStringList parts = requestLine.split(" ");
    if (parts.size() < 2) return;
    
    QString method = parts[0];
    QString path = parts[1];
    
    // Simple HTTP response
    QString response;
    QString contentType = "application/json";

    // Handle CORS preflight
    if (method == "OPTIONS") {
        QString preflight = QString("HTTP/1.1 204 No Content\r\n"
                                    "Access-Control-Allow-Origin: *\r\n"
                                    "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                                    "Access-Control-Allow-Headers: Content-Type\r\n"
                                    "Access-Control-Max-Age: 86400\r\n"
                                    "Content-Length: 0\r\n"
                                    "Connection: close\r\n\r\n");
        socket->write(preflight.toUtf8());
        socket->disconnectFromHost();
        return;
    }
    
    if (path.startsWith("/api/")) {
        // Handle API requests
        QJsonObject data;
        if (method == "POST") {
            // Extract JSON from request body
            QString body = request.split("\r\n\r\n").last();
            QJsonDocument doc = QJsonDocument::fromJson(body.toUtf8());
            data = doc.object();
        }
        
        QJsonObject result = handleApiRequest(path, data);
        response = QJsonDocument(result).toJson();
    } else {
        // Serve the main HTML page
        contentType = "text/html";
        response = R"(
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
        )";
    }
    
    // Send HTTP response
    QString httpResponse = QString("HTTP/1.1 200 OK\r\n"
                                  "Content-Type: %1\r\n"
                                  "Access-Control-Allow-Origin: *\r\n"
                                  "Access-Control-Allow-Methods: GET, POST, OPTIONS\r\n"
                                  "Access-Control-Allow-Headers: Content-Type\r\n"
                                  "Content-Length: %2\r\n"
                                  "Connection: close\r\n"
                                  "\r\n"
                                  "%3")
                          .arg(contentType)
                          .arg(response.length())
                          .arg(response);
    
    socket->write(httpResponse.toUtf8());
    socket->disconnectFromHost();
}

void WebServer::onWebChannelObjectReceived(const QJsonObject &message)
{
    Q_UNUSED(message)
    // Handle web channel messages if needed
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
    if (endpoint == "/api/system-info") {
        return handleGetSystemInfo();
    } else if (endpoint == "/api/models") {
        return handleGetModels();
    } else if (endpoint == "/api/download-model") {
        return handleDownloadModel(data);
    } else if (endpoint == "/api/remove-model") {
        return handleRemoveModel(data);
    } else if (endpoint == "/api/chat") {
        return handleChat(data);
    } else if (endpoint == "/api/download-progress") {
        return handleGetDownloadProgress(data);
    }
    
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