#include "mainwindow.h"
#include "backend_client.h"
#include <QApplication>
#include <QIcon>
#include <QDebug>
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("RunMyModel Desktop");
    app.setApplicationVersion("0.3.0-BETA");
    app.setOrganizationName("RunMyModel.org");
    
    qDebug() << "🚀 RunMyModel Desktop v0.3.0-BETA";
    qDebug() << "🔧 Architecture: C++ Qt6 Frontend + Python FastAPI Backend";
    qDebug() << "🧠 AI Engine: llama.cpp (GGUF models)";

    // Start Python backend
    BackendClient *backendClient = new BackendClient(&app);
    
    qDebug() << "📡 Starting Python backend...";
    if (!backendClient->startBackend()) {
        QMessageBox::critical(nullptr, "Backend Error",
            "Failed to start Python backend.\n\n"
            "Make sure Python 3 is installed and requirements are met:\n"
            "pip install -r backend-python/requirements.txt");
        return 1;
    }

    // Wait for backend to be ready
    if (!backendClient->waitForBackendReady(15000)) {
        QMessageBox::warning(nullptr, "Backend Timeout",
            "Python backend took too long to start.\n\n"
            "The app will continue, but features may not work.\n"
            "Check console for errors.");
    }

    // Create and show main window
    MainWindow window(backendClient);
    window.show();

    int result = app.exec();

    // Cleanup
    qDebug() << "🛑 Shutting down...";
    backendClient->stopBackend();

    return result;
}
