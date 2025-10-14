#include "mainwindow.h"
#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    
    app.setApplicationName("RunMyModel Desktop");
    app.setApplicationVersion("0.4.0-Pure-C++");
    app.setOrganizationName("RunMyModel.org");
    
    qDebug() << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";
    qDebug() << "🚀 RunMyModel Desktop v0.4.0 - Pure C++ Edition";
    qDebug() << "🦙 TinyLlama-1.1B Chat";
    qDebug() << "━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━";

    MainWindow window;
    window.show();

    return app.exec();
}

