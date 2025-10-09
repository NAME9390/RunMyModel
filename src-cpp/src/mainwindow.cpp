#include "mainwindow.h"
#include <QApplication>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollBar>
#include <QFileDialog>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_backend(new Backend(this))
{
    setWindowTitle("RunMyModel Desktop - v0.2.0 ALPHA");
    resize(1400, 900);
    
    setupUI();
    applyModernStyling();
    loadAvailableModels();
    loadInstalledModels();
    updateModelSelector();
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    m_centralWidget = new QWidget(this);
    setCentralWidget(m_centralWidget);
    
    m_mainLayout = new QHBoxLayout(m_centralWidget);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setSpacing(0);
    
    setupSidebar();
    
    // Create stacked widget for different pages
    m_contentStack = new QStackedWidget();
    m_mainLayout->addWidget(m_contentStack);
    
    setupChatPage();
    setupDownloadPage();
    setupInstalledPage();
    setupCustomPage();
    
    // Add pages to stack
    m_contentStack->addWidget(m_chatPage);
    m_contentStack->addWidget(m_downloadPage);
    m_contentStack->addWidget(m_installedPage);
    m_contentStack->addWidget(m_customPage);
    
    // Show chat page by default
    switchToPage(0);
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
    m_brandingLabel = new QLabel("🤖 RunMyModel");
    m_brandingLabel->setObjectName("branding");
    QFont brandFont = m_brandingLabel->font();
    brandFont.setPointSize(18);
    brandFont.setBold(true);
    m_brandingLabel->setFont(brandFont);
    m_sidebarLayout->addWidget(m_brandingLabel);
    
    QLabel *versionLabel = new QLabel("v0.2.0 ALPHA");
    versionLabel->setObjectName("version");
    m_sidebarLayout->addWidget(versionLabel);
    
    m_sidebarLayout->addSpacing(20);
    
    // New Chat button (prominent)
    m_newChatBtn = new QPushButton("➕ New Chat");
    m_newChatBtn->setObjectName("newChatBtn");
    m_newChatBtn->setMinimumHeight(45);
    connect(m_newChatBtn, &QPushButton::clicked, this, &MainWindow::onNewChatClicked);
    m_sidebarLayout->addWidget(m_newChatBtn);
    
    m_sidebarLayout->addSpacing(20);
    
    // Navigation buttons
    m_chatPageBtn = createSidebarButton("💬", "Chat");
    connect(m_chatPageBtn, &QPushButton::clicked, [this]() { switchToPage(0); });
    m_sidebarLayout->addWidget(m_chatPageBtn);
    
    m_downloadPageBtn = createSidebarButton("🌐", "Browse Models");
    connect(m_downloadPageBtn, &QPushButton::clicked, [this]() { switchToPage(1); });
    m_sidebarLayout->addWidget(m_downloadPageBtn);
    
    m_installedPageBtn = createSidebarButton("📦", "My Models");
    connect(m_installedPageBtn, &QPushButton::clicked, [this]() { switchToPage(2); });
    m_sidebarLayout->addWidget(m_installedPageBtn);
    
    m_customPageBtn = createSidebarButton("⚙️", "Custom Models");
    connect(m_customPageBtn, &QPushButton::clicked, [this]() { switchToPage(3); });
    m_sidebarLayout->addWidget(m_customPageBtn);
    
    m_sidebarLayout->addStretch();
    
    // About button at bottom
    m_aboutBtn = createSidebarButton("ℹ️", "About");
    connect(m_aboutBtn, &QPushButton::clicked, this, &MainWindow::onAboutClicked);
    m_sidebarLayout->addWidget(m_aboutBtn);
    
    m_mainLayout->addWidget(m_sidebar);
}

QPushButton* MainWindow::createSidebarButton(const QString &icon, const QString &text)
{
    QPushButton *btn = new QPushButton(QString("%1  %2").arg(icon, text));
    btn->setObjectName("sidebarBtn");
    btn->setMinimumHeight(40);
    btn->setCheckable(true);
    return btn;
}

