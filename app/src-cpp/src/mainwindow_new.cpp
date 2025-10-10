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

void MainWindow::loadConfig()
{
    QFile file("config.json");
    if (file.open(QIODevice::ReadOnly)) {
        QByteArray data = file.readAll();
        m_config = QJsonDocument::fromJson(data).object();
        file.close();
        qDebug() << "✅ Config loaded";
    } else {
        qWarning() << "⚠️ Config not found, using defaults";
    }
}

void MainWindow::saveConfig()
{
    QFile file("config.json");
    if (file.open(QIODevice::WriteOnly)) {
        QJsonDocument doc(m_config);
        file.write(doc.toJson(QJsonDocument::Indented));
        file.close();
        qDebug() << "✅ Config saved";
    }
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    setupSidebar();
    
    // Content stack
    m_contentStack = new QStackedWidget();
    m_mainLayout->addWidget(m_contentStack);
    
    setupChatPage();
    setupMemoryPage();
    setupModelsPage();
    
    // Add pages
    m_contentStack->addWidget(m_chatPage);
    m_contentStack->addWidget(m_memoryPage);
    m_contentStack->addWidget(m_modelsPage);
    
    switchToPage(0); // Default to chat
}

void MainWindow::setupSidebar()
{
    m_sidebar = new QFrame();
    m_sidebar->setObjectName("sidebar");
    m_sidebar->setFixedWidth(250);
    
    m_sidebarLayout = new QVBoxLayout(m_sidebar);
    m_sidebarLayout->setContentsMargins(15, 20, 15, 20);
    m_sidebarLayout->setSpacing(10);
    
    // Branding
    QLabel *branding = new QLabel("🧠 RunMyModel");
    branding->setObjectName("branding");
    QFont brandFont = branding->font();
    brandFont.setPointSize(24);
    brandFont.setBold(true);
    branding->setFont(brandFont);
    m_sidebarLayout->addWidget(branding);
    
    QLabel *subtitle = new QLabel("Self-Learning AI");
    subtitle->setObjectName("subtitle");
    m_sidebarLayout->addWidget(subtitle);
    
    m_sidebarLayout->addSpacing(20);
    
    // Backend status
    m_backendStatus = new QLabel("● Disconnected");
    m_backendStatus->setObjectName("backendStatus");
    m_sidebarLayout->addWidget(m_backendStatus);
    
    m_sidebarLayout->addSpacing(10);
    
    // Navigation
    m_chatBtn = createSidebarButton("💬", "Chat");
    connect(m_chatBtn, &QPushButton::clicked, [this]() { switchToPage(0); });
    m_sidebarLayout->addWidget(m_chatBtn);
    
    m_memoryBtn = createSidebarButton("🧠", "Memory");
    connect(m_memoryBtn, &QPushButton::clicked, [this]() { switchToPage(1); });
    m_sidebarLayout->addWidget(m_memoryBtn);
    
    m_modelsBtn = createSidebarButton("🤖", "Models");
    connect(m_modelsBtn, &QPushButton::clicked, [this]() { switchToPage(2); });
    m_sidebarLayout->addWidget(m_modelsBtn);
    
    m_sidebarLayout->addStretch();
    
    m_aboutBtn = createSidebarButton("ℹ️", "About");
    connect(m_aboutBtn, &QPushButton::clicked, [this]() {
        QMessageBox::about(this, "About RunMyModel",
            "<h2>RunMyModel v0.3.0</h2>"
            "<p><b>Self-Learning LLM Platform</b></p>"
            "<p>Local-first AI with knowledge management and RAG retrieval.</p>"
            "<p><b>Features:</b></p>"
            "<ul>"
            "<li>Run local .gguf models</li>"
            "<li>Ingest knowledge from documents</li>"
            "<li>FAISS vector storage</li>"
            "<li>RAG-based context retrieval</li>"
            "</ul>"
            "<p><b>License:</b> MPL 2.0</p>");
    });
    m_sidebarLayout->addWidget(m_aboutBtn);
    
    m_mainLayout->addWidget(m_sidebar);
}

