#include "prompt_architect_widget.h"
#include "backend_client.h"
#include <QScrollArea>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QJsonDocument>
#include <QDebug>

// ==================== PromptBlockWidget ====================

PromptBlockWidget::PromptBlockWidget(const QString &blockType, const QString &blockId, QWidget *parent)
    : QGroupBox(parent)
    , m_blockId(blockId)
    , m_blockType(blockType)
{
    setupUI();
}

void PromptBlockWidget::setupUI()
{
    // Icon and title based on type
    QString icon;
    QString title;
    
    if (m_blockType == "system") {
        icon = "⚙️";
        title = "System Prompt";
    } else if (m_blockType == "user") {
        icon = "👤";
        title = "User Message";
    } else if (m_blockType == "context") {
        icon = "📋";
        title = "Context";
    } else if (m_blockType == "instruction") {
        icon = "📝";
        title = "Instructions";
    } else if (m_blockType == "output_format") {
        icon = "📤";
        title = "Output Format";
    } else {
        icon = "📄";
        title = "Custom Block";
    }
    
    setTitle(icon + " " + title);
    setObjectName("promptBlock");
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    
    // Content editor
    m_contentEdit = new QTextEdit();
    m_contentEdit->setPlaceholderText("Enter " + title.toLower() + " content here...\nUse {variable_name} for variables.");
    m_contentEdit->setMinimumHeight(80);
    layout->addWidget(m_contentEdit);
    
    // Actions
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    
    m_removeBtn = new QPushButton("🗑️ Remove Block");
    m_removeBtn->setObjectName("removeBlockBtn");
    actionsLayout->addStretch();
    actionsLayout->addWidget(m_removeBtn);
    
    layout->addLayout(actionsLayout);
    
    // Connect signals
    connect(m_contentEdit, &QTextEdit::textChanged, [this]() {
        emit contentChanged(m_blockId);
    });
    
    connect(m_removeBtn, &QPushButton::clicked, [this]() {
        emit removeRequested(m_blockId);
    });
}

QString PromptBlockWidget::content() const
{
    return m_contentEdit->toPlainText();
}

void PromptBlockWidget::setContent(const QString &content)
{
    m_contentEdit->setPlainText(content);
}

// ==================== PromptArchitectWidget ====================

PromptArchitectWidget::PromptArchitectWidget(BackendClient *backendClient, QWidget *parent)
    : QWidget(parent)
    , m_backendClient(backendClient)
{
    setupUI();
    applyModernStyling();
    loadTemplates();
}

PromptArchitectWidget::~PromptArchitectWidget()
{
}

