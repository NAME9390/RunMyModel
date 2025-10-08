#ifndef SYSTEM_INFO_H
#define SYSTEM_INFO_H

#include <QObject>
#include <QString>
#include <QJsonObject>
#include <QProcess>

class SystemInfo : public QObject
{
    Q_OBJECT

public:
    explicit SystemInfo(QObject *parent = nullptr);
    ~SystemInfo();

    QJsonObject getSystemInfo();
    QJsonObject getGpuInfo();
    QJsonObject getCpuInfo();
    QJsonObject getHuggingFaceInfo();

private:
    struct GpuInfo {
        bool available;
        QString name;
        qint64 memory;
        QString driver;
    };

    struct CpuInfo {
        int cores;
        QString name;
        qint64 memory;
    };

    struct HuggingFaceInfo {
        bool available;
        QString cacheDir;
        QStringList modelsDownloaded;
    };

    GpuInfo detectGpu();
    CpuInfo detectCpu();
    HuggingFaceInfo detectHuggingFace();
    QString executeCommand(const QString &command, const QStringList &arguments = QStringList());
    QString getGpuName();
    qint64 getTotalMemory();
    int getCpuCores();
    QString getCpuName();
    QString getHuggingFaceCacheDir();
    QStringList getDownloadedModels();
};

#endif // SYSTEM_INFO_H