void MainWindow::setupChatPage()
{
    m_chatPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_chatPage);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(15);
    
    // Header
    QLabel *title = new QLabel("💬 Chat");
    QFont titleFont = title->font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet("color: #111827; font-weight: 800;");
    layout->addWidget(title);
    
    QLabel *desc = new QLabel("Conversation with your self-learning AI");
    desc->setStyleSheet("color: #6b7280; font-size: 14px; margin-bottom: 10px;");
    layout->addWidget(desc);
    
    // Status bar
    QHBoxLayout *statusBar = new QHBoxLayout();
    
    m_modelStatus = new QLabel("No model loaded");
    m_modelStatus->setObjectName("modelStatus");
    statusBar->addWidget(m_modelStatus);
    
    m_contextIndicator = new QLabel("");
    m_contextIndicator->setObjectName("contextIndicator");
    statusBar->addWidget(m_contextIndicator);
    
    m_newChatBtn = new QPushButton("➕ New Chat");
    m_newChatBtn->setObjectName("actionBtn");
    m_newChatBtn->setMinimumHeight(40);
    connect(m_newChatBtn, &QPushButton::clicked, this, &MainWindow::onNewChat);
    statusBar->addWidget(m_newChatBtn);
    
    statusBar->addStretch();
    layout->addLayout(statusBar);
    
    // Chat display
    m_chatDisplay = new QTextEdit();
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setObjectName("chatDisplay");
    layout->addWidget(m_chatDisplay);
    
    // Input
    QHBoxLayout *inputLayout = new QHBoxLayout();
    
    m_chatInput = new QLineEdit();
    m_chatInput->setPlaceholderText("Ask your AI anything...");
    m_chatInput->setMinimumHeight(55);
    m_chatInput->setObjectName("chatInput");
    connect(m_chatInput, &QLineEdit::returnPressed, this, &MainWindow::onSendMessage);
    inputLayout->addWidget(m_chatInput);
    
    m_sendBtn = new QPushButton("Send ➤");
    m_sendBtn->setObjectName("sendBtn");
    m_sendBtn->setMinimumHeight(55);
    m_sendBtn->setMinimumWidth(120);
    connect(m_sendBtn, &QPushButton::clicked, this, &MainWindow::onSendMessage);
    inputLayout->addWidget(m_sendBtn);
    
    layout->addLayout(inputLayout);
}

