#include "mainwindow.h"
#include <QApplication>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollBar>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
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
    
    // Connect backend signals for download progress
    connect(m_backend, &Backend::modelDownloadProgress, this, &MainWindow::onModelDownloadProgress);
    connect(m_backend, &Backend::modelDownloadComplete, this, &MainWindow::onModelDownloadComplete);
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
    setupDownloadDock();
    
    // Add pages to stack
    m_contentStack->addWidget(m_chatPage);
    m_contentStack->addWidget(m_downloadPage);
    m_contentStack->addWidget(m_installedPage);
    m_contentStack->addWidget(m_customPage);
    
    // Show chat page by default
    switchToPage(0);
}

void MainWindow::setupDownloadDock()
{
    m_downloadPanel = new DownloadPanel();
    
    m_downloadDock = new QDockWidget("Downloads", this);
    m_downloadDock->setObjectName("downloadDock");
    m_downloadDock->setWidget(m_downloadPanel);
    m_downloadDock->setAllowedAreas(Qt::RightDockWidgetArea | Qt::BottomDockWidgetArea);
    m_downloadDock->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    
    addDockWidget(Qt::RightDockWidgetArea, m_downloadDock);
    m_downloadDock->hide(); // Hide by default
    
    // Connect signals
    connect(m_downloadPanel, &DownloadPanel::cancelDownloadRequested, [this](const QString &modelName) {
        // TODO: Implement cancel
        qDebug() << "Cancel requested for:" << modelName;
    });
    
    connect(m_downloadPanel, &DownloadPanel::allDownloadsComplete, [this]() {
        m_downloadDock->hide();
    });
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
    
    QLabel *subtitleLabel = new QLabel("Explore 361+ AI models - Steam-style cards");
    subtitleLabel->setStyleSheet("color: #6b7280;");
    layout->addWidget(subtitleLabel);
    
    // Search and filters
    QHBoxLayout *filterBar = new QHBoxLayout();
    
    m_downloadSearch = new QLineEdit();
    m_downloadSearch->setPlaceholderText("🔍 Search models by name, task, or size...");
    m_downloadSearch->setMinimumHeight(40);
    filterBar->addWidget(m_downloadSearch);
    
    m_taskTypeFilter = new QComboBox();
    m_taskTypeFilter->addItem("All Tasks");
    m_taskTypeFilter->setMinimumHeight(40);
    m_taskTypeFilter->setMinimumWidth(200);
    filterBar->addWidget(m_taskTypeFilter);
    
    m_sortByCombo = new QComboBox();
    m_sortByCombo->addItems({"Sort by Rating", "Sort by Name", "Sort by Size"});
    m_sortByCombo->setMinimumHeight(40);
    m_sortByCombo->setMinimumWidth(180);
    filterBar->addWidget(m_sortByCombo);
    
    m_downloadCountLabel = new QLabel("0 models");
    m_downloadCountLabel->setObjectName("countLabel");
    filterBar->addWidget(m_downloadCountLabel);
    
    layout->addLayout(filterBar);
    
    // Steam-style grid of model cards
    m_modelsScrollArea = new QScrollArea();
    m_modelsScrollArea->setWidgetResizable(true);
    m_modelsScrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_modelsScrollArea->setObjectName("modelsScroll");
    
    m_modelsContainer = new QWidget();
    m_modelsGrid = new QGridLayout(m_modelsContainer);
    m_modelsGrid->setSpacing(15);
    m_modelsGrid->setContentsMargins(0, 0, 0, 0);
    
    m_modelsScrollArea->setWidget(m_modelsContainer);
    layout->addWidget(m_modelsScrollArea);
    
    // Connect signals
    connect(m_downloadSearch, &QLineEdit::textChanged, this, &MainWindow::onModelSearchChanged);
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
    
    QLabel *descLabel = new QLabel("Load custom GGUF/GGML models from your filesystem");
    descLabel->setStyleSheet("color: #6b7280;");
    layout->addWidget(descLabel);
    
    layout->addSpacing(20);
    
    // Create a nice form card
    QFrame *formCard = new QFrame();
    formCard->setObjectName("formCard");
    QVBoxLayout *formLayout = new QVBoxLayout(formCard);
    formLayout->setContentsMargins(20, 20, 20, 20);
    formLayout->setSpacing(15);
    
    // Model path
    QLabel *pathLabel = new QLabel("<b>Model File:</b>");
    formLayout->addWidget(pathLabel);
    
    QHBoxLayout *pathRow = new QHBoxLayout();
    m_customModelPath = new QLineEdit();
    m_customModelPath->setPlaceholderText("Select a .gguf or .ggml file...");
    m_customModelPath->setMinimumHeight(40);
    pathRow->addWidget(m_customModelPath);
    
    m_browseCustomBtn = new QPushButton("📁 Browse...");
    m_browseCustomBtn->setMinimumHeight(40);
    m_browseCustomBtn->setMinimumWidth(120);
    connect(m_browseCustomBtn, &QPushButton::clicked, [this]() {
        QString fileName = QFileDialog::getOpenFileName(this, "Select Model File", 
                                                        QDir::homePath(), 
                                                        "Model Files (*.gguf *.ggml);;All Files (*)");
        if (!fileName.isEmpty()) {
            m_customModelPath->setText(fileName);
            // Auto-fill model name from filename
            QFileInfo fileInfo(fileName);
            m_customModelName->setText(fileInfo.baseName());
        }
    });
    pathRow->addWidget(m_browseCustomBtn);
    formLayout->addLayout(pathRow);
    
    // Model name
    QLabel *nameLabel = new QLabel("<b>Model Name:</b>");
    formLayout->addWidget(nameLabel);
    
    m_customModelName = new QLineEdit();
    m_customModelName->setPlaceholderText("e.g., my-custom-llama-7b");
    m_customModelName->setMinimumHeight(40);
    formLayout->addWidget(m_customModelName);
    
    // Model format
    QLabel *formatLabel = new QLabel("<b>Format:</b>");
    formLayout->addWidget(formatLabel);
    
    m_customModelFormat = new QComboBox();
    m_customModelFormat->addItems({"GGUF", "GGML", "PyTorch", "TensorFlow", "Other"});
    m_customModelFormat->setMinimumHeight(40);
    formLayout->addWidget(m_customModelFormat);
    
    // Configuration (optional)
    QLabel *configLabel = new QLabel("<b>Configuration (Optional JSON):</b>");
    formLayout->addWidget(configLabel);
    
    m_customModelConfig = new QTextEdit();
    m_customModelConfig->setPlaceholderText("{\n  \"context_length\": 4096,\n  \"temperature\": 0.7,\n  \"max_tokens\": 2048\n}");
    m_customModelConfig->setMaximumHeight(120);
    formLayout->addWidget(m_customModelConfig);
    
    // Add button
    m_addCustomModelBtn = new QPushButton("➕ Add Custom Model");
    m_addCustomModelBtn->setObjectName("addBtn");
    m_addCustomModelBtn->setMinimumHeight(50);
    connect(m_addCustomModelBtn, &QPushButton::clicked, this, &MainWindow::onAddCustomModel);
    formLayout->addWidget(m_addCustomModelBtn);
    
    layout->addWidget(formCard);
    
    layout->addSpacing(20);
    
    // List of custom models
    QLabel *listLabel = new QLabel("<b>Your Custom Models:</b>");
    QFont listFont = listLabel->font();
    listFont.setPointSize(14);
    listLabel->setFont(listFont);
    layout->addWidget(listLabel);
    
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
        
        /* Steam-style Model Cards */
        #modelCard {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ffffff, stop:1 #f8f9fa);
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            padding: 0px;
        }
        
        #modelCard:hover {
            border-color: #667eea;
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #f8f9ff, stop:1 #f0f4ff);
        }
        
        #modelCardName {
            color: #1f2937;
            font-weight: bold;
        }
        
        #modelStat {
            color: #6b7280;
            font-size: 12px;
        }
        
        #statusBadge {
            background-color: #6b7280;
            color: white;
            padding: 4px 12px;
            border-radius: 12px;
            font-size: 11px;
            font-weight: bold;
        }
        
        #modelCardButton {
            background-color: #667eea;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
            font-size: 12px;
        }
        
        #modelCardButton:hover {
            background-color: #5568d3;
        }
        
        #modelCardButton:disabled {
            background-color: #9ca3af;
        }
        
        #modelProgress {
            border: none;
            background-color: #e5e7eb;
            border-radius: 3px;
        }
        
        #modelProgress::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #10b981, stop:1 #059669);
            border-radius: 3px;
        }
        
        #modelsScroll {
            background-color: transparent;
            border: none;
        }
        
        #formCard {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 12px;
        }
        
        /* Download Panel */
        #downloadDock {
            background-color: #f9fafb;
        }
        
        #downloadPanel {
            background-color: #f9fafb;
        }
        
        #downloadItem {
            background-color: white;
            border: 1px solid #e5e7eb;
            border-radius: 8px;
        }
        
        #downloadItemName {
            color: #1f2937;
        }
        
        #downloadStatus {
            color: #6b7280;
            font-size: 12px;
        }
        
        #downloadDetails {
            color: #9ca3af;
            font-size: 11px;
        }
        
        #downloadProgressBar {
            border: 1px solid #e5e7eb;
            border-radius: 4px;
            background-color: #f3f4f6;
            text-align: center;
        }
        
        #downloadProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:1 #2563eb);
            border-radius: 4px;
        }
        
        #cancelBtn {
            background-color: #ef4444;
            color: white;
            border: none;
            border-radius: 14px;
            font-weight: bold;
        }
        
        #cancelBtn:hover {
            background-color: #dc2626;
        }
        
        #emptyLabel {
            color: #9ca3af;
            font-style: italic;
        }
    )";
    
    setStyleSheet(styleSheet);
}

