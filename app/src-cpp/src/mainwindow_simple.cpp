/**
 * RunMyModel Desktop - Pure C++ Edition
 * Simple chat with TinyLlama
 */

#include "mainwindow.h"
#include "llama_engine.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QLineEdit>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
{
    setWindowTitle("RunMyModel Desktop - Pure C++ Edition");
    resize(1200, 800);
    
    // Create llama engine
    m_llamaEngine = new LlamaEngine(this);
    
    // Setup UI
    QWidget *central = new QWidget(this);
    setCentralWidget(central);
    
    QVBoxLayout *layout = new QVBoxLayout(central);
    
    // Header
    QLabel *header = new QLabel("🦙 TinyLlama Chat - Pure C++ Edition");
    header->setStyleSheet("font-size: 24px; font-weight: bold; color: #6366F1; padding: 20px;");
    layout->addWidget(header);
    
    // Chat display
    m_chatDisplay = new QTextEdit();
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setStyleSheet(
        "QTextEdit {"
        "  background: #1e293b;"
        "  color: #e2e8f0;"
        "  border: none;"
        "  border-radius: 10px;"
        "  padding: 20px;"
        "  font-size: 14px;"
        "  font-family: 'Segoe UI', sans-serif;"
        "}"
    );
    layout->addWidget(m_chatDisplay);
    
    // Input area
    QHBoxLayout *inputLayout = new QHBoxLayout();
    
    m_messageInput = new QLineEdit();
    m_messageInput->setPlaceholderText("Type your message...");
    m_messageInput->setStyleSheet(
        "QLineEdit {"
        "  background: #334155;"
        "  color: #e2e8f0;"
        "  border: 2px solid #475569;"
        "  border-radius: 8px;"
        "  padding: 12px 20px;"
        "  font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "  border: 2px solid #6366F1;"
        "}"
    );
    inputLayout->addWidget(m_messageInput);
    
    QPushButton *sendBtn = new QPushButton("Send");
    sendBtn->setStyleSheet(
        "QPushButton {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #6366F1, stop:1 #4F46E5);"
        "  color: white;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 12px 30px;"
        "  font-size: 14px;"
        "  font-weight: 600;"
        "}"
        "QPushButton:hover {"
        "  background: qlineargradient(x1:0, y1:0, x2:0, y2:1, stop:0 #4F46E5, stop:1 #4338CA);"
        "}"
    );
    inputLayout->addWidget(sendBtn);
    
    layout->addLayout(inputLayout);
    
    // Connections
    connect(sendBtn, &QPushButton::clicked, this, &MainWindow::sendMessage);
    connect(m_messageInput, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);
    connect(m_llamaEngine, &LlamaEngine::tokenGenerated, this, &MainWindow::onTokenReceived);
    connect(m_llamaEngine, &LlamaEngine::responseComplete, this, &MainWindow::onResponseComplete);
    connect(m_llamaEngine, &LlamaEngine::error, this, &MainWindow::onError);
    
    // Load model
    QString modelPath = QCoreApplication::applicationDirPath() + "/../models/tinyllama.gguf";
    if (!m_llamaEngine->loadModel(modelPath)) {
        m_chatDisplay->append("<span style='color: #ef4444;'>❌ Failed to load model</span>");
    } else {
        m_chatDisplay->append(
            "<div style='background: rgba(16, 185, 129, 0.1); padding: 15px; border-radius: 10px; margin: 10px 0;'>"
            "<b style='color: #10B981;'>✅ TinyLlama Ready!</b><br>"
            "<span style='color: #E2E8F0;'>Model loaded successfully. Start chatting!</span>"
            "</div>"
        );
    }
}

MainWindow::~MainWindow() {
}

void MainWindow::sendMessage() {
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty()) return;
    
    // Add user message
    m_chatDisplay->append(
        "<div style='background: rgba(99, 102, 241, 0.1); padding: 15px; border-radius: 10px; margin: 10px 0;'>"
        "<b style='color: #6366F1;'>You:</b><br>"
        "<span style='color: #E2E8F0;'>" + message.toHtmlEscaped() + "</span>"
        "</div>"
    );
    
    m_messageInput->clear();
    m_currentResponse.clear();
    
    // Start AI response
    m_chatDisplay->append(
        "<div style='background: rgba(16, 185, 129, 0.1); padding: 15px; border-radius: 10px; margin: 10px 0;'>"
        "<b style='color: #10B981;'>TinyLlama:</b><br>"
        "<span style='color: #E2E8F0;'>"
    );
    
    m_llamaEngine->generateResponse(message);
}

void MainWindow::onTokenReceived(const QString &token) {
    m_currentResponse += token;
    m_chatDisplay->insertPlainText(token);
}

void MainWindow::onResponseComplete() {
    m_chatDisplay->append("</span></div>");
}

void MainWindow::onError(const QString &error) {
    m_chatDisplay->append(
        "<div style='background: rgba(239, 68, 68, 0.1); padding: 15px; border-radius: 10px; margin: 10px 0;'>"
        "<b style='color: #EF4444;'>Error:</b><br>"
        "<span style='color: #E2E8F0;'>" + error.toHtmlEscaped() + "</span>"
        "</div>"
    );
}

