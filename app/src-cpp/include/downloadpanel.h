#ifndef DOWNLOADPANEL_H
#define DOWNLOADPANEL_H

#include <QWidget>
#include <QLabel>
#include <QProgressBar>
#include <QPushButton>
#include <QVBoxLayout>
#include <QFrame>
#include <QMap>

class DownloadItem : public QFrame
{
    Q_OBJECT

public:
    explicit DownloadItem(const QString &modelName, QWidget *parent = nullptr);
    
    void updateProgress(double percent, qint64 downloaded, qint64 total, double speed);
    void setComplete();
    void setError(const QString &error);
    QString getModelName() const { return m_modelName; }

signals:
    void cancelRequested(const QString &modelName);

private:
    QString m_modelName;
    QLabel *m_nameLabel;
    QLabel *m_statusLabel;
    QProgressBar *m_progressBar;
    QLabel *m_detailsLabel;
    QPushButton *m_cancelBtn;
    
    QString formatSize(qint64 bytes);
    QString formatSpeed(double bytesPerSec);
    QString formatTime(int seconds);
};

class DownloadPanel : public QWidget
{
    Q_OBJECT

public:
    explicit DownloadPanel(QWidget *parent = nullptr);
    
    void addDownload(const QString &modelName);
    void updateDownload(const QString &modelName, double percent, qint64 downloaded, qint64 total, double speed);
    void completeDownload(const QString &modelName);
    void errorDownload(const QString &modelName, const QString &error);
    void removeDownload(const QString &modelName);
    
    bool hasActiveDownloads() const;
    int activeDownloadCount() const;

signals:
    void cancelDownloadRequested(const QString &modelName);
    void allDownloadsComplete();

private:
    QVBoxLayout *m_layout;
    QLabel *m_titleLabel;
    QLabel *m_emptyLabel;
    QMap<QString, DownloadItem*> m_downloads;
    
    void updateEmptyState();
};

#endif // DOWNLOADPANEL_H