void MainWindow::loadAvailableModels()
{
    QJsonArray models = m_backend->getAllHuggingFaceModels();
    
    m_availableModels.clear();
    
    // Clear existing cards
    for (ModelCard *card : m_modelCards) {
        m_modelsGrid->removeWidget(card);
        card->deleteLater();
    }
    m_modelCards.clear();
    
    QSet<QString> taskTypes;
    
    int row = 0, col = 0;
    const int columnsPerRow = 2; // 2 cards per row for better layout
    
    for (const QJsonValue &value : models) {
        QJsonObject model = value.toObject();
        m_availableModels.append(model);
        
        QString task = model["task_type"].toString();
        taskTypes.insert(task);
        
        // Create Steam-style model card
        ModelCard *card = new ModelCard(model, m_modelsContainer);
        
        // Connect card signals
        connect(card, &ModelCard::downloadRequested, this, &MainWindow::onDownloadModelRequested);
        connect(card, &ModelCard::useRequested, [this](const QString &modelName) {
            m_modelSelector->setCurrentText(modelName);
            switchToPage(0); // Switch to chat
        });
        
        m_modelsGrid->addWidget(card, row, col);
        m_modelCards[model["name"].toString()] = card;
        
        col++;
        if (col >= columnsPerRow) {
            col = 0;
            row++;
        }
    }
    
    // Add spacer at the end
    m_modelsGrid->setRowStretch(row + 1, 1);
    
    // Update task filter
    m_taskTypeFilter->clear();
    m_taskTypeFilter->addItem("All Tasks");
    for (const QString &task : taskTypes) {
        m_taskTypeFilter->addItem(task);
    }
    
    m_downloadCountLabel->setText(QString("%1 models").arg(models.size()));
    qDebug() << "Loaded" << models.size() << "models with Steam-style cards";
}

