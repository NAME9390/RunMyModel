#include <QApplication>
#include "backend.h"
#include <QWebEngineView>
#include <QWebEngineProfile>
#include <QWebChannel>
#include <QWebEngineScript>
#include <QWebEngineProfile>
#include <QtWebEngineCore/QWebEngineScriptCollection>
#include <QDir>
#include <QStandardPaths>
#include <QMessageBox>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("RunMyModel Desktop");
    app.setApplicationVersion("0.2.0-ALPHA");
    app.setOrganizationName("RunMyModel.org");
    
    // Create main window
    QWebEngineView *webView = new QWebEngineView();
    webView->setWindowTitle("RunMyModel Desktop");
    webView->resize(1200, 800);
    
    // Start backend server
    Backend *backend = new Backend();
    backend->startWebServer();

    // Load frontend from filesystem
    QString frontendPath = QDir::currentPath() + "/dist/index.html";
    
    if (QFile::exists(frontendPath)) {
        // Load from filesystem
        // Inject backend port so the frontend uses the correct API base
        {
            QWebEngineScript script;
            script.setName("inject-backend-port");
            script.setInjectionPoint(QWebEngineScript::DocumentCreation);
            script.setWorldId(QWebEngineScript::MainWorld);
            const QString code = QString("window.__BACKEND_PORT__=%1;").arg(backend->getWebServerPort());
            script.setSourceCode(code);
            webView->page()->profile()->scripts()->insert(script);
        }
        QUrl url = QUrl::fromLocalFile(frontendPath);
        webView->load(url);
        qDebug() << "Loading frontend from filesystem:" << frontendPath;
    } else {
        // Simple fallback HTML
        QString html = "<!DOCTYPE html><html><head><title>RunMyModel Desktop</title></head><body><h1>RunMyModel Desktop</h1><p>Qt Desktop Application is running!</p><p>Frontend files not found at: " + frontendPath + "</p></body></html>";
        webView->setHtml(html);
        qDebug() << "Using simple fallback HTML";
    }
    
    // Show the window
    webView->show();
    
    int result = app.exec();
    
    // Cleanup
    backend->stopWebServer();
    delete backend;
    delete webView;
    
    return result;
}