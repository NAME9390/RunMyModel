#include "mainwindow.h"
#include <QApplication>
#include <QClipboard>
#include <QGroupBox>
#include <QMessageBox>
#include <QScrollBar>
#include <QFileDialog>
#include <QFileInfo>
#include <QDir>
#include <QTimer>
#include <QSplitter>
#include <QStack>
#include <QJsonArray>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

MainWindow::MainWindow(BackendClient *backendClient, QWidget *parent)
    : QMainWindow(parent)
    , m_backend(new Backend(this))
    , m_backendClient(backendClient)
    , m_detectedVRAM(0)
{
    // Performance optimizations
    setAttribute(Qt::WA_DontCreateNativeAncestors);
    setAttribute(Qt::WA_NativeWindow);
    
    setWindowTitle("RunMyModel Desktop - v0.3.0 BETA");
    resize(1400, 900);
    
    // Detect GPU
    m_gpuDetector = new GPUDetector(this);
    GPUDetector::GPUInfo gpuInfo = m_gpuDetector->detectGPU();
    m_detectedVRAM = gpuInfo.vramMB;
    
    setupUI();
    applyModernStyling();
    
    // Load models AFTER UI is fully set up (to avoid null pointer crashes)
    QTimer::singleShot(0, this, [this]() {
        updateModelSelector();
    });
    
    // Connect backend signals for download progress
    connect(m_backend, &Backend::modelDownloadProgress, this, &MainWindow::onModelDownloadProgress);
    connect(m_backend, &Backend::modelDownloadComplete, this, &MainWindow::onModelDownloadComplete);
    connect(m_backend, &Backend::modelDownloadError, this, &MainWindow::onModelDownloadError);
    
    // Connect Python backend signals for real inference
    if (m_backendClient) {
        connect(m_backendClient, &BackendClient::streamToken, this, &MainWindow::onStreamToken);
        connect(m_backendClient, &BackendClient::streamComplete, this, &MainWindow::onStreamComplete);
        connect(m_backendClient, &BackendClient::streamError, this, &MainWindow::onStreamError);
        connect(m_backendClient, &BackendClient::modelsListed, this, &MainWindow::onBackendModelsListed);
        connect(m_backendClient, &BackendClient::modelLoaded, this, &MainWindow::onBackendModelLoaded);
    }
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
    setupPromptArchitectPage();
    setupCustomPage();

    // Add pages to stack
    m_contentStack->addWidget(m_chatPage);
    m_contentStack->addWidget(m_promptArchitectPage);
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
        qDebug() << "Cancel requested for:" << modelName;
        bool cancelled = m_backend->cancelModelDownload(modelName);
        if (cancelled) {
            m_downloadPanel->errorDownload(modelName, "Cancelled");
            if (m_modelCards.contains(modelName)) {
                m_modelCards[modelName]->setDownloading(false);
            }
        }
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
    
    // Navigation buttons - ONLY 3 pages now
    m_chatPageBtn = createSidebarButton("💬", "Chat");
    connect(m_chatPageBtn, &QPushButton::clicked, [this]() { switchToPage(0); });
    m_sidebarLayout->addWidget(m_chatPageBtn);
    
    m_promptArchitectBtn = createSidebarButton("🎨", "Visual Designer");
    connect(m_promptArchitectBtn, &QPushButton::clicked, [this]() { switchToPage(1); });
    m_sidebarLayout->addWidget(m_promptArchitectBtn);
    
    m_customPageBtn = createSidebarButton("📦", "My Models");
    connect(m_customPageBtn, &QPushButton::clicked, [this]() { switchToPage(2); });
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
    
    // Title with better styling
    QLabel *titleLabel = new QLabel("💬 Chat");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(28);
    titleFont.setBold(true);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, -1.2);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #111827; font-weight: 800; margin-bottom: 4px;");
    layout->addWidget(titleLabel);
    
    QLabel *subtitle = new QLabel("Chat with your local AI models");
    subtitle->setStyleSheet("color: #6b7280; font-size: 14px; margin-bottom: 16px;");
    layout->addWidget(subtitle);
    
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

