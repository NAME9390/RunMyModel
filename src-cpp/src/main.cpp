#include <QApplication>
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebChannel>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>
#include "backend.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("RunMyModel Desktop");
    app.setApplicationVersion("1.0.0");
    app.setOrganizationName("RunMyModel.org");
    
    // Create main window
    QWebEngineView *webView = new QWebEngineView();
    webView->setWindowTitle("RunMyModel Desktop");
    webView->resize(1200, 800);
    
    // Create backend
    Backend *backend = new Backend();
    
    // Setup web channel for communication
    QWebChannel *channel = new QWebChannel();
    channel->registerObject("backend", backend);
    
    QWebEnginePage *page = webView->page();
    page->setWebChannel(channel);
    
    // Load the frontend
    QString frontendPath = QDir::currentPath() + "/dist/index.html";
    if (QFile::exists(frontendPath)) {
        page->load(QUrl::fromLocalFile(frontendPath));
    } else {
        // Fallback to a simple HTML page if dist doesn't exist
        QString html = R"(
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
        page->setHtml(html);
    }
    
    // Show the window
    webView->show();
    
    // Start backend services
    backend->startWebServer();
    
    int result = app.exec();
    
    // Cleanup
    backend->stopWebServer();
    delete backend;
    delete channel;
    delete webView;
    
    return result;
}