void MainWindow::setupMemoryPage()
{
    m_memoryPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_memoryPage);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(15);
    
    // Header
    QLabel *title = new QLabel("🧠 Knowledge Memory");
    QFont titleFont = title->font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet("color: #111827; font-weight: 800;");
    layout->addWidget(title);
    
    QLabel *desc = new QLabel("Manage your AI's knowledge base");
    desc->setStyleSheet("color: #6b7280; font-size: 14px; margin-bottom: 10px;");
    layout->addWidget(desc);
    
    // Stats
    m_knowledgeStats = new QLabel("0 knowledge sources | 0 chunks");
    m_knowledgeStats->setObjectName("statsLabel");
    layout->addWidget(m_knowledgeStats);
    
    // Actions
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    
    m_knowledgeSearch = new QLineEdit();
    m_knowledgeSearch->setPlaceholderText("Search knowledge...");
    m_knowledgeSearch->setMinimumHeight(45);
    connect(m_knowledgeSearch, &QLineEdit::textChanged, this, &MainWindow::onSearchKnowledge);
    actionsLayout->addWidget(m_knowledgeSearch);
    
    m_addKnowledgeBtn = new QPushButton("📁 Add Knowledge");
    m_addKnowledgeBtn->setObjectName("actionBtn");
    m_addKnowledgeBtn->setMinimumHeight(45);
    connect(m_addKnowledgeBtn, &QPushButton::clicked, this, &MainWindow::onAddKnowledge);
    actionsLayout->addWidget(m_addKnowledgeBtn);
    
    m_refreshKnowledgeBtn = new QPushButton("🔄 Refresh");
    m_refreshKnowledgeBtn->setObjectName("actionBtn");
    m_refreshKnowledgeBtn->setMinimumHeight(45);
    connect(m_refreshKnowledgeBtn, &QPushButton::clicked, this, &MainWindow::onRefreshKnowledge);
    actionsLayout->addWidget(m_refreshKnowledgeBtn);
    
    m_deleteKnowledgeBtn = new QPushButton("🗑️ Delete");
    m_deleteKnowledgeBtn->setObjectName("deleteBtn");
    m_deleteKnowledgeBtn->setMinimumHeight(45);
    m_deleteKnowledgeBtn->setEnabled(false);
    connect(m_deleteKnowledgeBtn, &QPushButton::clicked, this, &MainWindow::onDeleteKnowledge);
    actionsLayout->addWidget(m_deleteKnowledgeBtn);
    
    layout->addLayout(actionsLayout);
    
    // Split view: list + preview
    QHBoxLayout *contentLayout = new QHBoxLayout();
    
    m_knowledgeList = new QListWidget();
    m_knowledgeList->setObjectName("knowledgeList");
    connect(m_knowledgeList, &QListWidget::itemClicked, this, &MainWindow::onKnowledgeSelected);
    contentLayout->addWidget(m_knowledgeList, 1);
    
    m_knowledgePreview = new QTextEdit();
    m_knowledgePreview->setReadOnly(true);
    m_knowledgePreview->setObjectName("knowledgePreview");
    m_knowledgePreview->setPlaceholderText("Select a knowledge source to preview...");
    contentLayout->addWidget(m_knowledgePreview, 2);
    
    layout->addLayout(contentLayout);
}