void MainWindow::setupPromptArchitectPage()
{
    // Visual UI Designer - drag and drop blocks (squares, shapes, text, buttons)
    m_promptArchitectPage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(m_promptArchitectPage);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    
    // Top toolbar
    QFrame *toolbar = new QFrame();
    toolbar->setObjectName("designerToolbar");
    toolbar->setFixedHeight(60);
    QHBoxLayout *toolbarLayout = new QHBoxLayout(toolbar);
    
    QLabel *titleLabel = new QLabel("🎨 Advanced UI Designer");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleFont.setLetterSpacing(QFont::AbsoluteSpacing, -0.8);
    titleLabel->setFont(titleFont);
    titleLabel->setStyleSheet("color: #111827; font-weight: 700;");
    toolbarLayout->addWidget(titleLabel);
    
    QLabel *subtitle = new QLabel("Design → AI Prompt → Code");
    subtitle->setStyleSheet("color: #6b7280; font-size: 12px; margin-left: 10px;");
    toolbarLayout->addWidget(subtitle);
    
    // Layer controls
    QLabel *layersLabel = new QLabel("Layers: 0");
    layersLabel->setObjectName("layersLabel");
    layersLabel->setStyleSheet("color: #6b7280; font-size: 11px; margin-left: 20px; padding: 4px 12px; background: #f3f4f6; border-radius: 6px;");
    toolbarLayout->addWidget(layersLabel);
    
    toolbarLayout->addStretch();
    
    QPushButton *undoBtn = new QPushButton("↶ Undo");
    undoBtn->setObjectName("toolbarBtn");
    undoBtn->setMinimumHeight(38);
    undoBtn->setMinimumWidth(90);
    undoBtn->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(undoBtn);
    
    QPushButton *redoBtn = new QPushButton("↷ Redo");
    redoBtn->setObjectName("toolbarBtn");
    redoBtn->setMinimumHeight(38);
    redoBtn->setMinimumWidth(90);
    redoBtn->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(redoBtn);
    
    QPushButton *clearBtn = new QPushButton("🗑️ Clear");
    clearBtn->setObjectName("toolbarBtn");
    clearBtn->setMinimumHeight(38);
    clearBtn->setMinimumWidth(90);
    clearBtn->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(clearBtn);
    
    QPushButton *exportCodeBtn = new QPushButton("💻 Export Code");
    exportCodeBtn->setObjectName("toolbarBtn");
    exportCodeBtn->setMinimumHeight(38);
    exportCodeBtn->setMinimumWidth(120);
    exportCodeBtn->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(exportCodeBtn);
    
    QPushButton *exportPromptBtn = new QPushButton("💾 Export Prompt");
    exportPromptBtn->setObjectName("toolbarBtn");
    exportPromptBtn->setMinimumHeight(38);
    exportPromptBtn->setMinimumWidth(130);
    exportPromptBtn->setCursor(Qt::PointingHandCursor);
    toolbarLayout->addWidget(exportPromptBtn);
    
    layout->addWidget(toolbar);
    
    // Main split: Component palette | Canvas | Properties panel
    QSplitter *splitter = new QSplitter(Qt::Horizontal);
    
    // LEFT: Block Palette
    QFrame *paletteFrame = new QFrame();
    paletteFrame->setObjectName("blockPalette");
    paletteFrame->setMinimumWidth(250);
    paletteFrame->setMaximumWidth(300);
    
    QVBoxLayout *paletteLayout = new QVBoxLayout(paletteFrame);
    paletteLayout->setContentsMargins(15, 15, 15, 15);
    
    QLabel *paletteTitle = new QLabel("UI Components");
    QFont paletteFont = paletteTitle->font();
    paletteFont.setPointSize(13);
    paletteFont.setBold(true);
    paletteTitle->setFont(paletteFont);
    paletteTitle->setStyleSheet("color: #111827; font-weight: 700; padding: 8px 0;");
    paletteLayout->addWidget(paletteTitle);
    
    QLabel *paletteSubtitle = new QLabel("Drag to canvas to design UI");
    paletteSubtitle->setStyleSheet("color: #6b7280; font-size: 11px; padding-bottom: 4px;");
    paletteLayout->addWidget(paletteSubtitle);
    
    paletteLayout->addSpacing(10);
    
    // Block categories
    QScrollArea *paletteScroll = new QScrollArea();
    paletteScroll->setWidgetResizable(true);
    paletteScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    QWidget *paletteContent = new QWidget();
    QVBoxLayout *blocksLayout = new QVBoxLayout(paletteContent);
    blocksLayout->setSpacing(10);
    
    // LAYOUT
    QLabel *layoutLabel = new QLabel("LAYOUT");
    layoutLabel->setStyleSheet("font-weight: 600; font-size: 10px; color: #9ca3af; letter-spacing: 1px; margin-top: 8px;");
    blocksLayout->addWidget(layoutLabel);
    
    QStringList layoutComponents = {"📦 Container", "↔️ Flex Row", "↕️ Flex Column", "🗂️ Grid", "📐 Section"};
    for (const QString &comp : layoutComponents) {
        QPushButton *btn = new QPushButton(comp);
        btn->setObjectName("blockBtn");
        btn->setMinimumHeight(45);
        blocksLayout->addWidget(btn);
    }
    
    blocksLayout->addSpacing(12);
    
    // TYPOGRAPHY
    QLabel *typoLabel = new QLabel("TYPOGRAPHY");
    typoLabel->setStyleSheet("font-weight: 600; font-size: 10px; color: #9ca3af; letter-spacing: 1px; margin-top: 8px;");
    blocksLayout->addWidget(typoLabel);
    
    QStringList typoComponents = {"📝 Heading (H1)", "📄 Heading (H2-H6)", "📃 Paragraph", "📌 Link", "✨ Badge"};
    for (const QString &comp : typoComponents) {
        QPushButton *btn = new QPushButton(comp);
        btn->setObjectName("blockBtn");
        btn->setMinimumHeight(45);
        blocksLayout->addWidget(btn);
    }
    
    blocksLayout->addSpacing(12);
    
    // FORM ELEMENTS
    QLabel *formLabel = new QLabel("FORM ELEMENTS");
    formLabel->setStyleSheet("font-weight: 600; font-size: 10px; color: #9ca3af; letter-spacing: 1px; margin-top: 8px;");
    blocksLayout->addWidget(formLabel);
    
    QStringList formComponents = {"🔘 Button", "📥 Text Input", "📧 Email Input", "🔒 Password Input", 
                                    "📋 Textarea", "☑️ Checkbox", "📻 Radio Button", "📂 Select Dropdown", "📎 File Upload"};
    for (const QString &comp : formComponents) {
        QPushButton *btn = new QPushButton(comp);
        btn->setObjectName("blockBtn");
        btn->setMinimumHeight(45);
        blocksLayout->addWidget(btn);
    }
    
    blocksLayout->addSpacing(12);
    
    // MEDIA
    QLabel *mediaLabel = new QLabel("MEDIA");
    mediaLabel->setStyleSheet("font-weight: 600; font-size: 10px; color: #9ca3af; letter-spacing: 1px; margin-top: 8px;");
    blocksLayout->addWidget(mediaLabel);
    
    QStringList mediaComponents = {"🖼️ Image", "🎬 Video", "🎵 Audio", "🖼️ Gallery", "📸 Avatar"};
    for (const QString &comp : mediaComponents) {
        QPushButton *btn = new QPushButton(comp);
        btn->setObjectName("blockBtn");
        btn->setMinimumHeight(45);
        blocksLayout->addWidget(btn);
    }
    
    blocksLayout->addSpacing(12);
    
    // NAVIGATION
    QLabel *navLabel = new QLabel("NAVIGATION");
    navLabel->setStyleSheet("font-weight: 600; font-size: 10px; color: #9ca3af; letter-spacing: 1px; margin-top: 8px;");
    blocksLayout->addWidget(navLabel);
    
    QStringList navComponents = {"🧭 Navbar", "🍔 Hamburger Menu", "📍 Breadcrumb", "📑 Tabs", "🔗 Pagination"};
    for (const QString &comp : navComponents) {
        QPushButton *btn = new QPushButton(comp);
        btn->setObjectName("blockBtn");
        btn->setMinimumHeight(45);
        blocksLayout->addWidget(btn);
    }
    
    blocksLayout->addSpacing(12);
    
    // COMPONENTS
    QLabel *componentsLabel = new QLabel("COMPONENTS");
    componentsLabel->setStyleSheet("font-weight: 600; font-size: 10px; color: #9ca3af; letter-spacing: 1px; margin-top: 8px;");
    blocksLayout->addWidget(componentsLabel);
    
    QStringList advancedComponents = {"🎴 Card", "🎭 Modal/Dialog", "🔔 Alert/Toast", "🎯 Tooltip", "📊 Progress Bar",
                                       "⏳ Spinner", "🏷️ Tag", "📄 Footer", "🎪 Hero Section", "💬 Chat Bubble"};
    for (const QString &comp : advancedComponents) {
        QPushButton *btn = new QPushButton(comp);
        btn->setObjectName("blockBtn");
        btn->setMinimumHeight(45);
        blocksLayout->addWidget(btn);
    }
    
    blocksLayout->addStretch();
    
    paletteContent->setLayout(blocksLayout);
    paletteScroll->setWidget(paletteContent);
    paletteLayout->addWidget(paletteScroll);
    
    splitter->addWidget(paletteFrame);
    
    // RIGHT: Canvas
    QFrame *canvasFrame = new QFrame();
    canvasFrame->setObjectName("designCanvas");
    
    QVBoxLayout *canvasLayout = new QVBoxLayout(canvasFrame);
    canvasLayout->setContentsMargins(20, 20, 20, 20);
    
    QLabel *canvasTitle = new QLabel("Design Canvas");
    QFont canvasTitleFont = canvasTitle->font();
    canvasTitleFont.setPointSize(14);
    canvasTitleFont.setBold(true);
    canvasTitle->setFont(canvasTitleFont);
    canvasTitle->setStyleSheet("color: #111827; font-weight: 700; padding: 4px 0;");
    canvasLayout->addWidget(canvasTitle);
    
    // Split view: Design preview on top, generated prompt below
    QSplitter *designSplitter = new QSplitter(Qt::Vertical);
    
    // TOP: Visual design canvas
    QFrame *visualCanvas = new QFrame();
    visualCanvas->setObjectName("visualCanvas");
    visualCanvas->setMinimumSize(800, 400);
    visualCanvas->setStyleSheet(
        "#visualCanvas {"
        "   background-color: white;"
        "   border: 2px solid #e5e7eb;"
        "   border-radius: 12px;"
        "   padding: 20px;"
        "}"
    );
    
    QVBoxLayout *visualLayout = new QVBoxLayout(visualCanvas);
    QLabel *visualPlaceholder = new QLabel("🎨 Drag components here to design your UI\n\nYour design will appear here and an AI prompt will be generated below");
    visualPlaceholder->setAlignment(Qt::AlignCenter);
    visualPlaceholder->setStyleSheet("color: #9ca3af; font-size: 16px; padding: 80px;");
    visualLayout->addWidget(visualPlaceholder);
    
    designSplitter->addWidget(visualCanvas);
    
    // BOTTOM: Generated AI prompt
    QWidget *promptSection = new QWidget();
    QVBoxLayout *promptLayout = new QVBoxLayout(promptSection);
    promptLayout->setContentsMargins(0, 10, 0, 0);
    
    QLabel *promptLabel = new QLabel("📝 Generated AI Prompt");
    QFont promptFont = promptLabel->font();
    promptFont.setPointSize(12);
    promptFont.setBold(true);
    promptLabel->setFont(promptFont);
    promptLabel->setStyleSheet("color: #111827; font-weight: 700;");
    promptLayout->addWidget(promptLabel);
    
    QTextEdit *generatedPrompt = new QTextEdit();
    generatedPrompt->setObjectName("generatedPrompt");
    generatedPrompt->setReadOnly(true);
    generatedPrompt->setPlaceholderText("Design your UI above - I'll generate a prompt that describes it...");
    generatedPrompt->setMinimumHeight(150);
    generatedPrompt->setStyleSheet(
        "#generatedPrompt {"
        "   background-color: #f9fafb;"
        "   border: 2px solid #e5e7eb;"
        "   border-radius: 12px;"
        "   padding: 16px;"
        "   font-size: 13px;"
        "   font-family: 'Consolas', 'Monaco', monospace;"
        "   line-height: 1.6;"
        "}"
    );
    promptLayout->addWidget(generatedPrompt);
    
    // Button row
    QHBoxLayout *promptButtonsLayout = new QHBoxLayout();
    
    QPushButton *copyPromptBtn = new QPushButton("📋 Copy Prompt");
    copyPromptBtn->setObjectName("toolbarBtn");
    copyPromptBtn->setMinimumHeight(40);
    promptButtonsLayout->addWidget(copyPromptBtn);
    
    QPushButton *useToChatBtn = new QPushButton("➤ Send to Chat");
    useToChatBtn->setObjectName("toolbarBtn");
    useToChatBtn->setMinimumHeight(40);
    QFont useBtnFont = useToChatBtn->font();
    useBtnFont.setBold(true);
    useToChatBtn->setFont(useBtnFont);
    promptButtonsLayout->addWidget(useToChatBtn);
    
    promptButtonsLayout->addStretch();
    promptLayout->addLayout(promptButtonsLayout);
    
    designSplitter->addWidget(promptSection);
    designSplitter->setStretchFactor(0, 2); // Design canvas gets more space
    designSplitter->setStretchFactor(1, 1); // Prompt gets less space
    
    canvasLayout->addWidget(designSplitter);
    
    connect(useToChatBtn, &QPushButton::clicked, [this, generatedPrompt]() {
        switchToPage(0); // Go to chat
        m_chatInput->setText(generatedPrompt->toPlainText());
    });
    
    connect(copyPromptBtn, &QPushButton::clicked, [generatedPrompt]() {
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(generatedPrompt->toPlainText());
    });
    
    splitter->addWidget(canvasFrame);
    splitter->setStretchFactor(1, 1); // Canvas takes more space
    
    // RIGHT: Properties Panel
    QFrame *propertiesPanel = new QFrame();
    propertiesPanel->setObjectName("propertiesPanel");
    propertiesPanel->setMinimumWidth(280);
    propertiesPanel->setMaximumWidth(320);
    
    QVBoxLayout *propertiesLayout = new QVBoxLayout(propertiesPanel);
    propertiesLayout->setContentsMargins(15, 15, 15, 15);
    
    QLabel *propertiesTitle = new QLabel("⚙️ Properties");
    QFont propFont = propertiesTitle->font();
    propFont.setPointSize(13);
    propFont.setBold(true);
    propertiesTitle->setFont(propFont);
    propertiesTitle->setStyleSheet("color: #111827; font-weight: 700; padding: 8px 0;");
    propertiesLayout->addWidget(propertiesTitle);
    
    QLabel *propertiesSubtitle = new QLabel("Component settings");
    propertiesSubtitle->setStyleSheet("color: #6b7280; font-size: 11px; padding-bottom: 12px;");
    propertiesLayout->addWidget(propertiesSubtitle);
    
    // Properties content (dynamic based on selection)
    QScrollArea *propScroll = new QScrollArea();
    propScroll->setWidgetResizable(true);
    propScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    QWidget *propContent = new QWidget();
    QVBoxLayout *propContentLayout = new QVBoxLayout(propContent);
    propContentLayout->setSpacing(12);
    
    // Stats
    QGroupBox *statsGroup = new QGroupBox("📊 Design Stats");
    QVBoxLayout *statsLayout = new QVBoxLayout(statsGroup);
    
    QLabel *totalComponentsLabel = new QLabel("Components: 0");
    totalComponentsLabel->setObjectName("totalComponentsLabel");
    totalComponentsLabel->setStyleSheet("font-size: 13px; color: #374151; padding: 4px 0;");
    statsLayout->addWidget(totalComponentsLabel);
    
    QLabel *estimatedSizeLabel = new QLabel("Est. Code Size: 0 KB");
    estimatedSizeLabel->setObjectName("estimatedSizeLabel");
    estimatedSizeLabel->setStyleSheet("font-size: 13px; color: #374151; padding: 4px 0;");
    statsLayout->addWidget(estimatedSizeLabel);
    
    QLabel *complexityLabel = new QLabel("Complexity: Low");
    complexityLabel->setObjectName("complexityLabel");
    complexityLabel->setStyleSheet("font-size: 13px; color: #10b981; padding: 4px 0;");
    statsLayout->addWidget(complexityLabel);
    
    propContentLayout->addWidget(statsGroup);
    
    // Export Options
    QGroupBox *exportGroup = new QGroupBox("💾 Export Options");
    QVBoxLayout *exportLayout = new QVBoxLayout(exportGroup);
    
    QComboBox *frameworkSelector = new QComboBox();
    frameworkSelector->addItems({"React + TypeScript", "Next.js 14", "Vue 3", "Svelte", "HTML + Tailwind"});
    frameworkSelector->setMinimumHeight(40);
    exportLayout->addWidget(new QLabel("Framework:"));
    exportLayout->addWidget(frameworkSelector);
    
    QComboBox *styleSelector = new QComboBox();
    styleSelector->addItems({"Tailwind CSS", "CSS Modules", "Styled Components", "SCSS"});
    styleSelector->setMinimumHeight(40);
    exportLayout->addWidget(new QLabel("Styling:"));
    exportLayout->addWidget(styleSelector);
    
    propContentLayout->addWidget(exportGroup);
    
    // Quick Actions
    QGroupBox *actionsGroup = new QGroupBox("⚡ Quick Actions");
    QVBoxLayout *actionsLayout = new QVBoxLayout(actionsGroup);
    
    QPushButton *previewBtn = new QPushButton("👁️ Preview Design");
    previewBtn->setObjectName("toolbarBtn");
    previewBtn->setMinimumHeight(42);
    actionsLayout->addWidget(previewBtn);
    
    QPushButton *saveTemplateBtn = new QPushButton("💾 Save as Template");
    saveTemplateBtn->setObjectName("toolbarBtn");
    saveTemplateBtn->setMinimumHeight(42);
    actionsLayout->addWidget(saveTemplateBtn);
    
    QPushButton *generateCodeBtn = new QPushButton("🚀 Generate Code");
    generateCodeBtn->setObjectName("toolbarBtn");
    generateCodeBtn->setMinimumHeight(42);
    actionsLayout->addWidget(generateCodeBtn);
    
    propContentLayout->addWidget(actionsGroup);
    
    propContentLayout->addStretch();
    
    propContent->setLayout(propContentLayout);
    propScroll->setWidget(propContent);
    propertiesLayout->addWidget(propScroll);
    
    splitter->addWidget(propertiesPanel);
    splitter->setStretchFactor(2, 0); // Properties panel fixed width
    
    layout->addWidget(splitter);
    
    // PROFESSIONAL COMPONENT SYSTEM - Connect all 50+ components
    QStringList *designComponents = new QStringList();
    QStack<QStringList> *undoStack = new QStack<QStringList>();
    QStack<QStringList> *redoStack = new QStack<QStringList>();
    int *layerCount = new int(0);
    
    // Component definitions with detailed descriptions
    QMap<QString, QString> componentDescriptions = {
        // LAYOUT
        {"📦 Container", "Full-width responsive container (max-width 1200px) with padding"},
        {"↔️ Flex Row", "Horizontal flexbox layout with gap spacing"},
        {"↕️ Flex Column", "Vertical flexbox layout with gap spacing"},
        {"🗂️ Grid", "CSS Grid layout with responsive columns (auto-fit minmax)"},
        {"📐 Section", "Semantic section element with padding and background"},
        
        // TYPOGRAPHY
        {"📝 Heading (H1)", "Large heading (48px) with bold weight and gradient text"},
        {"📄 Heading (H2-H6)", "Subheading (24-32px) with medium weight"},
        {"📃 Paragraph", "Body text (16px) with comfortable line-height (1.6)"},
        {"📌 Link", "Hyperlink with hover underline animation"},
        {"✨ Badge", "Small colored badge/pill for status or labels"},
        
        // FORM ELEMENTS  
        {"🔘 Button", "Primary button with gradient background, hover effects, and shadow"},
        {"📥 Text Input", "Text input field with label, border, and focus state"},
        {"📧 Email Input", "Email input with validation styling"},
        {"🔒 Password Input", "Password input with toggle visibility icon"},
        {"📋 Textarea", "Multi-line text area with resize handle"},
        {"☑️ Checkbox", "Checkbox with custom styling and label"},
        {"📻 Radio Button", "Radio button group with custom styling"},
        {"📂 Select Dropdown", "Dropdown select with arrow icon"},
        {"📎 File Upload", "File upload button with drag-drop zone"},
        
        // MEDIA
        {"🖼️ Image", "Responsive image with object-fit cover and rounded corners"},
        {"🎬 Video", "HTML5 video player with custom controls"},
        {"🎵 Audio", "Audio player with custom controls"},
        {"🖼️ Gallery", "Image gallery grid with lightbox"},
        {"📸 Avatar", "Circular avatar image with border"},
        
        // NAVIGATION
        {"🧭 Navbar", "Sticky navigation bar with logo, links, and CTA button"},
        {"🍔 Hamburger Menu", "Mobile hamburger menu with slide-in drawer"},
        {"📍 Breadcrumb", "Breadcrumb navigation with separators"},
        {"📑 Tabs", "Horizontal tabs with active state indicator"},
        {"🔗 Pagination", "Numbered pagination with prev/next buttons"},
        
        // COMPONENTS
        {"🎴 Card", "Content card with image, title, description, and action button"},
        {"🎭 Modal/Dialog", "Centered modal overlay with backdrop blur"},
        {"🔔 Alert/Toast", "Notification toast with auto-dismiss and icon"},
        {"🎯 Tooltip", "Hover tooltip with arrow pointer"},
        {"📊 Progress Bar", "Animated progress bar with percentage"},
        {"⏳ Spinner", "Loading spinner animation"},
        {"🏷️ Tag", "Tag/chip component with remove button"},
        {"📄 Footer", "Footer section with columns, links, and social icons"},
        {"🎪 Hero Section", "Large hero section with heading, subtext, and CTA buttons"},
        {"💬 Chat Bubble", "Chat message bubble with avatar and timestamp"}
    };
    
    auto generateAdvancedPrompt = [designComponents, componentDescriptions]() -> QString {
        if (designComponents->isEmpty()) {
            return "";
        }
        
        QString prompt = "# Professional UI/Website Design Request\n\n";
        prompt += "Create a modern, responsive website/application with the following components:\n\n";
        prompt += "## Design Structure\n\n";
        
        for (int i = 0; i < designComponents->size(); i++) {
            QString comp = designComponents->at(i);
            QString compName = comp.split(": ").first();
            QString desc = componentDescriptions.value(compName, "Component");
            prompt += QString("%1. **%2**\n   - %3\n\n").arg(i + 1).arg(compName).arg(desc);
        }
        
        prompt += "## Technical Requirements\n\n";
        prompt += "- **Framework**: React with TypeScript (or Next.js)\n";
        prompt += "- **Styling**: Tailwind CSS with custom design system\n";
        prompt += "- **Responsive**: Mobile-first, breakpoints at 640px, 768px, 1024px, 1280px\n";
        prompt += "- **Accessibility**: ARIA labels, keyboard navigation, focus states\n";
        prompt += "- **Performance**: Lazy loading, optimized images, code splitting\n\n";
        
        prompt += "## Design Guidelines\n\n";
        prompt += "- Modern glassmorphism effects with backdrop blur\n";
        prompt += "- Smooth transitions and micro-interactions\n";
        prompt += "- Consistent color palette: Primary (#3b82f6), Secondary (#8b5cf6), Accent (#10b981)\n";
        prompt += "- Shadows: subtle elevation (0 4px 6px rgba(0,0,0,0.1))\n";
        prompt += "- Border radius: 12px for cards, 8px for buttons\n";
        prompt += "- Typography: Inter font family, proper hierarchy\n\n";
        
        prompt += "## Code Structure\n\n";
        prompt += "```\n";
        prompt += "components/\n";
        for (const QString &comp : *designComponents) {
            QString compName = comp.split(": ").first().remove(QRegularExpression("[^a-zA-Z0-9]"));
            prompt += QString("  ├─ %1.tsx\n").arg(compName);
        }
        prompt += "```\n\n";
        
        prompt += "Please provide:\n";
        prompt += "1. Complete React component code for each element\n";
        prompt += "2. Tailwind CSS configuration\n";
        prompt += "3. Full page layout combining all components\n";
        prompt += "4. Responsive design implementation\n";
        
        return prompt;
    };
    
    auto updateStats = [totalComponentsLabel, estimatedSizeLabel, complexityLabel, designComponents]() {
        int count = designComponents->size();
        totalComponentsLabel->setText(QString("Components: %1").arg(count));
        
        // Estimate code size (rough calculation)
        int estimatedKB = count * 3; // ~3KB per component
        estimatedSizeLabel->setText(QString("Est. Code Size: %1 KB").arg(estimatedKB));
        
        // Complexity rating
        QString complexity = "Low";
        QString color = "#10b981";
        if (count > 15) {
            complexity = "High";
            color = "#ef4444";
        } else if (count > 8) {
            complexity = "Medium";
            color = "#f59e0b";
        }
        complexityLabel->setText(QString("Complexity: %1").arg(complexity));
        complexityLabel->setStyleSheet(QString("font-size: 13px; color: %1; font-weight: 600; padding: 4px 0;").arg(color));
    };
    
    auto addComponent = [visualPlaceholder, generatedPrompt, designComponents, layerCount, layersLabel, generateAdvancedPrompt, undoStack, redoStack, updateStats](const QString &componentName) {
        // Save state for undo
        undoStack->push(*designComponents);
        redoStack->clear();
        
        (*layerCount)++;
        designComponents->append(QString("Layer %1: %2").arg(*layerCount).arg(componentName));
        
        // Update visual representation with tree structure
        QString visual = "🎨 Your Design Layout:\n\n";
        visual += "┌─────────────────────────────────────┐\n";
        for (const QString &component : *designComponents) {
            QString compName = component.split(": ").last();
            visual += QString("│ ├─ %1\n").arg(compName);
        }
        visual += "└─────────────────────────────────────┘\n\n";
        visual += QString("📊 Total Layers: %1").arg(*layerCount);
        
        visualPlaceholder->setText(visual);
        visualPlaceholder->setStyleSheet("color: #111827; font-family: monospace; font-size: 13px; padding: 40px; line-height: 1.8;");
        
        // Generate professional AI prompt
        generatedPrompt->setText(generateAdvancedPrompt());
        
        // Update UI
        layersLabel->setText(QString("Layers: %1").arg(*layerCount));
        updateStats();
    };
    
    // Connect ALL components dynamically
    for (int i = 0; i < paletteContent->findChildren<QPushButton*>().size(); i++) {
        QPushButton *btn = paletteContent->findChildren<QPushButton*>().at(i);
        QString btnText = btn->text();
        
        connect(btn, &QPushButton::clicked, [addComponent, btnText]() {
            addComponent(btnText);
        });
    }
    
    // Toolbar buttons - Undo/Redo
    connect(undoBtn, &QPushButton::clicked, [visualPlaceholder, generatedPrompt, designComponents, layerCount, layersLabel, generateAdvancedPrompt, undoStack, redoStack, updateStats]() {
        if (!undoStack->isEmpty()) {
            redoStack->push(*designComponents);
            *designComponents = undoStack->pop();
            *layerCount = designComponents->size();
            
            if (designComponents->isEmpty()) {
                visualPlaceholder->setText("🎨 Click components to start designing\n\nYour layout will appear here");
                visualPlaceholder->setStyleSheet("color: #9ca3af; font-size: 16px; padding: 80px;");
                generatedPrompt->clear();
            } else {
                QString visual = "🎨 Your Design Layout:\n\n┌─────────────────────────────────────┐\n";
                for (const QString &component : *designComponents) {
                    visual += QString("│ ├─ %1\n").arg(component.split(": ").last());
                }
                visual += QString("└─────────────────────────────────────┘\n\n📊 Total Layers: %1").arg(*layerCount);
                visualPlaceholder->setText(visual);
                visualPlaceholder->setStyleSheet("color: #111827; font-family: monospace; font-size: 13px; padding: 40px; line-height: 1.8;");
                generatedPrompt->setText(generateAdvancedPrompt());
            }
            
            layersLabel->setText(QString("Layers: %1").arg(*layerCount));
            updateStats();
        }
    });
    
    connect(redoBtn, &QPushButton::clicked, [visualPlaceholder, generatedPrompt, designComponents, layerCount, layersLabel, generateAdvancedPrompt, undoStack, redoStack, updateStats]() {
        if (!redoStack->isEmpty()) {
            undoStack->push(*designComponents);
            *designComponents = redoStack->pop();
            *layerCount = designComponents->size();
            
            QString visual = "🎨 Your Design Layout:\n\n┌─────────────────────────────────────┐\n";
            for (const QString &component : *designComponents) {
                visual += QString("│ ├─ %1\n").arg(component.split(": ").last());
            }
            visual += QString("└─────────────────────────────────────┘\n\n📊 Total Layers: %1").arg(*layerCount);
            visualPlaceholder->setText(visual);
            visualPlaceholder->setStyleSheet("color: #111827; font-family: monospace; font-size: 13px; padding: 40px; line-height: 1.8;");
            generatedPrompt->setText(generateAdvancedPrompt());
            
            layersLabel->setText(QString("Layers: %1").arg(*layerCount));
            updateStats();
        }
    });
    
    connect(clearBtn, &QPushButton::clicked, [visualPlaceholder, generatedPrompt, designComponents, layerCount, layersLabel, undoStack, redoStack, updateStats]() {
        undoStack->push(*designComponents);
        redoStack->clear();
        designComponents->clear();
        *layerCount = 0;
        visualPlaceholder->setText("🎨 Click components to start designing\n\nYour layout will appear here");
        visualPlaceholder->setStyleSheet("color: #9ca3af; font-size: 16px; padding: 80px;");
        generatedPrompt->clear();
        layersLabel->setText("Layers: 0");
        updateStats();
    });
    
    connect(exportPromptBtn, &QPushButton::clicked, [this, generatedPrompt]() {
        QString filename = QFileDialog::getSaveFileName(this, "Export AI Prompt", QDir::homePath(), "Markdown Files (*.md);;Text Files (*.txt)");
        if (!filename.isEmpty()) {
            QFile file(filename);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QTextStream out(&file);
                out << generatedPrompt->toPlainText();
                file.close();
                m_statusLabel->setText("✅ Prompt exported successfully!");
            }
        }
    });
    
    connect(exportCodeBtn, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Export Code", 
            "📝 Code Export Feature\n\n"
            "This will export the actual React/HTML code for your design.\n\n"
            "For now, use the generated AI prompt and send it to an AI model to get the code!\n\n"
            "Advanced code export coming in v0.4.0!");
    });
    
    // Properties panel actions
    connect(previewBtn, &QPushButton::clicked, [this]() {
        QMessageBox::information(this, "Preview Design", 
            "👁️ Live Preview\n\n"
            "This will show a live preview of your design in a browser.\n\n"
            "Coming in v0.4.0!");
    });
    
    connect(saveTemplateBtn, &QPushButton::clicked, [this, designComponents]() {
        if (designComponents->isEmpty()) {
            QMessageBox::warning(this, "No Design", "Create a design first before saving as template.");
            return;
        }
        
        QString filename = QFileDialog::getSaveFileName(this, "Save Template", QDir::homePath() + "/my-design-template.json", "JSON Files (*.json)");
        if (!filename.isEmpty()) {
            QFile file(filename);
            if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
                QJsonArray components;
                for (const QString &comp : *designComponents) {
                    components.append(comp);
                }
                QJsonObject templateData;
                templateData["name"] = "My Design Template";
                templateData["version"] = "0.3.0";
                templateData["components"] = components;
                
                QJsonDocument doc(templateData);
                file.write(doc.toJson(QJsonDocument::Indented));
                file.close();
                m_statusLabel->setText("✅ Template saved!");
            }
        }
    });
    
    connect(generateCodeBtn, &QPushButton::clicked, [this, generatedPrompt, frameworkSelector, styleSelector]() {
        if (generatedPrompt->toPlainText().isEmpty()) {
            QMessageBox::warning(this, "No Design", "Create a design first!");
            return;
        }
        
        QString framework = frameworkSelector->currentText();
        QString styling = styleSelector->currentText();
        
        QString message = QString(
            "🚀 Generate Code\n\n"
            "Framework: %1\n"
            "Styling: %2\n\n"
            "This will use the AI prompt and send it to your selected AI model to generate production-ready code!\n\n"
            "Click 'Send to Chat' in the prompt area to get started."
        ).arg(framework, styling);
        
        QMessageBox::information(this, "Generate Code", message);
    });
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
    // Pre-compile stylesheet for faster rendering
    setAttribute(Qt::WA_OpaquePaintEvent);
    setAttribute(Qt::WA_NoSystemBackground);
    
    // Modern dark/light theme
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
                font-size: 24px;
                font-weight: 800;
                letter-spacing: -1px;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #60a5fa, stop:1 #a78bfa);
                -webkit-background-clip: text;
                -webkit-text-fill-color: transparent;
            }

            #version {
                color: rgba(255, 255, 255, 0.6);
                font-size: 10px;
                letter-spacing: 0.5px;
                text-transform: uppercase;
            }

            #newChatBtn {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #2563eb);
                color: white;
                border: none;
                border-radius: 10px;
                padding: 14px;
                font-weight: bold;
                font-size: 14px;
                letter-spacing: 0.3px;
            }

            #newChatBtn:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #2563eb, stop:1 #1d4ed8);
            }

            #newChatBtn:pressed {
                background: #1e40af;
            }

            #sidebarBtn {
                background-color: transparent;
                color: rgba(255, 255, 255, 0.8);
                border: none;
                border-radius: 8px;
                padding: 12px 14px;
                text-align: left;
                font-size: 13px;
                margin: 2px 0px;
            }

            #sidebarBtn:hover {
                background-color: rgba(59, 130, 246, 0.15);
                color: white;
            }

            #sidebarBtn:checked {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 rgba(59, 130, 246, 0.25), stop:1 rgba(37, 99, 235, 0.25));
                color: white;
                font-weight: bold;
                border-left: 3px solid #3b82f6;
            }
        
            #chatDisplay {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #ffffff, stop:1 #f9fafb);
                border: 2px solid #e5e7eb;
                border-radius: 16px;
                padding: 24px;
                font-size: 14px;
                line-height: 1.7;
            }

            #chatInput {
                background-color: white;
                border: 2px solid #e5e7eb;
                border-radius: 16px;
                padding: 16px 24px;
                font-size: 15px;
            }

            #chatInput:focus {
                border-color: #3b82f6;
                outline: none;
                box-shadow: 0 0 0 3px rgba(59, 130, 246, 0.1);
            }

            #sendBtn {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #3b82f6, stop:0.5 #2563eb, stop:1 #1d4ed8);
                color: white;
                border: none;
                border-radius: 16px;
                font-weight: 700;
                font-size: 15px;
                letter-spacing: 0.5px;
                box-shadow: 0 4px 12px rgba(59, 130, 246, 0.3);
            }

            #sendBtn:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #2563eb, stop:0.5 #1d4ed8, stop:1 #1e40af);
                box-shadow: 0 6px 16px rgba(59, 130, 246, 0.4);
                transform: translateY(-1px);
            }

            #sendBtn:pressed {
                background: #1e40af;
                box-shadow: 0 2px 6px rgba(59, 130, 246, 0.3);
                transform: translateY(0px);
            }
        
            #modelsList {
                background-color: white;
                border: 1px solid #e5e7eb;
                border-radius: 12px;
                padding: 8px;
            }

            #modelsList::item {
                padding: 14px 16px;
                border-bottom: none;
                border-radius: 8px;
                margin: 2px 0px;
                color: #374151;
            }

            #modelsList::item:selected {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #eff6ff, stop:1 #dbeafe);
                color: #1e40af;
                font-weight: 500;
            }

            #modelsList::item:hover {
                background-color: #f9fafb;
            }

            #detailsBox {
                background-color: white;
                border: 1px solid #e5e7eb;
                border-radius: 12px;
                padding: 20px;
            }
        
            #downloadBtn, #addBtn, #useBtn, #toolbarBtn {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #2563eb);
                color: white;
                border: none;
                border-radius: 10px;
                padding: 10px 20px;
                font-weight: 600;
                letter-spacing: 0.3px;
            }

            #downloadBtn:hover, #addBtn:hover, #useBtn:hover, #toolbarBtn:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #2563eb, stop:1 #1d4ed8);
            }

            #downloadBtn:pressed, #addBtn:pressed, #useBtn:pressed, #toolbarBtn:pressed {
                background: #1e40af;
            }
            
            /* Visual Designer Styles - Modern Figma-like */
            #designerToolbar {
                background: white;
                border-bottom: 1px solid #e5e7eb;
                padding: 16px 24px;
            }
            
            #blockPalette {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #fafafa, stop:1 #f5f5f5);
                border-right: 1px solid #e5e7eb;
            }
            
            #blockBtn {
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #ffffff, stop:1 #f9fafb);
                border: 2px solid #e5e7eb;
                border-radius: 12px;
                text-align: left;
                padding: 14px 18px;
                font-size: 13px;
                color: #111827;
                font-weight: 600;
            }
            
            #blockBtn:hover {
                border-color: #3b82f6;
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #eff6ff, stop:0.5 #dbeafe, stop:1 #bfdbfe);
                color: #1e40af;
                transform: translateX(4px);
                box-shadow: 0 4px 12px rgba(59, 130, 246, 0.15);
            }
            
            #blockBtn:pressed {
                transform: translateX(2px);
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #dbeafe, stop:1 #bfdbfe);
            }
            
            #designCanvas {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:1,
                    stop:0 #fafafa, stop:0.5 #f5f5f5, stop:1 #fafafa);
            }
        
            #removeBtn {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #ef4444, stop:1 #dc2626);
                color: white;
                border: none;
                border-radius: 10px;
                padding: 10px 20px;
                font-weight: 600;
                letter-spacing: 0.3px;
            }

            #removeBtn:hover {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #dc2626, stop:1 #b91c1c);
            }

            #removeBtn:pressed {
                background: #991b1b;
            }

            #statusLabel {
                color: #10b981;
                font-weight: 600;
                font-size: 13px;
            }

            #countLabel {
                color: #1e40af;
                font-weight: 600;
                padding: 6px 14px;
                background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                    stop:0 #eff6ff, stop:1 #dbeafe);
                border-radius: 8px;
                font-size: 12px;
            }

            QComboBox {
                background-color: white;
                border: 1.5px solid #e5e7eb;
                border-radius: 10px;
                padding: 8px 14px;
                font-size: 13px;
            }

            QComboBox:focus {
                border-color: #3b82f6;
            }

            QLineEdit {
                background-color: white;
                border: 1.5px solid #e5e7eb;
                border-radius: 10px;
                padding: 10px 14px;
                font-size: 13px;
            }

            QLineEdit:focus {
                border-color: #3b82f6;
                outline: none;
            }
        
            QProgressBar {
                border: none;
                border-radius: 8px;
                text-align: center;
                background-color: #e5e7eb;
                height: 8px;
            }

            QProgressBar::chunk {
                background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                    stop:0 #3b82f6, stop:1 #2563eb);
                border-radius: 8px;
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
        
        /* Download Panel - Enhanced */
        #downloadDock {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #f9fafb, stop:1 #ffffff);
        }
        
        #downloadDock::title {
            background-color: #3b82f6;
            color: white;
            padding: 8px;
            font-weight: bold;
        }
        
        #downloadPanel {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #f9fafb, stop:1 #f3f4f6);
        }
        
        #downloadItem {
            background-color: white;
            border: 2px solid #e5e7eb;
            border-radius: 12px;
            padding: 4px;
        }
        
        #downloadItem:hover {
            border-color: #3b82f6;
            box-shadow: 0 4px 6px rgba(59, 130, 246, 0.1);
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
            border: 2px solid #e5e7eb;
            border-radius: 6px;
            background-color: #f3f4f6;
            text-align: center;
            font-weight: bold;
            color: #1f2937;
        }
        
        #downloadProgressBar::chunk {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #3b82f6, stop:0.5 #2563eb, stop:1 #1d4ed8);
            border-radius: 4px;
        }
        
        #cancelBtn {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ef4444, stop:1 #dc2626);
            color: white;
            border: none;
            border-radius: 14px;
            font-weight: bold;
            font-size: 14px;
        }
        
        #cancelBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #dc2626, stop:1 #b91c1c);
        }
        
        #cancelBtn:pressed {
            background-color: #991b1b;
        }
        
          #emptyLabel {
              color: #9ca3af;
              font-style: italic;
          }
          
          /* Properties Panel - Professional Figma-style */
          #propertiesPanel {
              background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                  stop:0 #fafafa, stop:1 #f5f5f5);
              border-left: 1px solid #e5e7eb;
          }
          
          QGroupBox {
              background-color: white;
              border: 1px solid #e5e7eb;
              border-radius: 10px;
              padding: 16px 12px 12px 12px;
              margin-top: 12px;
              font-weight: 600;
              font-size: 12px;
              color: #111827;
          }
          
          QGroupBox::title {
              subcontrol-origin: margin;
              subcontrol-position: top left;
              left: 12px;
              padding: 0 6px;
              background-color: white;
              color: #111827;
          }
      )";
      
      setStyleSheet(styleSheet);
  }