void MainWindow::setupChatPage()
{
    m_chatPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_chatPage);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(15);
    
    // Title
    QLabel *titleLabel = new QLabel("💬 Chat with AI Models");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);
    
    // Model selector bar
    QHBoxLayout *modelBar = new QHBoxLayout();
    modelBar->addWidget(new QLabel("Model:"));
    
    m_modelSelector = new QComboBox();
    m_modelSelector->setMinimumWidth(400);
    m_modelSelector->setMinimumHeight(35);
    modelBar->addWidget(m_modelSelector);
    
    m_modelSelectorBtn = new QPushButton("🔄 Refresh Models");
    connect(m_modelSelectorBtn, &QPushButton::clicked, this, &MainWindow::onRefreshModels);
    modelBar->addWidget(m_modelSelectorBtn);
    
    m_statusLabel = new QLabel("Ready");
    m_statusLabel->setObjectName("statusLabel");
    modelBar->addWidget(m_statusLabel);
    modelBar->addStretch();
    
    layout->addLayout(modelBar);
    
    // Chat display
    m_chatDisplay = new QTextEdit();
    m_chatDisplay->setReadOnly(true);
    m_chatDisplay->setObjectName("chatDisplay");
    layout->addWidget(m_chatDisplay);
    
    // Input area
    QHBoxLayout *inputLayout = new QHBoxLayout();
    
    m_chatInput = new QLineEdit();
    m_chatInput->setPlaceholderText("Type your message here...");
    m_chatInput->setMinimumHeight(50);
    m_chatInput->setObjectName("chatInput");
    inputLayout->addWidget(m_chatInput);
    
    m_sendButton = new QPushButton("Send ➤");
    m_sendButton->setObjectName("sendBtn");
    m_sendButton->setMinimumHeight(50);
    m_sendButton->setMinimumWidth(120);
    inputLayout->addWidget(m_sendButton);
    
    layout->addLayout(inputLayout);
    
    // Connect signals
    connect(m_sendButton, &QPushButton::clicked, this, &MainWindow::onSendMessage);
    connect(m_chatInput, &QLineEdit::returnPressed, this, &MainWindow::onSendMessage);
    connect(m_modelSelector, &QComboBox::currentTextChanged, this, &MainWindow::onModelSelected);
}

void MainWindow::setupDownloadPage()
{
    m_downloadPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_downloadPage);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(15);
    
    // Title
    QLabel *titleLabel = new QLabel("🌐 Browse & Download Models");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);
    
    // Search and filters
    QHBoxLayout *filterBar = new QHBoxLayout();
    
    m_downloadSearch = new QLineEdit();
    m_downloadSearch->setPlaceholderText("🔍 Search models...");
    m_downloadSearch->setMinimumHeight(35);
    filterBar->addWidget(m_downloadSearch);
    
    m_taskTypeFilter = new QComboBox();
    m_taskTypeFilter->addItem("All Tasks");
    m_taskTypeFilter->setMinimumHeight(35);
    filterBar->addWidget(m_taskTypeFilter);
    
    m_sortByCombo = new QComboBox();
    m_sortByCombo->addItems({"Sort by Rating", "Sort by Name", "Sort by Size"});
    m_sortByCombo->setMinimumHeight(35);
    filterBar->addWidget(m_sortByCombo);
    
    m_downloadCountLabel = new QLabel("0 models");
    m_downloadCountLabel->setObjectName("countLabel");
    filterBar->addWidget(m_downloadCountLabel);
    
    layout->addLayout(filterBar);
    
    // Splitter for list and details
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    
    // Models list
    QWidget *listWidget = new QWidget();
    QVBoxLayout *listLayout = new QVBoxLayout(listWidget);
    listLayout->setContentsMargins(0, 0, 0, 0);
    
    m_availableModelsList = new QListWidget();
    m_availableModelsList->setObjectName("modelsList");
    listLayout->addWidget(m_availableModelsList);
    
    splitter->addWidget(listWidget);
    
    // Model details
    QWidget *detailsWidget = new QWidget();
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsWidget);
    detailsLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *detailsTitle = new QLabel("Model Details");
    QFont detailsFont = detailsTitle->font();
    detailsFont.setPointSize(14);
    detailsFont.setBold(true);
    detailsTitle->setFont(detailsFont);
    detailsLayout->addWidget(detailsTitle);
    
    m_downloadModelDetails = new QTextEdit();
    m_downloadModelDetails->setReadOnly(true);
    m_downloadModelDetails->setObjectName("detailsBox");
    detailsLayout->addWidget(m_downloadModelDetails);
    
    // Action buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    m_downloadModelBtn = new QPushButton("⬇️ Download Model");
    m_downloadModelBtn->setObjectName("downloadBtn");
    m_downloadModelBtn->setMinimumHeight(40);
    buttonLayout->addWidget(m_downloadModelBtn);
    buttonLayout->addStretch();
    detailsLayout->addLayout(buttonLayout);
    
    // Progress bar
    m_downloadProgress = new QProgressBar();
    m_downloadProgress->setVisible(false);
    m_downloadProgress->setMinimumHeight(25);
    detailsLayout->addWidget(m_downloadProgress);
    
    splitter->addWidget(detailsWidget);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);
    
    layout->addWidget(splitter);
    
    // Connect signals
    connect(m_downloadSearch, &QLineEdit::textChanged, this, &MainWindow::onModelSearchChanged);
    connect(m_availableModelsList, &QListWidget::itemClicked, this, &MainWindow::showModelDetails);
    connect(m_downloadModelBtn, &QPushButton::clicked, this, &MainWindow::onDownloadModel);
}

