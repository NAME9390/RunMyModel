#include "mainwindow.h"
#include <QApplication>
#include <QIcon>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    // Set application properties
    app.setApplicationName("RunMyModel Desktop");
    app.setApplicationVersion("0.2.0-ALPHA");
    app.setOrganizationName("RunMyModel.org");
    
    qDebug() << "Starting RunMyModel Desktop v0.2.0-ALPHA";
    qDebug() << "Native Qt Widgets UI - No web dependencies";
    
    // Create and show main window
    MainWindow window;
    window.show();
    
    return app.exec();
}
