#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QScrollBar>
#include <QFileDialog>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_llamaEngine(new LlamaEngine(this))
    , m_isGenerating(false)
    , m_tokenCount(0)
    , m_temperature(0.8f)
    , m_maxTokens(512)
{
    qDebug() << "🏗️  Constructing MainWindow...";
    
    setWindowTitle("RunMyModel 0.4.0 - Pure C++ LLM Platform");
    resize(1200, 800);
    
    setupUI();
    
    // Connect LlamaEngine signals
    connect(m_llamaEngine, &LlamaEngine::tokenGenerated, this, &MainWindow::onTokenReceived);
    connect(m_llamaEngine, &LlamaEngine::responseComplete, this, &MainWindow::onResponseComplete);
    connect(m_llamaEngine, &LlamaEngine::error, this, &MainWindow::onError);
    
    qDebug() << "✅ MainWindow constructed";
    
    // Load available models
    loadAvailableModels();
    
    // Auto-load TinyLlama if available
    QString defaultModel = "models/tinyllama.gguf";
    if (QFile::exists(defaultModel)) {
        m_statusLabel->setText("⏳ Auto-loading TinyLlama...");
        if (m_llamaEngine->loadModel(defaultModel, 2048, 4)) {
            m_currentModelPath = defaultModel;
            m_currentModelLabel->setText("✅ Loaded: tinyllama.gguf");
            m_statusLabel->setText("✅ Ready - Model loaded with GPU acceleration");
            appendMessage("TinyLlama-1.1B loaded successfully with GPU acceleration!", "System");
            appendMessage("Type your message below and press Enter or Send.", "System");
        } else {
            m_statusLabel->setText("❌ Failed to load model");
            appendMessage("Failed to load model. Try loading manually from Models tab.", "System");
        }
    } else {
        m_statusLabel->setText("⚠️  No model loaded");
        appendMessage("Welcome! Please load a model from the Models tab to get started.", "System");
    }
}

MainWindow::~MainWindow() {
    qDebug() << "🧹 Destroying MainWindow";
}

void MainWindow::setupUI() {
    // Set dark theme stylesheet
    setStyleSheet(R"(
        QMainWindow {
            background: #1e1e1e;
        }
        QTabWidget::pane {
            border: 1px solid #3d3d3d;
            background: #1e1e1e;
        }
        QTabBar::tab {
            background: #2d2d2d;
            color: #ffffff;
            padding: 10px 20px;
            margin-right: 2px;
            border-top-left-radius: 4px;
            border-top-right-radius: 4px;
        }
        QTabBar::tab:selected {
            background: #0078d4;
        }
        QTabBar::tab:hover {
            background: #3d3d3d;
        }
    )");
    
    m_tabWidget = new QTabWidget(this);
    setCentralWidget(m_tabWidget);
    
    createChatTab();
    createSettingsTab();
    createModelsTab();
    
    m_tabWidget->setCurrentIndex(0);
}