void MainWindow::setupInstalledPage()
{
    m_installedPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_installedPage);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(15);
    
    // Title
    QLabel *titleLabel = new QLabel("📦 My Installed Models");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);
    
    // Search bar
    QHBoxLayout *searchBar = new QHBoxLayout();
    
    m_installedSearch = new QLineEdit();
    m_installedSearch->setPlaceholderText("🔍 Search installed models...");
    m_installedSearch->setMinimumHeight(35);
    searchBar->addWidget(m_installedSearch);
    
    m_installedCountLabel = new QLabel("0 models installed");
    m_installedCountLabel->setObjectName("countLabel");
    searchBar->addWidget(m_installedCountLabel);
    
    layout->addLayout(searchBar);
    
    // Splitter for list and details
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    
    // Installed models list
    m_installedModelsList = new QListWidget();
    m_installedModelsList->setObjectName("modelsList");
    splitter->addWidget(m_installedModelsList);
    
    // Model details and actions
    QWidget *detailsWidget = new QWidget();
    QVBoxLayout *detailsLayout = new QVBoxLayout(detailsWidget);
    detailsLayout->setContentsMargins(0, 0, 0, 0);
    
    QLabel *detailsTitle = new QLabel("Model Information");
    QFont detailsFont = detailsTitle->font();
    detailsFont.setPointSize(14);
    detailsFont.setBold(true);
    detailsTitle->setFont(detailsFont);
    detailsLayout->addWidget(detailsTitle);
    
    m_installedModelDetails = new QTextEdit();
    m_installedModelDetails->setReadOnly(true);
    m_installedModelDetails->setObjectName("detailsBox");
    detailsLayout->addWidget(m_installedModelDetails);
    
    // Action buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_useModelBtn = new QPushButton("✓ Use This Model");
    m_useModelBtn->setObjectName("useBtn");
    m_useModelBtn->setMinimumHeight(40);
    buttonLayout->addWidget(m_useModelBtn);
    
    m_removeModelBtn = new QPushButton("🗑️ Remove Model");
    m_removeModelBtn->setObjectName("removeBtn");
    m_removeModelBtn->setMinimumHeight(40);
    buttonLayout->addWidget(m_removeModelBtn);
    
    buttonLayout->addStretch();
    detailsLayout->addLayout(buttonLayout);
    
    splitter->addWidget(detailsWidget);
    splitter->setStretchFactor(0, 2);
    splitter->setStretchFactor(1, 1);
    
    layout->addWidget(splitter);
    
    // Connect signals
    connect(m_installedModelsList, &QListWidget::itemClicked, this, &MainWindow::showModelDetails);
    connect(m_removeModelBtn, &QPushButton::clicked, this, &MainWindow::onRemoveModel);
    connect(m_useModelBtn, &QPushButton::clicked, [this]() {
        QListWidgetItem *item = m_installedModelsList->currentItem();
        if (item) {
            QString modelName = item->data(Qt::UserRole).toJsonObject()["name"].toString();
            m_modelSelector->setCurrentText(modelName);
            switchToPage(0); // Switch to chat
        }
    });
}

