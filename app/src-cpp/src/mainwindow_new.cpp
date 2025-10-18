/*
 * This file contains an older version of MainWindow that conflicts with the main implementation.
 * It has been commented out to prevent compilation errors.
 * The main MainWindow implementation is in mainwindow.cpp
 */

/*
#include "mainwindow.h"
#include <QApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QScrollBar>
#include <QGroupBox>
#include <QFormLayout>
#include <QFile>
#include <QJsonDocument>
#include <QDebug>

MainWindow::MainWindow(BackendClient *backendClient, QWidget *parent)
    : QMainWindow(parent)
    , m_backendClient(backendClient)
{
    setWindowTitle("RunMyModel - Self-Learning LLM Platform v0.3.0");
    resize(1400, 900);
    
    loadConfig();
    setupUI();
    applyModernStyling();
    
    // Connect backend if available
    if (m_backendClient) {
        connect(m_backendClient, &BackendClient::streamToken, this, &MainWindow::onStreamToken);
        connect(m_backendClient, &BackendClient::streamComplete, this, &MainWindow::onStreamComplete);
        connect(m_backendClient, &BackendClient::streamError, this, &MainWindow::onStreamError);
        connect(m_backendClient, &BackendClient::modelLoaded, this, &MainWindow::onModelLoaded);
        
        updateBackendStatus("Connected", "#10b981");
    } else {
        updateBackendStatus("Disconnected", "#ef4444");
    }
}

MainWindow::~MainWindow()
{
    saveConfig();
}

// ... rest of the implementation would be here ...
*/