void MainWindow::loadInstalledModels()
{
    // TODO: Load from backend
    m_installedCountLabel->setText("0 models installed");
}

void MainWindow::updateModelSelector()
{
    m_modelSelector->clear();
    
    // Add a placeholder
    m_modelSelector->addItem("-- Select an installed model --", "");
    
    // TODO: Only add INSTALLED models to the selector
    // For now, we'll add all models but the validation is in onSendMessage
    
    // Add custom models from the custom models list
    for (int i = 0; i < m_customModelsList->count(); ++i) {
        QListWidgetItem *item = m_customModelsList->item(i);
        QString modelPath = item->data(Qt::UserRole).toString();
        QString modelName = item->text().split('\n')[0]; // Get first line (name)
        m_modelSelector->addItem(QString("🔧 %1").arg(modelName), modelPath);
    }
    
    // TODO: Add downloaded models from backend
    // For demo purposes, keeping the list empty to encourage downloads
    
    if (m_modelSelector->count() > 1) {
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
    
    // Update installed models details
    if (sender() == m_installedModelsList) {
        m_installedModelDetails->setHtml(details);
    }
}

void MainWindow::onSendMessage()
{
    QString message = m_chatInput->text().trimmed();
    if (message.isEmpty()) return;
    
    QString currentModel = m_modelSelector->currentData().toString();
    if (currentModel.isEmpty()) {
        QMessageBox::warning(this, "No Model Selected", 
            "Please select a model from the dropdown first.");
        return;
    }
    
    // Check if model is installed
    bool isInstalled = false;
    QString modelPath = currentModel;
    
    // Check if it's a custom model (has full path)
    if (QFile::exists(modelPath)) {
        isInstalled = true;
    } else {
        // Check if it's a downloaded Hugging Face model
        // TODO: Get actual installed models from backend
        // For now, we'll allow the message but show a warning
        QMessageBox msgBox(this);
        msgBox.setWindowTitle("Model Not Installed");
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText(QString("<h3>%1 is not installed</h3>").arg(currentModel));
        msgBox.setInformativeText(
            "This model needs to be downloaded before you can use it.\n\n"
            "📥 Go to 'Browse Models' to download it.\n"
            "⚙️ Or add it as a custom model if you have it locally.\n\n"
            "Inference is not available for uninstalled models."
        );
        msgBox.setStandardButtons(QMessageBox::Ok);
        msgBox.exec();
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

// onDownloadModel removed - using onDownloadModelRequested with cards instead

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
    // Filter model cards based on search text
    for (auto it = m_modelCards.begin(); it != m_modelCards.end(); ++it) {
        QString modelName = it.key();
        bool matches = modelName.contains(text, Qt::CaseInsensitive);
        it.value()->setVisible(matches);
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


void MainWindow::onModelDownloadProgress(const QString &modelName, double progress)
{
    if (m_modelCards.contains(modelName)) {
        m_modelCards[modelName]->setDownloadProgress(static_cast<int>(progress));
    }
    
    // Update download panel - get detailed info from backend
    QJsonObject progressInfo = m_backend->getModelDownloadProgress(modelName);
    qint64 downloaded = progressInfo["downloaded_bytes"].toInteger();
    qint64 total = progressInfo["total_bytes"].toInteger();
    double speed = progressInfo["speed"].toDouble(0);
    
    m_downloadPanel->updateDownload(modelName, progress, downloaded, total, speed);
    
    qDebug() << "Download progress:" << modelName << progress << "%";
}

void MainWindow::onModelDownloadComplete(const QString &modelName)
{
    if (m_modelCards.contains(modelName)) {
        m_modelCards[modelName]->setInstalled(true);
    }
    
    // Update download panel
    m_downloadPanel->completeDownload(modelName);
    
    // Show toast notification instead of blocking dialog
    m_statusLabel->setText(QString("✅ Downloaded: %1").arg(modelName));
    m_statusLabel->setStyleSheet("color: #10b981; font-weight: bold;");
    
    loadInstalledModels();
    updateModelSelector();
    
    qDebug() << "Download complete:" << modelName;
}

void MainWindow::onDownloadModelRequested(const QString &modelName)
{
    qDebug() << "Download requested for:" << modelName;
    
    // Show confirmation dialog
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Confirm Download");
    msgBox.setIcon(QMessageBox::Question);
    msgBox.setText(QString("<h3>Download %1?</h3>").arg(modelName));
    msgBox.setInformativeText(
        "This will download the model from Hugging Face.\n\n"
        "⚠️ Large models may take significant time and bandwidth.\n"
        "📁 Models are saved to your cache directory.\n"
        "📊 Progress will be shown in the download panel.\n\n"
        "Do you want to proceed?"
    );
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    
    int ret = msgBox.exec();
    
    if (ret == QMessageBox::Yes) {
        // Update card status
        if (m_modelCards.contains(modelName)) {
            m_modelCards[modelName]->setDownloading(true);
        }
        
        // Add to download panel
        m_downloadPanel->addDownload(modelName);
        m_downloadDock->show();
        
        // Start download
        QString result = m_backend->downloadHuggingFaceModel(modelName);
        qDebug() << "Download started:" << result;
    } else {
        qDebug() << "Download cancelled by user:" << modelName;
    }
}

void MainWindow::onAddCustomModel()
{
    QString modelPath = m_customModelPath->text().trimmed();
    QString modelName = m_customModelName->text().trimmed();
    QString format = m_customModelFormat->currentText();
    QString config = m_customModelConfig->toPlainText().trimmed();
    
    // Validation
    if (modelPath.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please select a model file.");
        return;
    }
    
    if (modelName.isEmpty()) {
        QMessageBox::warning(this, "Missing Information", "Please enter a model name.");
        return;
    }
    
    QFileInfo fileInfo(modelPath);
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "File Not Found", 
            QString("The file '%1' does not exist.").arg(modelPath));
        return;
    }
    
    // Create custom model entry
    QString displayText = QString("%1\n  Format: %2 | Size: %3 MB\n  Path: %4")
        .arg(modelName)
        .arg(format)
        .arg(fileInfo.size() / (1024 * 1024))
        .arg(modelPath);
    
    QListWidgetItem *item = new QListWidgetItem(displayText);
    item->setData(Qt::UserRole, modelPath);
    m_customModelsList->addItem(item);
    
    // Show success message
    QMessageBox msgBox(this);
    msgBox.setWindowTitle("Custom Model Added");
    msgBox.setIcon(QMessageBox::Information);
    msgBox.setText(QString("<h3>✅ Success!</h3>"));
    msgBox.setInformativeText(
        QString("Custom model <b>%1</b> has been added successfully!\n\n"
                "📍 Location: %2\n"
                "📊 Size: %3 MB\n"
                "🎯 Format: %4\n\n"
                "You can now select it from the Chat page to start using it.")
            .arg(modelName)
            .arg(modelPath)
            .arg(fileInfo.size() / (1024 * 1024))
            .arg(format)
    );
    msgBox.setStandardButtons(QMessageBox::Ok);
    msgBox.exec();
    
    // Update model selector
    updateModelSelector();
    
    // Clear form
    m_customModelPath->clear();
    m_customModelName->clear();
    m_customModelConfig->clear();
    m_customModelFormat->setCurrentIndex(0);
    
    qDebug() << "Custom model added:" << modelName << "at" << modelPath;
}