void MainWindow::setupCustomPage()
{
    m_customPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_customPage);
    layout->setContentsMargins(30, 30, 30, 30);
    layout->setSpacing(15);
    
    // Title
    QLabel *titleLabel = new QLabel("⚙️ Add Custom Models");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    layout->addWidget(titleLabel);
    
    QLabel *descLabel = new QLabel("Load custom GGUF models from your filesystem");
    layout->addWidget(descLabel);
    
    layout->addSpacing(20);
    
    // Model path
    QHBoxLayout *pathLayout = new QHBoxLayout();
    pathLayout->addWidget(new QLabel("Model File:"));
    
    m_customModelPath = new QLineEdit();
    m_customModelPath->setPlaceholderText("Select a .gguf file...");
    m_customModelPath->setMinimumHeight(35);
    pathLayout->addWidget(m_customModelPath);
    
    m_browseCustomBtn = new QPushButton("📁 Browse...");
    m_browseCustomBtn->setMinimumHeight(35);
    connect(m_browseCustomBtn, &QPushButton::clicked, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Select Model File", 
                                                        QString(), "GGUF Files (*.gguf);;All Files (*)");
        if (!fileName.isEmpty()) {
            m_customModelPath->setText(fileName);
        }
    });
    pathLayout->addWidget(m_browseCustomBtn);
    
    layout->addLayout(pathLayout);
    
    // Model name
    QHBoxLayout *nameLayout = new QHBoxLayout();
    nameLayout->addWidget(new QLabel("Model Name:"));
    
    m_customModelName = new QLineEdit();
    m_customModelName->setPlaceholderText("e.g., my-custom-model-7b");
    m_customModelName->setMinimumHeight(35);
    nameLayout->addWidget(m_customModelName);
    
    layout->addLayout(nameLayout);
    
    // Model format
    QHBoxLayout *formatLayout = new QHBoxLayout();
    formatLayout->addWidget(new QLabel("Format:"));
    
    m_customModelFormat = new QComboBox();
    m_customModelFormat->addItems({"GGUF", "GGML", "Other"});
    m_customModelFormat->setMinimumHeight(35);
    formatLayout->addWidget(m_customModelFormat);
    formatLayout->addStretch();
    
    layout->addLayout(formatLayout);
    
    // Configuration (optional)
    layout->addWidget(new QLabel("Configuration (JSON, optional):"));
    m_customModelConfig = new QTextEdit();
    m_customModelConfig->setPlaceholderText("{\n  \"context_length\": 2048,\n  \"temperature\": 0.7\n}");
    m_customModelConfig->setMaximumHeight(150);
    layout->addWidget(m_customModelConfig);
    
    // Add button
    m_addCustomModelBtn = new QPushButton("➕ Add Custom Model");
    m_addCustomModelBtn->setObjectName("addBtn");
    m_addCustomModelBtn->setMinimumHeight(45);
    layout->addWidget(m_addCustomModelBtn);
    
    layout->addSpacing(20);
    
    // List of custom models
    layout->addWidget(new QLabel("Your Custom Models:"));
    m_customModelsList = new QListWidget();
    m_customModelsList->setObjectName("modelsList");
    layout->addWidget(m_customModelsList);
}