void MainWindow::loadAvailableModels()
{
    // This function is no longer used (we removed model download feature)
    // Only custom models are supported now
    return;
    
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
    
    int totalModels = 0;
    int filteredCount = 0;
    
    // Check if GPU filtering is enabled
    bool filterByGPU = (m_gpuFilterCheckbox && m_gpuFilterCheckbox->isChecked() && m_detectedVRAM > 0);
    
    for (const QJsonValue &value : models) {
        QJsonObject model = value.toObject();
        totalModels++;
        
        // Apply GPU filtering if enabled
        if (filterByGPU) {
            QString sizeStr = model["size"].toString().toUpper();
            
            // Parse size: "3B" -> 3GB, "357B" -> 357GB, "1000B" -> 1TB
            QRegularExpression re("([0-9.]+)\\s*([KMGT]?B)");
            QRegularExpressionMatch match = re.match(sizeStr);
            
            if (match.hasMatch()) {
                double size = match.captured(1).toDouble();
                QString unit = match.captured(2).toUpper();
                
                // Convert to MB
                double sizeInMB = 0;
                if (unit == "KB") {
                    sizeInMB = size / 1024.0;
                } else if (unit == "MB") {
                    sizeInMB = size;
                } else if (unit == "GB" || unit == "B") {
                    // "B" usually means billions of parameters (GB of VRAM)
                    sizeInMB = size * 1024.0;
                } else if (unit == "TB") {
                    sizeInMB = size * 1024.0 * 1024.0;
                }
                
                // For parameter count (e.g., "3B"), estimate VRAM:
                // Q4_K_M quantization: ~0.5-0.6 bytes per parameter
                // So 3B params ≈ 1.8GB, 7B ≈ 4.2GB, 13B ≈ 7.8GB
                if (unit == "B" && size < 10000) { // Likely parameter count, not file size
                    sizeInMB = size * 600; // MB (0.6 bytes per param)
                }
                
                // Skip if model needs more VRAM than available
                qDebug() << "Model:" << model["name"].toString() 
                         << "Size:" << sizeStr << "→" << sizeInMB << "MB"
                         << "GPU:" << m_detectedVRAM << "MB" 
                         << "Skip:" << (sizeInMB > m_detectedVRAM);
                
                if (sizeInMB > m_detectedVRAM) {
                    continue;
                }
            }
        }
        
        filteredCount++;
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
    
    QString countText = QString("%1 models").arg(filteredCount);
    if (filterByGPU && filteredCount < totalModels) {
        countText += QString(" (%1 hidden by GPU filter)").arg(totalModels - filteredCount);
    }
    m_downloadCountLabel->setText(countText);
    qDebug() << "Loaded" << filteredCount << "/" << totalModels << "models (GPU filtering:" << filterByGPU << ")";
}

void MainWindow::loadInstalledModels()
{
    // This function is no longer used (we removed model download feature)
    // Only custom models are supported now
    return;
    
    // Scan ~/Documents/rmm/ for downloaded models
    QString rmmPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation) + "/rmm";
    QDir rmmDir(rmmPath);
    
    if (!rmmDir.exists()) {
        m_installedCountLabel->setText("0 models installed");
        return;
    }
    
    QStringList modelDirs = rmmDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    int installedCount = 0;
    
    for (const QString &dirName : modelDirs) {
        QString modelPath = rmmPath + "/" + dirName + "/model.gguf";
        if (QFile::exists(modelPath)) {
            installedCount++;
        }
    }
    
    m_installedCountLabel->setText(QString("%1 models installed").arg(installedCount));
    qDebug() << "Found" << installedCount << "installed models in" << rmmPath;
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
    
    QString currentModelName = m_modelSelector->currentText();
    QString currentModelPath = m_modelSelector->currentData().toString();
    
    if (currentModelPath.isEmpty()) {
        QMessageBox::warning(this, "No Model Selected", 
            "Please select a model from the dropdown first.");
        return;
    }
    
    // Remove emoji prefixes for model name
    QString cleanModelName = currentModelName;
    cleanModelName.remove(QRegularExpression("^[🔧📦]\\s+"));
    
    appendChatMessage("You", message);
    m_chatInput->clear();
    
    // Start streaming response - prepare assistant message
    m_currentStreamingResponse.clear();
    appendChatMessage("Assistant", ""); // Empty message, will be filled by streaming
    
    m_statusLabel->setText("🧠 Generating...");
    m_statusLabel->setStyleSheet("color: #f59e0b; font-weight: bold;");
    m_sendButton->setEnabled(false);
    
    // Build messages array for chat completion
    QJsonArray messages;
    QJsonObject userMsg;
    userMsg["role"] = "user";
    userMsg["content"] = message;
    messages.append(userMsg);
    
    // Call Python backend for REAL INFERENCE
    if (m_backendClient) {
        m_backendClient->chatCompletion(cleanModelName, messages, 0.7, 512);
    } else {
        // Fallback to old backend (placeholder)
        QJsonObject request;
        request["model"] = currentModelName;
        request["messages"] = messages;
        
        QJsonObject response = m_backend->chatWithHuggingFace(request);
        
        m_statusLabel->setText("✓ Ready");
        m_statusLabel->setStyleSheet("color: #10b981; font-weight: bold;");
        m_sendButton->setEnabled(true);
        
        QString responseText = response["content"].toString();
        if (responseText.isEmpty()) {
            responseText = "⚠️ Backend not available. Make sure Python backend is running.";
        }
        
        // Update the last message
        QString html = m_chatDisplay->toHtml();
        int lastIndex = html.lastIndexOf("<div style='margin-bottom: 20px;");
        if (lastIndex >= 0) {
            QString newHtml = html.left(lastIndex);
            m_chatDisplay->setHtml(newHtml);
        }
        appendChatMessage("Assistant", responseText);
    }
}