void MainWindow::createChatTab() {
    QWidget *chatWidget = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(chatWidget);
    layout->setSpacing(10);
    layout->setContentsMargins(15, 15, 15, 15);
    
    // Status and stats bar
    QHBoxLayout *statusLayout = new QHBoxLayout();
    
    m_statusLabel = new QLabel("Initializing...");
    m_statusLabel->setStyleSheet(
        "QLabel {"
        "   padding: 8px 12px;"
        "   background: #2d2d2d;"
        "   color: #00ff00;"
        "   border-radius: 4px;"
        "   font-family: 'Monospace';"
        "   font-size: 11px;"
        "   font-weight: bold;"
        "}"
    );
    statusLayout->addWidget(m_statusLabel, 1);
    
    m_statsLabel = new QLabel("Tokens: 0 | Speed: 0 t/s");
    m_statsLabel->setStyleSheet(
        "QLabel {"
        "   padding: 8px 12px;"
        "   background: #2d2d2d;"
        "   color: #00bfff;"
        "   border-radius: 4px;"
        "   font-family: 'Monospace';"
        "   font-size: 11px;"
        "}"
    );
    statusLayout->addWidget(m_statsLabel);
    
    layout->addLayout(statusLayout);
    
    // Chat display
    m_chatDisplay = new QTextEdit();
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setStyleSheet(
        "QTextEdit {"
        "   background: #0d1117;"
        "   color: #ffffff;"
        "   border: 2px solid #30363d;"
        "   border-radius: 6px;"
        "   padding: 15px;"
        "   font-family: 'Consolas', 'Monaco', monospace;"
        "   font-size: 14px;"
        "   line-height: 1.6;"
        "}"
        "QScrollBar:vertical {"
        "   background: #1e1e1e;"
        "   width: 12px;"
        "   border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical {"
        "   background: #3d3d3d;"
        "   border-radius: 6px;"
        "}"
        "QScrollBar::handle:vertical:hover {"
        "   background: #4d4d4d;"
        "}"
    );
    layout->addWidget(m_chatDisplay, 1);
    
    // Action buttons
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    
    m_clearButton = new QPushButton("🗑️ Clear");
    m_clearButton->setStyleSheet(
        "QPushButton {"
        "   background: #3d3d3d;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 8px 16px;"
        "   font-size: 12px;"
        "}"
        "QPushButton:hover { background: #4d4d4d; }"
    );
    connect(m_clearButton, &QPushButton::clicked, this, &MainWindow::onClearChat);
    actionsLayout->addWidget(m_clearButton);
    
    m_saveButton = new QPushButton("💾 Save");
    m_saveButton->setStyleSheet(m_clearButton->styleSheet());
    connect(m_saveButton, &QPushButton::clicked, this, &MainWindow::onSaveChat);
    actionsLayout->addWidget(m_saveButton);
    
    actionsLayout->addStretch();
    layout->addLayout(actionsLayout);
    
    // Input area
    QHBoxLayout *inputLayout = new QHBoxLayout();
    
    m_messageInput = new QLineEdit();
    m_messageInput->setPlaceholderText("Type your message here...");
    m_messageInput->setStyleSheet(
        "QLineEdit {"
        "   background: #2d2d2d;"
        "   color: #ffffff;"
        "   border: 2px solid #3d3d3d;"
        "   border-radius: 6px;"
        "   padding: 12px 15px;"
        "   font-size: 14px;"
        "}"
        "QLineEdit:focus {"
        "   border-color: #0078d4;"
        "}"
    );
    connect(m_messageInput, &QLineEdit::returnPressed, this, &MainWindow::sendMessage);
    inputLayout->addWidget(m_messageInput, 1);
    
    m_stopButton = new QPushButton("⏹️ Stop");
    m_stopButton->setEnabled(false);
    m_stopButton->setStyleSheet(
        "QPushButton {"
        "   background: #d32f2f;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 6px;"
        "   padding: 12px 20px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background: #f44336; }"
        "QPushButton:disabled {"
        "   background: #3d3d3d;"
        "   color: #666666;"
        "}"
    );
    connect(m_stopButton, &QPushButton::clicked, this, &MainWindow::onStopGeneration);
    inputLayout->addWidget(m_stopButton);
    
    m_sendButton = new QPushButton("📤 Send");
    m_sendButton->setStyleSheet(
        "QPushButton {"
        "   background: #0078d4;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 6px;"
        "   padding: 12px 24px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background: #106ebe; }"
        "QPushButton:pressed { background: #005a9e; }"
        "QPushButton:disabled {"
        "   background: #3d3d3d;"
        "   color: #666666;"
        "}"
    );
    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::sendMessage);
    inputLayout->addWidget(m_sendButton);
    
    layout->addLayout(inputLayout);
    
    m_tabWidget->addTab(chatWidget, "💬 Chat");
}