void MainWindow::applyModernStyling()
{
    // Modern dark/light theme
    QString styleSheet = R"(
        QMainWindow {
            background-color: #f5f5f5;
        }
        
        #sidebar {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                stop:0 #667eea, stop:1 #764ba2);
            border-right: 1px solid #5a67d8;
        }
        
        #branding {
            color: white;
            font-size: 20px;
            font-weight: bold;
        }
        
        #version {
            color: rgba(255, 255, 255, 0.7);
            font-size: 11px;
        }
        
        #newChatBtn {
            background-color: rgba(255, 255, 255, 0.95);
            color: #667eea;
            border: none;
            border-radius: 8px;
            padding: 12px;
            font-weight: bold;
            font-size: 14px;
        }
        
        #newChatBtn:hover {
            background-color: white;
        }
        
        #sidebarBtn {
            background-color: rgba(255, 255, 255, 0.1);
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px;
            text-align: left;
            font-size: 13px;
        }
        
        #sidebarBtn:hover {
            background-color: rgba(255, 255, 255, 0.2);
        }
        
        #sidebarBtn:checked {
            background-color: rgba(255, 255, 255, 0.25);
            font-weight: bold;
        }
        
        #chatDisplay {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            padding: 15px;
            font-size: 13px;
        }
        
        #chatInput {
            background-color: white;
            border: 2px solid #e0e0e0;
            border-radius: 25px;
            padding: 12px 20px;
            font-size: 14px;
        }
        
        #chatInput:focus {
            border-color: #667eea;
        }
        
        #sendBtn {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #667eea, stop:1 #764ba2);
            color: white;
            border: none;
            border-radius: 25px;
            font-weight: bold;
            font-size: 14px;
        }
        
        #sendBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #5568d3, stop:1 #653a8b);
        }
        
        #modelsList {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            padding: 5px;
        }
        
        #modelsList::item {
            padding: 12px;
            border-bottom: 1px solid #f0f0f0;
            border-radius: 4px;
        }
        
        #modelsList::item:selected {
            background-color: #f0f4ff;
            color: #667eea;
        }
        
        #modelsList::item:hover {
            background-color: #f8f8f8;
        }
        
        #detailsBox {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            padding: 15px;
        }
        
        #downloadBtn, #addBtn, #useBtn {
            background-color: #667eea;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-weight: bold;
        }
        
        #downloadBtn:hover, #addBtn:hover, #useBtn:hover {
            background-color: #5568d3;
        }
        
        #removeBtn {
            background-color: #ef4444;
            color: white;
            border: none;
            border-radius: 8px;
            padding: 10px 20px;
            font-weight: bold;
        }
        
        #removeBtn:hover {
            background-color: #dc2626;
        }
        
        #statusLabel {
            color: #10b981;
            font-weight: bold;
        }
        
        #countLabel {
            color: #667eea;
            font-weight: bold;
            padding: 5px 10px;
            background-color: #f0f4ff;
            border-radius: 4px;
        }
        
        QComboBox {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 5px 10px;
        }
        
        QLineEdit {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            padding: 5px 10px;
        }
        
        QLineEdit:focus {
            border-color: #667eea;
        }
        
        QProgressBar {
            border: 1px solid #e0e0e0;
            border-radius: 6px;
            text-align: center;
            background-color: #f0f0f0;
        }
        
        QProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #667eea, stop:1 #764ba2);
            border-radius: 6px;
        }
    )";
    
    setStyleSheet(styleSheet);
}

void MainWindow::loadAvailableModels()
{
    QJsonArray models = m_backend->getAllHuggingFaceModels();
    
    m_availableModels.clear();
    m_availableModelsList->clear();
    
    QSet<QString> taskTypes;
    
    for (const QJsonValue &value : models) {
        QJsonObject model = value.toObject();
        m_availableModels.append(model);
        
        QString name = model["name"].toString();
        QString size = model["size"].toString();
        QString task = model["task_type"].toString();
        QString rating = model["rating"].toString();
        
        taskTypes.insert(task);
        
        // Create nice formatted item
        QString displayText = QString("📦 %1\n   Size: %2 | Task: %3").arg(name, size, task);
        if (rating != "N/A") {
            displayText += QString(" | ⭐ %1").arg(rating);
        }
        
        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, QVariant::fromValue(model));
        m_availableModelsList->addItem(item);
    }
    
    // Update task filter
    m_taskTypeFilter->clear();
    m_taskTypeFilter->addItem("All Tasks");
    for (const QString &task : taskTypes) {
        m_taskTypeFilter->addItem(task);
    }
    
    m_downloadCountLabel->setText(QString("%1 models available").arg(models.size()));
    qDebug() << "Loaded" << models.size() << "available models";
}

