#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QJsonArray>
#include <QHttpServer>
#include <QWebEngineView>
#include <QWebEnginePage>
#include <QWebChannel>
#include <memory>

class Backend;

class WebServer : public QObject
{
    Q_OBJECT

public:
    explicit WebServer(QObject *parent = nullptr);
    ~WebServer();

    bool startServer(quint16 port = 8080);
    void stopServer();
    bool isRunning() const;

    void setBackend(Backend *backend);

private slots:
    void onWebChannelObjectReceived(const QJsonObject &message);

private:
    QHttpServer *m_httpServer;
    QWebEngineView *m_webView;
    QWebEnginePage *m_webPage;
    QWebChannel *m_webChannel;
    Backend *m_backend;
    quint16 m_port;
    bool m_running;

    void setupRoutes();
    void setupWebChannel();
    QJsonObject handleApiRequest(const QString &endpoint, const QJsonObject &data);
    
    // API endpoints
    QJsonObject handleGetSystemInfo();
    QJsonObject handleGetModels();
    QJsonObject handleDownloadModel(const QJsonObject &data);
    QJsonObject handleRemoveModel(const QJsonObject &data);
    QJsonObject handleChat(const QJsonObject &data);
    QJsonObject handleGetDownloadProgress(const QJsonObject &data);
};

#endif // WEB_SERVER_H