void MainWindow::createSettingsTab() {
    QWidget *settingsWidget = new QWidget();
    settingsWidget->setStyleSheet("background: #1e1e1e; color: white;");
    
    QVBoxLayout *layout = new QVBoxLayout(settingsWidget);
    layout->setSpacing(20);
    layout->setContentsMargins(30, 30, 30, 30);
    
    // Header
    QLabel *header = new QLabel("⚙️ Generation Settings");
    header->setStyleSheet("font-size: 20px; font-weight: bold; color: #0078d4;");
    layout->addWidget(header);
    
    // Temperature setting
    QVBoxLayout *tempLayout = new QVBoxLayout();
    m_temperatureLabel = new QLabel(QString("🌡️ Temperature: %1").arg(m_temperature, 0, 'f', 2));
    m_temperatureLabel->setStyleSheet("font-size: 14px; margin-bottom: 5px;");
    tempLayout->addWidget(m_temperatureLabel);
    
    m_temperatureSlider = new QSlider(Qt::Horizontal);
    m_temperatureSlider->setRange(0, 200);
    m_temperatureSlider->setValue(80);
    m_temperatureSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "   height: 8px;"
        "   background: #2d2d2d;"
        "   border-radius: 4px;"
        "}"
        "QSlider::handle:horizontal {"
        "   background: #0078d4;"
        "   width: 18px;"
        "   margin: -5px 0;"
        "   border-radius: 9px;"
        "}"
    );
    connect(m_temperatureSlider, &QSlider::valueChanged, this, &MainWindow::onTemperatureChanged);
    tempLayout->addWidget(m_temperatureSlider);
    
    QLabel *tempHelp = new QLabel("Lower = more focused, Higher = more creative");
    tempHelp->setStyleSheet("font-size: 11px; color: #888888;");
    tempLayout->addWidget(tempHelp);
    
    layout->addLayout(tempLayout);
    
    // Max tokens setting
    QVBoxLayout *tokensLayout = new QVBoxLayout();
    QLabel *tokensLabel = new QLabel("📏 Max Tokens:");
    tokensLabel->setStyleSheet("font-size: 14px; margin-bottom: 5px;");
    tokensLayout->addWidget(tokensLabel);
    
    m_maxTokensSpinBox = new QSpinBox();
    m_maxTokensSpinBox->setRange(50, 4096);
    m_maxTokensSpinBox->setValue(512);
    m_maxTokensSpinBox->setSingleStep(50);
    m_maxTokensSpinBox->setStyleSheet(
        "QSpinBox {"
        "   background: #2d2d2d;"
        "   color: white;"
        "   border: 2px solid #3d3d3d;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-size: 14px;"
        "}"
    );
    connect(m_maxTokensSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), 
            this, &MainWindow::onMaxTokensChanged);
    tokensLayout->addWidget(m_maxTokensSpinBox);
    
    QLabel *tokensHelp = new QLabel("Maximum length of generated responses");
    tokensHelp->setStyleSheet("font-size: 11px; color: #888888;");
    tokensLayout->addWidget(tokensHelp);
    
    layout->addLayout(tokensLayout);
    
    layout->addStretch();
    
    m_tabWidget->addTab(settingsWidget, "⚙️ Settings");
}

void MainWindow::createModelsTab() {
    QWidget *modelsWidget = new QWidget();
    modelsWidget->setStyleSheet("background: #1e1e1e; color: white;");
    
    QVBoxLayout *layout = new QVBoxLayout(modelsWidget);
    layout->setSpacing(15);
    layout->setContentsMargins(20, 20, 20, 20);
    
    // Header
    QLabel *header = new QLabel("🤖 Model Management");
    header->setStyleSheet("font-size: 20px; font-weight: bold; color: #0078d4;");
    layout->addWidget(header);
    
    // Current model
    m_currentModelLabel = new QLabel("No model loaded");
    m_currentModelLabel->setStyleSheet(
        "QLabel {"
        "   background: #2d2d2d;"
        "   padding: 12px;"
        "   border-radius: 4px;"
        "   font-size: 13px;"
        "}"
    );
    layout->addWidget(m_currentModelLabel);
    
    // Progress bar
    m_modelLoadProgress = new QProgressBar();
    m_modelLoadProgress->setVisible(false);
    m_modelLoadProgress->setStyleSheet(
        "QProgressBar {"
        "   background: #2d2d2d;"
        "   border-radius: 4px;"
        "   height: 20px;"
        "}"
        "QProgressBar::chunk {"
        "   background: #0078d4;"
        "   border-radius: 4px;"
        "}"
    );
    layout->addWidget(m_modelLoadProgress);
    
    // Available models list
    QLabel *listLabel = new QLabel("📁 Available Models:");
    listLabel->setStyleSheet("font-size: 14px; margin-top: 10px;");
    layout->addWidget(listLabel);
    
    m_modelsList = new QListWidget();
    m_modelsList->setStyleSheet(
        "QListWidget {"
        "   background: #0d1117;"
        "   border: 2px solid #30363d;"
        "   border-radius: 6px;"
        "   padding: 8px;"
        "   font-size: 13px;"
        "}"
        "QListWidget::item {"
        "   padding: 8px;"
        "   border-radius: 4px;"
        "   margin: 2px 0;"
        "}"
        "QListWidget::item:selected {"
        "   background: #0078d4;"
        "}"
        "QListWidget::item:hover {"
        "   background: #2d2d2d;"
        "}"
    );
    layout->addWidget(m_modelsList, 1);
    
    // Buttons
    QHBoxLayout *buttonsLayout = new QHBoxLayout();
    
    m_loadModelButton = new QPushButton("📥 Load Selected");
    m_loadModelButton->setStyleSheet(
        "QPushButton {"
        "   background: #0078d4;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 10px 20px;"
        "   font-size: 13px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background: #106ebe; }"
    );
    connect(m_loadModelButton, &QPushButton::clicked, this, &MainWindow::onLoadModel);
    buttonsLayout->addWidget(m_loadModelButton);
    
    m_unloadModelButton = new QPushButton("🗑️ Unload Model");
    m_unloadModelButton->setStyleSheet(
        "QPushButton {"
        "   background: #d32f2f;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 10px 20px;"
        "   font-size: 13px;"
        "}"
        "QPushButton:hover { background: #f44336; }"
    );
    connect(m_unloadModelButton, &QPushButton::clicked, this, &MainWindow::onUnloadModel);
    buttonsLayout->addWidget(m_unloadModelButton);
    
    layout->addLayout(buttonsLayout);
    
    m_tabWidget->addTab(modelsWidget, "🤖 Models");
}