void MainWindow::onStreamToken(const QString &token)
{
    // Append token to current streaming response
    m_currentStreamingResponse += token;
    
    // Update the last assistant message in chat display
    QString html = m_chatDisplay->toHtml();
    
    // Find and replace the last assistant message
    int lastAssistantIndex = html.lastIndexOf("<div style='margin-bottom: 20px; padding: 15px; background-color: #f0fdf4;");
    
    if (lastAssistantIndex >= 0) {
        QString beforeLastMessage = html.left(lastAssistantIndex);
        
        QString roleColor = "#10b981";
        QString icon = "🤖";
        
        QString msgHtml = QString(
            "<div style='margin-bottom: 20px; padding: 15px; background-color: #f0fdf4; border-radius: 8px;'>"
            "<b style='color: %1; font-size: 14px;'>%2 Assistant</b><br><br>"
            "<span style='color: #1f2937;'>%3</span>"
            "</div>")
            .arg(roleColor)
            .arg(icon)
            .arg(m_currentStreamingResponse.toHtmlEscaped());
        
        m_chatDisplay->setHtml(beforeLastMessage + msgHtml);
        
        // Auto-scroll to bottom
        QScrollBar *scrollBar = m_chatDisplay->verticalScrollBar();
        scrollBar->setValue(scrollBar->maximum());
    }
}