void PromptArchitectWidget::setupUI()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(15);
    
    // Header
    QLabel *titleLabel = new QLabel("🧠 Prompt Architect");
    QFont titleFont = titleLabel->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    titleLabel->setFont(titleFont);
    mainLayout->addWidget(titleLabel);
    
    QLabel *subtitleLabel = new QLabel("Build your prompt visually with blocks and templates");
    subtitleLabel->setStyleSheet("color: #6b7280;");
    mainLayout->addWidget(subtitleLabel);
    
    // Template selector
    QHBoxLayout *templateLayout = new QHBoxLayout();
    
    QLabel *templateLabel = new QLabel("📋 Start from template:");
    templateLayout->addWidget(templateLabel);
    
    m_templateCombo = new QComboBox();
    m_templateCombo->setMinimumWidth(300);
    templateLayout->addWidget(m_templateCombo);
    
    m_loadTemplateBtn = new QPushButton("Load Template");
    m_loadTemplateBtn->setObjectName("loadTemplateBtn");
    templateLayout->addWidget(m_loadTemplateBtn);
    
    templateLayout->addStretch();
    mainLayout->addLayout(templateLayout);
    
    // Main content area (splitter-style)
    QHBoxLayout *contentLayout = new QHBoxLayout();
    
    // Left: Blocks builder
    QGroupBox *builderGroup = new QGroupBox("Build Prompt");
    builderGroup->setObjectName("builderGroup");
    QVBoxLayout *builderLayout = new QVBoxLayout(builderGroup);
    
    // Add block controls
    QHBoxLayout *addBlockLayout = new QHBoxLayout();
    
    m_blockTypeCombo = new QComboBox();
    m_blockTypeCombo->addItems({"System Prompt", "User Message", "Context", "Instructions", "Output Format"});
    addBlockLayout->addWidget(m_blockTypeCombo);
    
    m_addBlockBtn = new QPushButton("➕ Add Block");
    m_addBlockBtn->setObjectName("addBlockBtn");
    addBlockLayout->addWidget(m_addBlockBtn);
    
    builderLayout->addLayout(addBlockLayout);
    
    // Blocks container (scrollable)
    QScrollArea *blocksScroll = new QScrollArea();
    blocksScroll->setWidgetResizable(true);
    blocksScroll->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    
    m_blocksContainer = new QWidget();
    m_blocksLayout = new QVBoxLayout(m_blocksContainer);
    m_blocksLayout->setSpacing(10);
    m_blocksLayout->addStretch();
    
    blocksScroll->setWidget(m_blocksContainer);
    builderLayout->addWidget(blocksScroll);
    
    contentLayout->addWidget(builderGroup, 3);
    
    // Right: Preview and variables
    QVBoxLayout *rightLayout = new QVBoxLayout();
    
    // Variables
    QGroupBox *variablesGroup = new QGroupBox("Variables");
    variablesGroup->setObjectName("variablesGroup");
    QVBoxLayout *variablesGroupLayout = new QVBoxLayout(variablesGroup);
    
    QScrollArea *variablesScroll = new QScrollArea();
    variablesScroll->setWidgetResizable(true);
    variablesScroll->setMaximumHeight(200);
    
    m_variablesContainer = new QWidget();
    m_variablesLayout = new QVBoxLayout(m_variablesContainer);
    m_variablesLayout->addStretch();
    
    variablesScroll->setWidget(m_variablesContainer);
    variablesGroupLayout->addWidget(variablesScroll);
    
    rightLayout->addWidget(variablesGroup);
    
    // Preview
    QGroupBox *previewGroup = new QGroupBox("Preview");
    previewGroup->setObjectName("previewGroup");
    QVBoxLayout *previewLayout = new QVBoxLayout(previewGroup);
    
    m_previewText = new QTextEdit();
    m_previewText->setReadOnly(true);
    m_previewText->setPlaceholderText("Your prompt will appear here...");
    previewLayout->addWidget(m_previewText);
    
    rightLayout->addWidget(previewGroup);
    
    contentLayout->addLayout(rightLayout, 2);
    
    mainLayout->addLayout(contentLayout);
    
    // Actions
    QHBoxLayout *actionsLayout = new QHBoxLayout();
    
    m_saveBtn = new QPushButton("💾 Save Prompt");
    m_saveBtn->setObjectName("actionBtn");
    actionsLayout->addWidget(m_saveBtn);
    
    m_loadBtn = new QPushButton("📂 Load Prompt");
    m_loadBtn->setObjectName("actionBtn");
    actionsLayout->addWidget(m_loadBtn);
    
    actionsLayout->addStretch();
    
    m_useInChatBtn = new QPushButton("💬 Use in Chat");
    m_useInChatBtn->setObjectName("primaryBtn");
    actionsLayout->addWidget(m_useInChatBtn);
    
    mainLayout->addLayout(actionsLayout);
    
    // Connect signals
    connect(m_templateCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &PromptArchitectWidget::onTemplateSelected);
    connect(m_loadTemplateBtn, &QPushButton::clicked, this, &PromptArchitectWidget::onTemplateSelected);
    connect(m_addBlockBtn, &QPushButton::clicked, this, &PromptArchitectWidget::onAddBlockClicked);
    connect(m_saveBtn, &QPushButton::clicked, this, &PromptArchitectWidget::onSavePromptClicked);
    connect(m_loadBtn, &QPushButton::clicked, this, &PromptArchitectWidget::onLoadPromptClicked);
    connect(m_useInChatBtn, &QPushButton::clicked, this, &PromptArchitectWidget::onBuildPromptClicked);
}

void PromptArchitectWidget::applyModernStyling()
{
    QString styleSheet = R"(
        #promptBlock {
            background: qlineargradient(x1:0, y1:0, x2:0, y2:1,
                stop:0 #ffffff, stop:1 #f8f9fa);
            border: 2px solid #e0e0e0;
            border-radius: 8px;
            padding: 10px;
            margin: 5px;
        }
        
        #promptBlock:hover {
            border-color: #667eea;
        }
        
        #builderGroup, #previewGroup, #variablesGroup {
            background-color: white;
            border: 1px solid #e0e0e0;
            border-radius: 8px;
            padding: 15px;
            font-weight: bold;
        }
        
        #addBlockBtn {
            background-color: #10b981;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
        }
        
        #addBlockBtn:hover {
            background-color: #059669;
        }
        
        #removeBlockBtn {
            background-color: #ef4444;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 6px 12px;
            font-size: 11px;
        }
        
        #removeBlockBtn:hover {
            background-color: #dc2626;
        }
        
        #loadTemplateBtn, #actionBtn {
            background-color: #3b82f6;
            color: white;
            border: none;
            border-radius: 6px;
            padding: 8px 16px;
            font-weight: bold;
        }
        
        #loadTemplateBtn:hover, #actionBtn:hover {
            background-color: #2563eb;
        }
        
        #primaryBtn {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #667eea, stop:1 #764ba2);
            color: white;
            border: none;
            border-radius: 6px;
            padding: 10px 24px;
            font-weight: bold;
            font-size: 14px;
        }
        
        #primaryBtn:hover {
            background: qlineargradient(x1:0, y1:0, x2:1, y2:0,
                stop:0 #5568d3, stop:1 #653a8b);
        }
    )";
    
    setStyleSheet(styleSheet);
}