void MainWindow::loadInstalledModels()
{
    // TODO: Load from backend
    m_installedCountLabel->setText("0 models installed");
}

void MainWindow::updateModelSelector()
{
    m_modelSelector->clear();
    
    for (const QJsonObject &model : m_availableModels) {
        QString name = model["name"].toString();
        QString size = model["size"].toString();
        m_modelSelector->addItem(QString("%1 (%2)").arg(name, size), name);
    }
    
    if (m_modelSelector->count() > 0) {
        m_modelSelector->setCurrentIndex(0);
    }
}

void MainWindow::showModelDetails(QListWidgetItem *item)
{
    if (!item) return;
    
    QJsonObject model = item->data(Qt::UserRole).toJsonObject();
    
    QString details = QString("<h2 style='color: #667eea;'>%1</h2>").arg(model["name"].toString());
    details += "<table cellpadding='8' style='font-size: 13px;'>";
    details += QString("<tr><td><b>📏 Size:</b></td><td>%1</td></tr>").arg(model["size"].toString());
    details += QString("<tr><td><b>🎯 Task:</b></td><td>%1</td></tr>").arg(model["task_type"].toString());
    
    QString rating = model["rating"].toString();
    if (rating != "N/A") {
        details += QString("<tr><td><b>⭐ Rating:</b></td><td>%1</td></tr>").arg(rating);
    }
    
    details += QString("<tr><td><b>🔗 URL:</b></td><td><a href='%1' style='color: #667eea;'>View on Hugging Face</a></td></tr>")
                   .arg(model["url"].toString());
    details += QString("<tr><td><b>📥 Downloaded:</b></td><td>%1</td></tr>")
                   .arg(model["downloaded"].toBool() ? "✓ Yes" : "✗ No");
    details += "</table>";
    
    // Update the appropriate details widget
    if (sender() == m_availableModelsList) {
        m_downloadModelDetails->setHtml(details);
        m_downloadModelBtn->setEnabled(!model["downloaded"].toBool());
    } else if (sender() == m_installedModelsList) {
        m_installedModelDetails->setHtml(details);
    }
}

void MainWindow::onSendMessage()
{
    QString message = m_chatInput->text().trimmed();
    if (message.isEmpty()) return;
    
    QString currentModel = m_modelSelector->currentData().toString();
    if (currentModel.isEmpty()) {
        QMessageBox::warning(this, "No Model", "Please select a model first.");
        return;
    }
    
    appendChatMessage("You", message);
    m_chatInput->clear();
    
    QJsonObject request;
    request["model"] = currentModel;
    
    QJsonArray messages;
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = message;
    messages.append(userMsg);
    request["messages"] = messages;
    
    m_statusLabel->setText("🤔 Thinking...");
    m_statusLabel->setStyleSheet("color: #f59e0b; font-weight: bold;");
    
    QJsonObject response = m_backend->chatWithHuggingFace(request);
    
    m_statusLabel->setText("✓ Ready");
    m_statusLabel->setStyleSheet("color: #10b981; font-weight: bold;");
    
    QString responseText = response["content"].toString();
    if (responseText.isEmpty()) {
        responseText = "⚠️ Model inference not yet implemented. This is v0.2.0 ALPHA - inference coming soon!";
    }
    appendChatMessage("Assistant", responseText);
}