void MainWindow::onStreamComplete()
{
    m_statusLabel->setText("✓ Ready");
    m_statusLabel->setStyleSheet("color: #10b981; font-weight: bold;");
    m_sendButton->setEnabled(true);
    m_currentStreamingResponse.clear();
    
    qDebug() << "✅ Streaming complete!";
}

void MainWindow::onStreamError(const QString &error)
{
    m_statusLabel->setText("❌ Error");
    m_statusLabel->setStyleSheet("color: #ef4444; font-weight: bold;");
    m_sendButton->setEnabled(true);
    
    qWarning() << "Streaming error:" << error;
    
    // Update last message with error
    QString html = m_chatDisplay->toHtml();
    int lastIndex = html.lastIndexOf("<div style='margin-bottom: 20px;");
    if (lastIndex >= 0) {
        QString newHtml = html.left(lastIndex);
        m_chatDisplay->setHtml(newHtml);
    }
    appendChatMessage("Error", "⚠️ " + error);
}

void MainWindow::onBackendModelsListed(const QJsonArray &models)
{
    qDebug() << "Backend models listed:" << models.size();
    // Could update UI with loaded models status
}

void MainWindow::onBackendModelLoaded(const QString &modelName)
{
    qDebug() << "Backend model loaded:" << modelName;
    m_statusLabel->setText(QString("✅ %1 loaded!").arg(modelName));
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

void MainWindow::onBrowseCustomModelPath()
{
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "Select Model File",
        QDir::homePath(),
        "Model Files (*.gguf *.ggml);;All Files (*)"
    );
    
    if (!filePath.isEmpty()) {
        m_customModelPath->setText(filePath);
    }
}