void MainWindow::setupModelsPage()
{
    m_modelsPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_modelsPage);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(15);
    
    // Header
    QLabel *title = new QLabel("🤖 Models");
    QFont titleFont = title->font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setStyleSheet("color: #111827; font-weight: 800;");
    layout->addWidget(title);
    
    QLabel *desc = new QLabel("Load and manage your AI models");
    desc->setStyleSheet("color: #6b7280; font-size: 14px; margin-bottom: 10px;");
    layout->addWidget(desc);
    
    // Model selection
    QGroupBox *modelGroup = new QGroupBox("Local Model");
    QVBoxLayout *modelGroupLayout = new QVBoxLayout(modelGroup);
    
    QHBoxLayout *modelSelectLayout = new QHBoxLayout();
    
    m_modelSelector = new QComboBox();
    m_modelSelector->setMinimumHeight(45);
    m_modelSelector->setPlaceholderText("Select a .gguf model...");
    modelSelectLayout->addWidget(m_modelSelector, 1);
    
    m_browseModelBtn = new QPushButton("📁 Browse...");
    m_browseModelBtn->setObjectName("actionBtn");
    m_browseModelBtn->setMinimumHeight(45);
    connect(m_browseModelBtn, &QPushButton::clicked, this, &MainWindow::onBrowseModel);
    modelSelectLayout->addWidget(m_browseModelBtn);
    
    modelGroupLayout->addLayout(modelSelectLayout);
    
    QHBoxLayout *modelActionsLayout = new QHBoxLayout();
    
    m_loadModelBtn = new QPushButton("✅ Load Model");
    m_loadModelBtn->setObjectName("actionBtn");
    m_loadModelBtn->setMinimumHeight(45);
    connect(m_loadModelBtn, &QPushButton::clicked, this, &MainWindow::onLoadModel);
    modelActionsLayout->addWidget(m_loadModelBtn);
    
    m_unloadModelBtn = new QPushButton("❌ Unload Model");
    m_unloadModelBtn->setObjectName("deleteBtn");
    m_unloadModelBtn->setMinimumHeight(45);
    m_unloadModelBtn->setEnabled(false);
    connect(m_unloadModelBtn, &QPushButton::clicked, this, &MainWindow::onUnloadModel);
    modelActionsLayout->addWidget(m_unloadModelBtn);
    
    modelGroupLayout->addLayout(modelActionsLayout);
    
    m_modelInfo = new QLabel("No model loaded");
    m_modelInfo->setObjectName("modelInfo");
    m_modelInfo->setWordWrap(true);
    modelGroupLayout->addWidget(m_modelInfo);
    
    layout->addWidget(modelGroup);
    
    // API configuration
    QGroupBox *apiGroup = new QGroupBox("API Mode (Optional)");
    QFormLayout *apiLayout = new QFormLayout(apiGroup);
    
    m_apiProviderSelector = new QComboBox();
    m_apiProviderSelector->addItems({"Hugging Face", "OpenAI", "LM Studio"});
    m_apiProviderSelector->setMinimumHeight(40);
    apiLayout->addRow("Provider:", m_apiProviderSelector);
    
    m_apiKeyInput = new QLineEdit();
    m_apiKeyInput->setPlaceholderText("Enter API key...");
    m_apiKeyInput->setEchoMode(QLineEdit::Password);
    m_apiKeyInput->setMinimumHeight(40);
    apiLayout->addRow("API Key:", m_apiKeyInput);
    
    QPushButton *saveApiBtn = new QPushButton("💾 Save API Key");
    saveApiBtn->setObjectName("actionBtn");
    saveApiBtn->setMinimumHeight(40);
    connect(saveApiBtn, &QPushButton::clicked, [this]() {
        m_config["api"] = QJsonObject{
            {"enabled", true},
            {"provider", m_apiProviderSelector->currentText()},
            {"key", m_apiKeyInput->text()}
        };
        saveConfig();
        QMessageBox::information(this, "Success", "API key saved!");
    });
    apiLayout->addRow("", saveApiBtn);
    
    layout->addWidget(apiGroup);
    
    layout->addStretch();
}