void MainWindow::appendChatMessage(const QString &role, const QString &message)
{
    QString html = m_chatDisplay->toHtml();
    
    QString roleColor = (role == "You") ? "#667eea" : "#10b981";
    QString icon = (role == "You") ? "👤" : "🤖";
    
    QString msgHtml = QString(
        "<div style='margin-bottom: 20px; padding: 15px; background-color: %1; border-radius: 8px;'>"
        "<b style='color: %2; font-size: 14px;'>%3 %4</b><br><br>"
        "<span style='color: #1f2937;'>%5</span>"
        "</div>")
        .arg(role == "You" ? "#f0f4ff" : "#f0fdf4")
        .arg(roleColor)
        .arg(icon)
        .arg(role)
        .arg(message.toHtmlEscaped());
    
    m_chatDisplay->setHtml(html + msgHtml);
    
    QScrollBar *scrollBar = m_chatDisplay->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

void MainWindow::onModelSelected(const QString &modelName)
{
    m_currentModel = modelName;
    m_statusLabel->setText(QString("Selected: %1").arg(modelName));
}

void MainWindow::onRefreshModels()
{
    m_statusLabel->setText("🔄 Refreshing...");
    loadAvailableModels();
    updateModelSelector();
    m_statusLabel->setText("✓ Refreshed");
}

void MainWindow::onDownloadModel()
{
    QListWidgetItem *currentItem = m_availableModelsList->currentItem();
    if (!currentItem) {
        QMessageBox::warning(this, "No Selection", "Please select a model to download.");
        return;
    }
    
    QJsonObject model = currentItem->data(Qt::UserRole).toJsonObject();
    QString modelName = model["name"].toString();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Download Model",
        QString("Download %1?\n\nThis may take some time depending on model size and your internet connection.").arg(modelName),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        m_downloadProgress->setVisible(true);
        m_downloadProgress->setValue(0);
        m_downloadModelBtn->setEnabled(false);
        
        QString result = m_backend->downloadHuggingFaceModel(modelName);
        
        // Simulate progress
        for (int i = 0; i <= 100; i += 5) {
            m_downloadProgress->setValue(i);
            QApplication::processEvents();
        }
        
        m_downloadProgress->setVisible(false);
        m_downloadModelBtn->setEnabled(true);
        
        QMessageBox::information(this, "Download", result);
        loadAvailableModels();
        loadInstalledModels();
    }
}

void MainWindow::onRemoveModel()
{
    QListWidgetItem *currentItem = m_installedModelsList->currentItem();
    if (!currentItem) return;
    
    QJsonObject model = currentItem->data(Qt::UserRole).toJsonObject();
    QString modelName = model["name"].toString();
    
    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        "Remove Model",
        QString("Are you sure you want to remove %1?").arg(modelName),
        QMessageBox::Yes | QMessageBox::No
    );
    
    if (reply == QMessageBox::Yes) {
        QString result = m_backend->removeHuggingFaceModel(modelName);
        QMessageBox::information(this, "Removed", result);
        loadInstalledModels();
    }
}

void MainWindow::onModelSearchChanged(const QString &text)
{
    for (int i = 0; i < m_availableModelsList->count(); ++i) {
        QListWidgetItem *item = m_availableModelsList->item(i);
        bool matches = item->text().contains(text, Qt::CaseInsensitive);
        item->setHidden(!matches);
    }
}

void MainWindow::onNewChatClicked()
{
    m_chatDisplay->clear();
    m_chatInput->clear();
    appendChatMessage("System", "New chat started. Select a model and start chatting!");
}

void MainWindow::onAboutClicked()
{
    QMessageBox::about(this, "About RunMyModel Desktop",
        "<h2>RunMyModel Desktop v0.2.0 ALPHA</h2>"
        "<p><b>A powerful desktop application for running AI models locally</b></p>"
        "<p>Features:</p>"
        "<ul>"
        "<li>✓ 361+ pre-configured AI models</li>"
        "<li>✓ Native Qt application (no web dependencies)</li>"
        "<li>✓ Model browsing and downloading</li>"
        "<li>✓ Custom model support</li>"
        "<li>✓ Cross-platform (Linux, Windows)</li>"
        "</ul>"
        "<p><b>© 2025 RunMyModel.org</b></p>"
        "<p>Licensed under Mozilla Public License 2.0</p>"
    );
}

void MainWindow::switchToPage(int index)
{
    m_contentStack->setCurrentIndex(index);
    
    // Update button states
    m_chatPageBtn->setChecked(index == 0);
    m_downloadPageBtn->setChecked(index == 1);
    m_installedPageBtn->setChecked(index == 2);
    m_customPageBtn->setChecked(index == 3);
    
    // Refresh data when switching pages
    if (index == 1) {
        loadAvailableModels();
    } else if (index == 2) {
        loadInstalledModels();
    }
}

void MainWindow::onPageChanged(const QString &page)
{
    // Not used - replaced by switchToPage
}

void MainWindow::onBrowseModelsClicked()
{
    switchToPage(1);
}