void MainWindow::onTaskTypeFilterChanged(const QString &filterText)
{
    Q_UNUSED(filterText)
    // TODO: Filter models by task type
    loadAvailableModels();
}

void MainWindow::onSortByChanged(const QString &sortByText)
{
    Q_UNUSED(sortByText)
    // TODO: Sort models
    loadAvailableModels();
}

void MainWindow::onGpuFilterCheckboxToggled(bool checked)
{
    Q_UNUSED(checked)
    // Reload models with GPU filtering
    loadAvailableModels();
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
    
    // Update button states - ONLY 3 pages now
    m_chatPageBtn->setChecked(index == 0);
    m_promptArchitectBtn->setChecked(index == 1);
    m_customPageBtn->setChecked(index == 2);
    
    // Refresh data when switching pages
    // (No longer needed - we removed download/installed pages)
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

void MainWindow::onModelDownloadError(const QString &modelName, const QString &error)
{
    if (m_modelCards.contains(modelName)) {
        m_modelCards[modelName]->setDownloading(false);
    }
    
    // Update download panel
    m_downloadPanel->errorDownload(modelName, error);
    
    // Show error in status
    m_statusLabel->setText(QString("❌ Error: %1").arg(error));
    m_statusLabel->setStyleSheet("color: #ef4444; font-weight: bold;");
    
    qDebug() << "Download error:" << modelName << error;
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