void MainWindow::applyModernStyling()
{
    QString styleSheet = R"(
        QMainWindow {
            background-color: #ffffff;
        }
        
        #sidebar {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #0f172a, stop:1 #1e293b);
            border-right: 1px solid #334155;
        }
        
        #branding {
            color: white;
            padding: 10px 0;
        }
        
        #subtitle {
            color: rgba(255, 255, 255, 0.6);
            font-size: 12px;
        }
        
        #backendStatus {
            font-size: 12px;
            font-weight: 600;
            padding: 8px 12px;
            background: rgba(255, 255, 255, 0.1);
            border-radius: 8px;
        }
        
        QPushButton[objectName="sidebarBtn"] {
            background-color: transparent;
            color: rgba(255, 255, 255, 0.8);
            border: none;
            border-radius: 8px;
            padding: 12px 14px;
            text-align: left;
            font-size: 14px;
        }
        
        QPushButton[objectName="sidebarBtn"]:hover {
            background-color: rgba(59, 130, 246, 0.15);
            color: white;
        }
        
        QPushButton[objectName="sidebarBtn"]:checked {
            background: rgba(59, 130, 246, 0.25);
            color: white;
            font-weight: bold;
        }
        
        #chatDisplay, #knowledgePreview {
            background: #f9fafb;
            border: 2px solid #e5e7eb;
            border-radius: 12px;
            padding: 20px;
            font-size: 14px;
            line-height: 1.6;
        }
        
        #chatInput {
            background: white;
            border: 2px solid #e5e7eb;
            border-radius: 12px;
            padding: 14px 20px;
            font-size: 15px;
        }
        
        #chatInput:focus {
            border-color: #3b82f6;
        }
        
        #sendBtn {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #2563eb);
            color: white;
            border: none;
            border-radius: 12px;
            font-weight: 700;
            font-size: 15px;
        }
        
        #sendBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2563eb, stop:1 #1d4ed8);
        }
        
        QPushButton[objectName="actionBtn"] {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #2563eb);
            color: white;
            border: none;
            border-radius: 10px;
            font-weight: 600;
            padding: 10px 20px;
        }
        
        QPushButton[objectName="actionBtn"]:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #2563eb, stop:1 #1d4ed8);
        }
        
        QPushButton[objectName="deleteBtn"] {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #ef4444, stop:1 #dc2626);
            color: white;
            border: none;
            border-radius: 10px;
            font-weight: 600;
            padding: 10px 20px;
        }
        
        QPushButton[objectName="deleteBtn"]:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #dc2626, stop:1 #b91c1c);
        }
        
        #knowledgeList {
            background: white;
            border: 2px solid #e5e7eb;
            border-radius: 12px;
            padding: 10px;
        }
        
        #knowledgeList::item {
            padding: 12px;
            border-radius: 8px;
            margin: 4px 0;
        }
        
        #knowledgeList::item:selected {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #eff6ff, stop:1 #dbeafe);
            color: #1e40af;
        }
        
        QComboBox {
            background: white;
            border: 2px solid #e5e7eb;
            border-radius: 10px;
            padding: 10px 14px;
            font-size: 14px;
        }
        
        QComboBox:focus {
            border-color: #3b82f6;
        }
        
        QLineEdit {
            background: white;
            border: 2px solid #e5e7eb;
            border-radius: 10px;
            padding: 10px 14px;
            font-size: 14px;
        }
        
        QLineEdit:focus {
            border-color: #3b82f6;
        }
        
        QGroupBox {
            background: white;
            border: 2px solid #e5e7eb;
            border-radius: 12px;
            padding: 20px;
            margin-top: 12px;
            font-weight: 600;
        }
        
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 12px;
            padding: 0 8px;
        }
        
        #modelStatus, #contextIndicator, #statsLabel {
            font-size: 13px;
            color: #6b7280;
            font-weight: 600;
            padding: 6px 12px;
            background: #f3f4f6;
            border-radius: 8px;
        }
        
        #modelInfo {
            font-size: 13px;
            color: #374151;
            padding: 12px;
            background: #f9fafb;
            border-radius: 8px;
            margin-top: 10px;
        }
    )";
    
    setStyleSheet(styleSheet);
}

QPushButton* MainWindow::createSidebarButton(const QString &icon, const QString &text)
{
    QPushButton *btn = new QPushButton(icon + "  " + text);
    btn->setObjectName("sidebarBtn");
    btn->setCheckable(true);
    btn->setMinimumHeight(48);
    btn->setCursor(Qt::PointingHandCursor);
    return btn;
}

void MainWindow::switchToPage(int index)
{
    m_contentStack->setCurrentIndex(index);
    
    m_chatBtn->setChecked(index == 0);
    m_memoryBtn->setChecked(index == 1);
    m_modelsBtn->setChecked(index == 2);
    
    // Refresh data when switching pages
    if (index == 1) {
        onRefreshKnowledge();
    } else if (index == 2) {
        onRefreshModels();
    }
}

void MainWindow::updateBackendStatus(const QString &status, const QString &color)
{
    m_backendStatus->setText("● " + status);
    m_backendStatus->setStyleSheet(QString("color: %1; font-weight: 600;").arg(color));
}