void MainWindow::loadAvailableModels() {
    m_modelsList->clear();
    
    QDir modelsDir("models");
    if (!modelsDir.exists()) {
        modelsDir.mkpath(".");
    }
    
    QStringList filters;
    filters << "*.gguf" << "*.bin";
    QFileInfoList models = modelsDir.entryInfoList(filters, QDir::Files);
    
    for (const QFileInfo &model : models) {
        QString displayText = QString("%1 (%2 MB)")
            .arg(model.fileName())
            .arg(model.size() / (1024 * 1024));
        
        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, model.filePath());
        m_modelsList->addItem(item);
    }
    
    if (models.isEmpty()) {
        m_modelsList->addItem("No models found in models/ directory");
    }
}

void MainWindow::appendMessage(const QString &message, const QString &sender) {
    QString html;
    if (sender == "You") {
        html = QString("<div style='margin: 15px 0;'>"
                      "<span style='color: #58a6ff; font-weight: bold; font-size: 14px;'>👤 You:</span><br/>"
                      "<span style='color: #e6edf3; margin-left: 20px;'>%1</span>"
                      "</div>").arg(message.toHtmlEscaped());
    } else if (sender == "AI") {
        html = QString("<div style='margin: 15px 0;'>"
                      "<span style='color: #7ee787; font-weight: bold; font-size: 14px;'>🤖 AI:</span><br/>"
                      "<span style='color: #f0f6fc; margin-left: 20px;'>%1</span>"
                      "</div>").arg(message.toHtmlEscaped());
    } else {
        html = QString("<div style='margin: 15px 0;'>"
                      "<span style='color: #d29922; font-weight: bold; font-size: 14px;'>ℹ️  %1:</span><br/>"
                      "<span style='color: #c9d1d9; margin-left: 20px;'>%2</span>"
                      "</div>").arg(sender.toHtmlEscaped(), message.toHtmlEscaped());
    }
    
    m_chatDisplay->append(html);
    m_chatDisplay->ensureCursorVisible();
}

void MainWindow::sendMessage() {
    if (m_isGenerating) {
        appendMessage("Please wait for the current response to complete or press Stop.", "System");
        return;
    }
    
    if (!m_llamaEngine->isLoaded()) {
        appendMessage("No model loaded! Please load a model from the Models tab first.", "System");
        return;
    }
    
    QString message = m_messageInput->text().trimmed();
    if (message.isEmpty()) {
        return;
    }
    
    m_messageInput->clear();
    appendMessage(message, "You");
    
    m_isGenerating = true;
    m_sendButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    m_messageInput->setEnabled(false);
    m_statusLabel->setText("🤖 Generating response...");
    
    m_currentResponse = "";
    m_tokenCount = 0;
    m_generationStartTime = QTime::currentTime();
    
    m_llamaEngine->generateResponse(message, m_maxTokens);
}

