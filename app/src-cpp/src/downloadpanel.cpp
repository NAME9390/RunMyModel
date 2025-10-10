#include "downloadpanel.h"
#include <QHBoxLayout>
#include <QFont>
#include <QTime>
#include <QTimer>

// DownloadItem Implementation
DownloadItem::DownloadItem(const QString &modelName, QWidget *parent)
    : QFrame(parent)
    , m_modelName(modelName)
{
    setObjectName("downloadItem");
    setMinimumHeight(100);
    
    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(15, 12, 15, 12);
    layout->setSpacing(8);
    
    // Top row: Name and cancel button
    QHBoxLayout *topRow = new QHBoxLayout();
    
    m_nameLabel = new QLabel(modelName);
    m_nameLabel->setObjectName("downloadItemName");
    QFont nameFont = m_nameLabel->font();
    nameFont.setPointSize(12);
    nameFont.setBold(true);
    m_nameLabel->setFont(nameFont);
    topRow->addWidget(m_nameLabel);
    
    topRow->addStretch();
    
    m_cancelBtn = new QPushButton("✕");
    m_cancelBtn->setObjectName("cancelBtn");
    m_cancelBtn->setFixedSize(28, 28);
    m_cancelBtn->setToolTip("Cancel download");
    connect(m_cancelBtn, &QPushButton::clicked, [this]() {
        emit cancelRequested(m_modelName);
    });
    topRow->addWidget(m_cancelBtn);
    
    layout->addLayout(topRow);
    
    // Status label
    m_statusLabel = new QLabel("Starting download...");
    m_statusLabel->setObjectName("downloadStatus");
    layout->addWidget(m_statusLabel);
    
    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setObjectName("downloadProgressBar");
    m_progressBar->setMinimumHeight(20);
    m_progressBar->setTextVisible(true);
    layout->addWidget(m_progressBar);
    
    // Details label (speed, ETA, size)
    m_detailsLabel = new QLabel("Preparing download...");
    m_detailsLabel->setObjectName("downloadDetails");
    layout->addWidget(m_detailsLabel);
}

void DownloadItem::updateProgress(double percent, qint64 downloaded, qint64 total, double speed)
{
    m_progressBar->setValue(static_cast<int>(percent));
    
    m_statusLabel->setText(QString("📥 Downloading... %1%").arg(QString::number(percent, 'f', 1)));
    
    QString details;
    if (total > 0) {
        details = QString("%1 / %2")
            .arg(formatSize(downloaded))
            .arg(formatSize(total));
    } else {
        details = formatSize(downloaded);
    }
    
    if (speed > 0) {
        details += QString("  •  %1/s").arg(formatSpeed(speed));
        
        // Calculate ETA
        if (total > downloaded && speed > 0) {
            int remainingBytes = total - downloaded;
            int eta = remainingBytes / speed;
            details += QString("  •  ETA: %1").arg(formatTime(eta));
        }
    }
    
    m_detailsLabel->setText(details);
}

void DownloadItem::setComplete()
{
    m_progressBar->setValue(100);
    m_statusLabel->setText("✅ Download complete!");
    m_statusLabel->setStyleSheet("color: #10b981; font-weight: bold;");
    m_cancelBtn->setVisible(false);
}

void DownloadItem::setError(const QString &error)
{
    m_statusLabel->setText("❌ Error: " + error);
    m_statusLabel->setStyleSheet("color: #ef4444; font-weight: bold;");
    m_cancelBtn->setText("Remove");
}

QString DownloadItem::formatSize(qint64 bytes)
{
    if (bytes < 1024) return QString::number(bytes) + " B";
    if (bytes < 1024 * 1024) return QString::number(bytes / 1024.0, 'f', 1) + " KB";
    if (bytes < 1024 * 1024 * 1024) return QString::number(bytes / (1024.0 * 1024), 'f', 1) + " MB";
    return QString::number(bytes / (1024.0 * 1024 * 1024), 'f', 2) + " GB";
}

QString DownloadItem::formatSpeed(double bytesPerSec)
{
    if (bytesPerSec < 1024) return QString::number(bytesPerSec, 'f', 0) + " B";
    if (bytesPerSec < 1024 * 1024) return QString::number(bytesPerSec / 1024.0, 'f', 1) + " KB";
    return QString::number(bytesPerSec / (1024.0 * 1024), 'f', 1) + " MB";
}

QString DownloadItem::formatTime(int seconds)
{
    if (seconds < 60) return QString::number(seconds) + "s";
    if (seconds < 3600) return QString::number(seconds / 60) + "m " + QString::number(seconds % 60) + "s";
    return QString::number(seconds / 3600) + "h " + QString::number((seconds % 3600) / 60) + "m";
}

// DownloadPanel Implementation
DownloadPanel::DownloadPanel(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("downloadPanel");
    
    m_layout = new QVBoxLayout(this);
    m_layout->setContentsMargins(20, 20, 20, 20);
    m_layout->setSpacing(15);
    
    // Title
    m_titleLabel = new QLabel("📥 Active Downloads");
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(16);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_layout->addWidget(m_titleLabel);
    
    // Empty state label
    m_emptyLabel = new QLabel("No active downloads");
    m_emptyLabel->setObjectName("emptyLabel");
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setMinimumHeight(100);
    m_layout->addWidget(m_emptyLabel);
    
    m_layout->addStretch();
}

void DownloadPanel::addDownload(const QString &modelName)
{
    if (m_downloads.contains(modelName)) {
        return; // Already exists
    }
    
    DownloadItem *item = new DownloadItem(modelName, this);
    connect(item, &DownloadItem::cancelRequested, this, &DownloadPanel::cancelDownloadRequested);
    
    m_layout->insertWidget(m_layout->count() - 1, item); // Insert before stretch
    m_downloads[modelName] = item;
    
    updateEmptyState();
    
    // Update title with count
    m_titleLabel->setText(QString("📥 Active Downloads (%1)").arg(m_downloads.size()));
}

void DownloadPanel::updateDownload(const QString &modelName, double percent, qint64 downloaded, qint64 total, double speed)
{
    if (m_downloads.contains(modelName)) {
        m_downloads[modelName]->updateProgress(percent, downloaded, total, speed);
    }
}

void DownloadPanel::completeDownload(const QString &modelName)
{
    if (m_downloads.contains(modelName)) {
        m_downloads[modelName]->setComplete();
        
        // Auto-remove after 3 seconds
        QTimer::singleShot(3000, [this, modelName]() {
            removeDownload(modelName);
        });
    }
}

void DownloadPanel::errorDownload(const QString &modelName, const QString &error)
{
    if (m_downloads.contains(modelName)) {
        m_downloads[modelName]->setError(error);
    }
}

void DownloadPanel::removeDownload(const QString &modelName)
{
    if (m_downloads.contains(modelName)) {
        DownloadItem *item = m_downloads[modelName];
        m_layout->removeWidget(item);
        item->deleteLater();
        m_downloads.remove(modelName);
        
        updateEmptyState();
        m_titleLabel->setText(QString("📥 Active Downloads (%1)").arg(m_downloads.size()));
        
        if (m_downloads.isEmpty()) {
            emit allDownloadsComplete();
        }
    }
}

bool DownloadPanel::hasActiveDownloads() const
{
    return !m_downloads.isEmpty();
}

int DownloadPanel::activeDownloadCount() const
{
    return m_downloads.size();
}

void DownloadPanel::updateEmptyState()
{
    m_emptyLabel->setVisible(m_downloads.isEmpty());
}