void PromptArchitectWidget::loadTemplates()
{
    // TODO: Call backend to get templates
    // For now, add placeholder
    m_templateCombo->addItem("-- Select a template --", "");
    m_templateCombo->addItem("🤖 Coding Assistant", "coding_assistant");
    m_templateCombo->addItem("✍️ Writing Assistant", "writing_assistant");
    m_templateCombo->addItem("📊 Data Analyst", "data_analysis");
    m_templateCombo->addItem("🐛 Debug Helper", "debugging");
    m_templateCombo->addItem("📚 Documentation Writer", "documentation");
    m_templateCombo->addItem("💬 General Assistant", "general_assistant");
}

void PromptArchitectWidget::onTemplateSelected(int index)
{
    Q_UNUSED(index)
    QString templateId = m_templateCombo->currentData().toString();
    if (!templateId.isEmpty()) {
        emit templateSelected(templateId);
        // TODO: Load template blocks from backend
    }
}

void PromptArchitectWidget::onAddBlockClicked()
{
    QString blockType;
    int selectedIndex = m_blockTypeCombo->currentIndex();
    
    switch (selectedIndex) {
        case 0: blockType = "system"; break;
        case 1: blockType = "user"; break;
        case 2: blockType = "context"; break;
        case 3: blockType = "instruction"; break;
        case 4: blockType = "output_format"; break;
        default: blockType = "text";
    }
    
    createNewBlock(blockType);
}

void PromptArchitectWidget::createNewBlock(const QString &blockType)
{
    QString blockId = QString("block_%1").arg(m_blockWidgets.size());
    
    PromptBlockWidget *blockWidget = new PromptBlockWidget(blockType, blockId, this);
    
    connect(blockWidget, &PromptBlockWidget::contentChanged, this, &PromptArchitectWidget::onBlockContentChanged);
    connect(blockWidget, &PromptBlockWidget::removeRequested, this, &PromptArchitectWidget::onRemoveBlockClicked);
    
    // Insert before the stretch
    int insertIndex = m_blocksLayout->count() - 1;
    m_blocksLayout->insertWidget(insertIndex, blockWidget);
    
    m_blockWidgets.append(blockWidget);
    
    updatePreview();
}

void PromptArchitectWidget::onRemoveBlockClicked()
{
    // Get sender block
    PromptBlockWidget *senderBlock = qobject_cast<PromptBlockWidget*>(sender());
    if (!senderBlock) return;
    
    m_blockWidgets.removeAll(senderBlock);
    senderBlock->deleteLater();
    
    updatePreview();
}

void PromptArchitectWidget::onBlockContentChanged()
{
    updatePreview();
}

void PromptArchitectWidget::onVariableChanged()
{
    updatePreview();
}

void PromptArchitectWidget::updatePreview()
{
    QStringList promptParts;
    
    for (PromptBlockWidget *block : m_blockWidgets) {
        QString content = block->content();
        if (!content.trimmed().isEmpty()) {
            QString blockType = block->blockType();
            
            if (blockType == "system") {
                promptParts << "System: " + content;
            } else if (blockType == "user") {
                promptParts << "User: " + content;
            } else if (blockType == "context") {
                promptParts << "Context: " + content;
            } else if (blockType == "instruction") {
                promptParts << "Instruction: " + content;
            } else if (blockType == "output_format") {
                promptParts << "Output Format: " + content;
            } else {
                promptParts << content;
            }
        }
    }
    
    QString finalPrompt = promptParts.join("\n\n");
    m_previewText->setPlainText(finalPrompt);
}

void PromptArchitectWidget::onBuildPromptClicked()
{
    QString prompt = m_previewText->toPlainText();
    if (!prompt.isEmpty()) {
        emit promptBuilt(prompt);
        QMessageBox::information(this, "Prompt Ready",
            "✅ Prompt has been sent to chat!\n\nYou can now use it with your selected model.");
    }
}

void PromptArchitectWidget::onSavePromptClicked()
{
    QString filename = QFileDialog::getSaveFileName(
        this,
        "Save Prompt",
        QDir::homePath() + "/Documents/rmm/prompts/my_prompt.json",
        "JSON Files (*.json)"
    );
    
    if (!filename.isEmpty()) {
        // TODO: Save to file
        QMessageBox::information(this, "Saved", "Prompt saved to: " + filename);
    }
}

void PromptArchitectWidget::onLoadPromptClicked()
{
    QString filename = QFileDialog::getOpenFileName(
        this,
        "Load Prompt",
        QDir::homePath() + "/Documents/rmm/prompts/",
        "JSON Files (*.json)"
    );
    
    if (!filename.isEmpty()) {
        // TODO: Load from file
        QMessageBox::information(this, "Loaded", "Prompt loaded from: " + filename);
    }
}

void PromptArchitectWidget::onTemplatesLoaded(const QJsonArray &templates)
{
    m_availableTemplates = templates;
    // TODO: Populate combo box from actual backend data
}