void MainWindow::appendChatMessage(const QString &role, const QString &message)
{
    QString html = m_chatDisplay->toHtml();
    
    QString roleColor = (role == "You") ? "#60a5fa" : "#10b981";
    QString icon = (role == "You") ? "👤" : "🤖";
    QString bgColor = (role == "You") ? "#eff6ff" : "#f0fdf4";
    
    QString msgHtml = QString(
        "<div style='margin-bottom: 20px; padding: 18px; background-color: %1; "
        "border-radius: 12px; box-shadow: 0 2px 8px rgba(0,0,0,0.05);'>"
        "<b style='color: %2; font-size: 15px;'>%3 %4</b><br><br>"
        "<span style='color: #1f2937;'>%5</span>"
        "</div>")
        .arg(bgColor, roleColor, icon, role, message.toHtmlEscaped());
    
    m_chatDisplay->setHtml(html + msgHtml);
    
    QScrollBar *scrollBar = m_chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

// ============================================================================
// Chat Slots
// ============================================================================

void MainWindow::onSendMessage()
{
    QString message = m_chatInput->text().trimmed();
    if (message.isEmpty() || !m_backendClient) return;
    
    appendChatMessage("You", message);
    m_chatInput->clear();
    
    m_currentStreamingResponse.clear();
    appendChatMessage("Assistant", "");
    
    m_sendBtn->setEnabled(false);
    m_modelStatus->setText("🧠 Generating...");
    m_modelStatus->setStyleSheet("color: #f59e0b; font-weight: 600;");
    
    // Send to backend (will be implemented in backend_client.cpp)
    m_backendClient->chatCompletion(message);
}

void MainWindow::onNewChat()
{
    m_chatDisplay->clear();
    appendChatMessage("System", "New chat started. Ask me anything!");
}

void MainWindow::onStreamToken(const QString &token)
{
    m_currentStreamingResponse += token;
    
    // Update last assistant message
    QString html = m_chatDisplay->toHtml();
    int lastAssistantIndex = html.lastIndexOf("<div style='margin-bottom: 20px; padding: 18px; background-color: #f0fdf4");
    
    if (lastAssistantIndex >= 0) {
        QString beforeLastMessage = html.left(lastAssistantIndex);
        
        QString msgHtml = QString(
            "<div style='margin-bottom: 20px; padding: 18px; background-color: #f0fdf4; "
            "border-radius: 12px; box-shadow: 0 2px 8px rgba(0,0,0,0.05);'>"
            "<b style='color: #10b981; font-size: 15px;'>🤖 Assistant</b><br><br>"
            "<span style='color: #1f2937;'>%1</span>"
            "</div>")
            .arg(m_currentStreamingResponse.toHtmlEscaped());
        
        m_chatDisplay->setHtml(beforeLastMessage + msgHtml);
        
        QScrollBar *scrollBar = m_chatDisplay->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }
}

void MainWindow::onStreamComplete()
{
    m_sendBtn->setEnabled(true);
    m_modelStatus->setText("✅ Ready");
    m_modelStatus->setStyleSheet("color: #10b981; font-weight: 600;");
    m_currentStreamingResponse.clear();
}

void MainWindow::onStreamError(const QString &error)
{
    m_sendBtn->setEnabled(true);
    m_modelStatus->setText("❌ Error");
    m_modelStatus->setStyleSheet("color: #ef4444; font-weight: 600;");
    
    appendChatMessage("Error", error);
}

// ============================================================================
// Memory Slots
// ============================================================================

void MainWindow::onAddKnowledge()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Knowledge File",
        QDir::homePath(),
        "Text Files (*.txt *.md);;All Files (*)"
    );
    
    if (filePath.isEmpty()) return;
    
    if (!m_backendClient) {
        QMessageBox::warning(this, "Error", "Backend not connected");
        return;
    }
    
    // Read file
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QMessageBox::warning(this, "Error", "Could not read file");
        return;
    }
    
    QString content = file.readAll();
    file.close();
    
    // Send to backend for ingestion
    m_addKnowledgeBtn->setEnabled(false);
    m_addKnowledgeBtn->setText("Processing...");
    
    // Backend call will be implemented
    m_backendClient->ingestKnowledge(filePath, content);
    
    QTimer::singleShot(2000, this, [this]() {
        m_addKnowledgeBtn->setEnabled(true);
        m_addKnowledgeBtn->setText("📁 Add Knowledge");
        onRefreshKnowledge();
    });
}