void MainWindow::onTokenReceived(const QString &token) {
    if (m_currentResponse.isEmpty()) {
        m_chatDisplay->append("<div style='margin: 15px 0;'>"
                             "<span style='color: #7ee787; font-weight: bold; font-size: 14px;'>🤖 AI:</span><br/>"
                             "<span style='color: #f0f6fc; margin-left: 20px;'>");
    }
    
    m_currentResponse += token;
    m_tokenCount++;
    
    m_chatDisplay->moveCursor(QTextCursor::End);
    m_chatDisplay->insertPlainText(token);
    m_chatDisplay->ensureCursorVisible();
    
    updateStats();
}

void MainWindow::onResponseComplete() {
    if (!m_currentResponse.isEmpty()) {
        m_chatDisplay->append("</span></div>");
    }
    
    m_isGenerating = false;
    m_sendButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_messageInput->setEnabled(true);
    m_statusLabel->setText("✅ Ready");
    m_currentResponse = "";
    
    updateStats();
}

void MainWindow::onError(const QString &error) {
    appendMessage(error, "Error");
    m_isGenerating = false;
    m_sendButton->setEnabled(true);
    m_stopButton->setEnabled(false);
    m_messageInput->setEnabled(true);
    m_statusLabel->setText("❌ Error occurred");
}

void MainWindow::onStopGeneration() {
    m_llamaEngine->stop();
    m_statusLabel->setText("⏹️  Generation stopped");
    appendMessage("Generation stopped by user.", "System");
}

void MainWindow::onClearChat() {
    m_chatDisplay->clear();
    appendMessage("Chat cleared. Ready for new conversation!", "System");
}

void MainWindow::onSaveChat() {
    QString fileName = QFileDialog::getSaveFileName(this, "Save Chat", 
                                                    QDir::homePath() + "/chat.txt",
                                                    "Text Files (*.txt);;All Files (*)");
    if (fileName.isEmpty()) {
        return;
    }
    
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out << m_chatDisplay->toPlainText();
        file.close();
        appendMessage("Chat saved to: " + fileName, "System");
    } else {
        appendMessage("Failed to save chat!", "Error");
    }
}

void MainWindow::onLoadModel() {
    QListWidgetItem *selected = m_modelsList->currentItem();
    if (!selected || selected->data(Qt::UserRole).toString().isEmpty()) {
        appendMessage("Please select a model to load!", "System");
        return;
    }
    
    QString modelPath = selected->data(Qt::UserRole).toString();
    m_statusLabel->setText("⏳ Loading model...");
    m_currentModelLabel->setText("⏳ Loading: " + QFileInfo(modelPath).fileName());
    
    if (m_llamaEngine->loadModel(modelPath, 2048, 4)) {
        m_currentModelPath = modelPath;
        m_currentModelLabel->setText("✅ Loaded: " + QFileInfo(modelPath).fileName());
        m_statusLabel->setText("✅ Ready - Model loaded with GPU acceleration");
        appendMessage("Model loaded successfully: " + QFileInfo(modelPath).fileName(), "System");
    } else {
        m_currentModelLabel->setText("❌ Failed to load model");
        m_statusLabel->setText("❌ Model load failed");
        appendMessage("Failed to load model!", "Error");
    }
}

void MainWindow::onUnloadModel() {
    // For now, we'll just inform the user
    // (LlamaEngine cleanup happens in destructor)
    m_currentModelPath.clear();
    m_currentModelLabel->setText("No model loaded");
    m_statusLabel->setText("⚠️  No model loaded");
    appendMessage("Model unloaded. Load a new model to continue chatting.", "System");
}

void MainWindow::onTemperatureChanged(int value) {
    m_temperature = value / 100.0f;
    m_temperatureLabel->setText(QString("🌡️ Temperature: %1").arg(m_temperature, 0, 'f', 2));
}

void MainWindow::onMaxTokensChanged(int value) {
    m_maxTokens = value;
}

void MainWindow::updateStats() {
    if (m_isGenerating && m_tokenCount > 0) {
        int elapsed = m_generationStartTime.msecsTo(QTime::currentTime());
        if (elapsed > 0) {
            float tokensPerSec = (m_tokenCount * 1000.0f) / elapsed;
            m_statsLabel->setText(QString("Tokens: %1 | Speed: %2 t/s")
                                .arg(m_tokenCount)
                                .arg(tokensPerSec, 0, 'f', 1));
        }
    } else {
        m_statsLabel->setText(QString("Tokens: %1 | Speed: 0 t/s").arg(m_tokenCount));
    }
}
