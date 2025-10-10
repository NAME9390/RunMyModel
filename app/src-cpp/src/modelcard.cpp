#include "modelcard.h"
#include <QFont>

ModelCard::ModelCard(const QJsonObject &model, QWidget *parent)
    : QFrame(parent)
    , m_isInstalled(false)
    , m_isDownloading(false)
{
    m_modelName = model["name"].toString();
    setupUI(model);
}

void ModelCard::setupUI(const QJsonObject &model)
{
    setObjectName("modelCard");
    setMinimumHeight(140);
    setMaximumHeight(140);
    
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(15, 12, 15, 12);
    mainLayout->setSpacing(8);
    
    // Top row: Name and status
    QHBoxLayout *topRow = new QHBoxLayout();
    
    m_nameLabel = new QLabel(model["name"].toString());
    m_nameLabel->setObjectName("modelCardName");
    QFont nameFont = m_nameLabel->font();
    nameFont.setPointSize(13);
    nameFont.setBold(true);
    m_nameLabel->setFont(nameFont);
    m_nameLabel->setWordWrap(false);
    m_nameLabel->setMaximumWidth(400);
    topRow->addWidget(m_nameLabel);
    
    topRow->addStretch();
    
    m_statusLabel = new QLabel("Available");
    m_statusLabel->setObjectName("statusBadge");
    topRow->addWidget(m_statusLabel);
    
    mainLayout->addLayout(topRow);
    
    // Middle row: Stats
    QHBoxLayout *statsRow = new QHBoxLayout();
    
    QString size = model["size"].toString();
    m_sizeLabel = new QLabel(QString("📊 %1").arg(size));
    m_sizeLabel->setObjectName("modelStat");
    statsRow->addWidget(m_sizeLabel);
    
    QString task = model["task_type"].toString();
    m_taskLabel = new QLabel(QString("🎯 %1").arg(task));
    m_taskLabel->setObjectName("modelStat");
    statsRow->addWidget(m_taskLabel);
    
    QString rating = model["rating"].toString();
    if (rating != "N/A") {
        m_ratingLabel = new QLabel(QString("⭐ %1").arg(rating));
        m_ratingLabel->setObjectName("modelStat");
        statsRow->addWidget(m_ratingLabel);
    }
    
    statsRow->addStretch();
    
    mainLayout->addLayout(statsRow);
    
    // Progress bar (hidden by default)
    m_progressBar = new QProgressBar();
    m_progressBar->setObjectName("modelProgress");
    m_progressBar->setMinimumHeight(6);
    m_progressBar->setMaximumHeight(6);
    m_progressBar->setTextVisible(false);
    m_progressBar->setVisible(false);
    mainLayout->addWidget(m_progressBar);
    
    // Bottom row: Action button
    QHBoxLayout *bottomRow = new QHBoxLayout();
    bottomRow->addStretch();
    
    m_actionButton = new QPushButton("⬇️ Download");
    m_actionButton->setObjectName("modelCardButton");
    m_actionButton->setMinimumHeight(32);
    m_actionButton->setMinimumWidth(120);
    
    connect(m_actionButton, &QPushButton::clicked, [this]() {
        if (m_isInstalled) {
            emit useRequested(m_modelName);
        } else if (!m_isDownloading) {
            emit downloadRequested(m_modelName);
        }
    });
    
    bottomRow->addWidget(m_actionButton);
    
    mainLayout->addLayout(bottomRow);
    
    updateButtonState();
}

void ModelCard::setDownloadProgress(int progress)
{
    m_progressBar->setValue(progress);
    
    if (progress >= 100) {
        setDownloading(false);
        setInstalled(true);
    }
}

void ModelCard::setDownloading(bool downloading)
{
    m_isDownloading = downloading;
    m_progressBar->setVisible(downloading);
    
    if (downloading) {
        m_statusLabel->setText("Downloading...");
        m_statusLabel->setStyleSheet("background-color: #f59e0b; color: white;");
    }
    
    updateButtonState();
}

void ModelCard::setInstalled(bool installed)
{
    m_isInstalled = installed;
    
    if (installed) {
        m_statusLabel->setText("✓ Installed");
        m_statusLabel->setStyleSheet("background-color: #10b981; color: white;");
    } else {
        m_statusLabel->setText("Available");
        m_statusLabel->setStyleSheet("background-color: #6b7280; color: white;");
    }
    
    updateButtonState();
}

void ModelCard::updateButtonState()
{
    if (m_isDownloading) {
        m_actionButton->setText("Downloading...");
        m_actionButton->setEnabled(false);
    } else if (m_isInstalled) {
        m_actionButton->setText("✓ Use Model");
        m_actionButton->setEnabled(true);
    } else {
        m_actionButton->setText("⬇️ Download");
        m_actionButton->setEnabled(true);
    }
}