void MainWindow::onDeleteKnowledge()
{
    QListWidgetItem *item = m_knowledgeList->currentItem();
    if (!item || !m_backendClient) return;
    
    int sourceId = item->data(Qt::UserRole).toInt();
    
    if (QMessageBox::question(this, "Confirm Delete",
        "Delete this knowledge source?") == QMessageBox::Yes) {
        
        m_backendClient->deleteKnowledge(sourceId);
        onRefreshKnowledge();
    }
}

void MainWindow::onRefreshKnowledge()
{
    if (!m_backendClient) return;
    
    m_knowledgeList->clear();
    
    // Backend call will populate the list
    m_backendClient->listKnowledge();
    
    // Placeholder for now
    m_knowledgeStats->setText("Loading...");
}

void MainWindow::onSearchKnowledge()
{
    QString query = m_knowledgeSearch->text();
    
    if (query.isEmpty()) {
        onRefreshKnowledge();
        return;
    }
    
    if (!m_backendClient) return;
    
    // Backend call
    m_backendClient->searchKnowledge(query);
}

void MainWindow::onKnowledgeSelected(QListWidgetItem *item)
{
    if (!item) return;
    
    m_deleteKnowledgeBtn->setEnabled(true);
    
    QString preview = item->data(Qt::UserRole + 1).toString();
    m_knowledgePreview->setText(preview);
}

// ============================================================================
// Model Slots
// ============================================================================

void MainWindow::onLoadModel()
{
    QString modelPath = m_modelSelector->currentText();
    if (modelPath.isEmpty() || !m_backendClient) return;
    
    m_loadModelBtn->setEnabled(false);
    m_loadModelBtn->setText("Loading...");
    m_modelInfo->setText("Loading model, please wait...");
    
    // Backend call
    m_backendClient->loadModel(modelPath);
}

void MainWindow::onUnloadModel()
{
    if (!m_backendClient) return;
    
    m_backendClient->unloadModel();
    
    m_loadModelBtn->setEnabled(true);
    m_unloadModelBtn->setEnabled(false);
    m_modelInfo->setText("No model loaded");
    m_modelStatus->setText("No model loaded");
}

void MainWindow::onBrowseModel()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Model File",
        QDir::homePath(),
        "GGUF Models (*.gguf);;All Files (*)"
    );
    
    if (!filePath.isEmpty()) {
        m_modelSelector->addItem(filePath);
        m_modelSelector->setCurrentText(filePath);
    }
}

void MainWindow::onModelLoaded(const QString &modelName)
{
    m_loadModelBtn->setEnabled(true);
    m_loadModelBtn->setText("✅ Load Model");
    m_unloadModelBtn->setEnabled(true);
    
    m_currentModel = modelName;
    
    QString info = QString(
        "<b>Model:</b> %1<br>"
        "<b>Status:</b> ✅ Loaded<br>"
        "<b>Context:</b> 4096 tokens"
    ).arg(modelName);
    
    m_modelInfo->setText(info);
    m_modelStatus->setText("✅ " + modelName);
    m_modelStatus->setStyleSheet("color: #10b981; font-weight: 600;");
}

void MainWindow::onRefreshModels()
{
    m_modelSelector->clear();
    
    // Scan models directory
    QString modelsDir = m_config["storage"].toObject()["models_dir"].toString();
    QDir dir(modelsDir);
    
    if (dir.exists()) {
        QStringList models = dir.entryList(QStringList() << "*.gguf", QDir::Files);
        for (const QString &model : models) {
            m_modelSelector->addItem(dir.absoluteFilePath(model));
        }
    }
}

void MainWindow::onBackendConnected()
{
    updateBackendStatus("Connected", "#10b981");
}

void MainWindow::onBackendDisconnected()
{
    updateBackendStatus("Disconnected", "#ef4444");
}

