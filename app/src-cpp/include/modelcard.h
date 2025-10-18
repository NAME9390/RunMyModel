#ifndef MODELCARD_H
#define MODELCARD_H

#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QProgressBar>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonObject>
#include <QFrame>

class ModelCard : public QFrame
{
    Q_OBJECT

public:
    explicit ModelCard(const QJsonObject &model, QWidget *parent = nullptr);
    
    QString getModelName() const { return m_modelName; }
    void setDownloadProgress(int progress);
    void setDownloading(bool downloading);
    void setInstalled(bool installed);

signals:
    void downloadRequested(const QString &modelName);
    void removeRequested(const QString &modelName);
    void useRequested(const QString &modelName);

private:
    QString m_modelName;
    QLabel *m_nameLabel;
    QLabel *m_sizeLabel;
    QLabel *m_taskLabel;
    QLabel *m_ratingLabel;
    QLabel *m_statusLabel;
    QPushButton *m_actionButton;
    QProgressBar *m_progressBar;
    
    bool m_isInstalled;
    bool m_isDownloading;
    
    void setupUI(const QJsonObject &model);
    void updateButtonState();
};

#endif // MODELCARD_H

