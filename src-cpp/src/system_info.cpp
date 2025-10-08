#include "system_info.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QSysInfo>
#include <QStorageInfo>
#include <QDir>
#include <QThread>

SystemInfo::SystemInfo(QObject *parent)
    : QObject(parent)
{
}

SystemInfo::~SystemInfo() = default;

QJsonObject SystemInfo::getSystemInfo()
{
    QJsonObject systemInfo;
    
    // Basic system info
    systemInfo["os"] = QSysInfo::prettyProductName();
    systemInfo["arch"] = QSysInfo::currentCpuArchitecture();
    systemInfo["kernel"] = QSysInfo::kernelType() + " " + QSysInfo::kernelVersion();
    
    // Hardware info
    systemInfo["cpu"] = getCpuInfo();
    systemInfo["gpu"] = getGpuInfo();
    systemInfo["memory"] = getTotalMemory();
    
    // Hugging Face info
    systemInfo["huggingface"] = getHuggingFaceInfo();
    
    return systemInfo;
}

QJsonObject SystemInfo::getGpuInfo()
{
    GpuInfo gpu = detectGpu();
    
    QJsonObject gpuInfo;
    gpuInfo["available"] = gpu.available;
    gpuInfo["name"] = gpu.name;
    gpuInfo["memory"] = gpu.memory;
    gpuInfo["driver"] = gpu.driver;
    
    return gpuInfo;
}

QJsonObject SystemInfo::getCpuInfo()
{
    CpuInfo cpu = detectCpu();
    
    QJsonObject cpuInfo;
    cpuInfo["cores"] = cpu.cores;
    cpuInfo["name"] = cpu.name;
    cpuInfo["memory"] = cpu.memory;
    
    return cpuInfo;
}

QJsonObject SystemInfo::getHuggingFaceInfo()
{
    HuggingFaceInfo hf = detectHuggingFace();
    
    QJsonObject hfInfo;
    hfInfo["available"] = hf.available;
    hfInfo["cache_dir"] = hf.cacheDir;
    
    QJsonArray modelsArray;
    for (const QString &model : hf.modelsDownloaded) {
        modelsArray.append(model);
    }
    hfInfo["models_downloaded"] = modelsArray;
    
    return hfInfo;
}

SystemInfo::GpuInfo SystemInfo::detectGpu()
{
    GpuInfo gpu;
    gpu.available = false;
    
    // Try to detect NVIDIA GPU
    QString nvidiaInfo = executeCommand("nvidia-smi", QStringList() << "--query-gpu=name,memory.total" << "--format=csv,noheader,nounits");
    if (!nvidiaInfo.isEmpty()) {
        QStringList lines = nvidiaInfo.split('\n', Qt::SkipEmptyParts);
        if (!lines.isEmpty()) {
            QStringList parts = lines[0].split(',');
            if (parts.size() >= 2) {
                gpu.available = true;
                gpu.name = parts[0].trimmed();
                gpu.memory = parts[1].trimmed().toLongLong() * 1024 * 1024; // Convert MB to bytes
                gpu.driver = "NVIDIA";
            }
        }
    }
    
    // Try to detect AMD GPU
    if (!gpu.available) {
        QString amdInfo = executeCommand("lspci", QStringList() << "-nn" << "|" << "grep" << "-i" << "vga");
        if (amdInfo.contains("AMD", Qt::CaseInsensitive) || amdInfo.contains("Radeon", Qt::CaseInsensitive)) {
            gpu.available = true;
            gpu.name = "AMD GPU";
            gpu.memory = 0; // Would need additional commands to get memory
            gpu.driver = "AMD";
        }
    }
    
    // Try to detect Intel GPU
    if (!gpu.available) {
        QString intelInfo = executeCommand("lspci", QStringList() << "-nn" << "|" << "grep" << "-i" << "vga");
        if (intelInfo.contains("Intel", Qt::CaseInsensitive)) {
            gpu.available = true;
            gpu.name = "Intel GPU";
            gpu.memory = 0; // Would need additional commands to get memory
            gpu.driver = "Intel";
        }
    }
    
    return gpu;
}

SystemInfo::CpuInfo SystemInfo::detectCpu()
{
    CpuInfo cpu;
    cpu.cores = getCpuCores();
    cpu.name = getCpuName();
    cpu.memory = getTotalMemory();
    
    return cpu;
}

SystemInfo::HuggingFaceInfo SystemInfo::detectHuggingFace()
{
    HuggingFaceInfo hf;
    hf.available = true; // Assume available since we're implementing it
    hf.cacheDir = getHuggingFaceCacheDir();
    hf.modelsDownloaded = getDownloadedModels();
    
    return hf;
}

QString SystemInfo::executeCommand(const QString &command, const QStringList &arguments)
{
    QProcess process;
    process.start(command, arguments);
    process.waitForFinished(5000); // 5 second timeout
    
    if (process.exitCode() == 0) {
        return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
    }
    
    return QString();
}

QString SystemInfo::getGpuName()
{
    // Try NVIDIA first
    QString nvidiaName = executeCommand("nvidia-smi", QStringList() << "--query-gpu=name" << "--format=csv,noheader");
    if (!nvidiaName.isEmpty()) {
        return nvidiaName.trimmed();
    }
    
    // Try AMD
    QString amdName = executeCommand("lspci", QStringList() << "-nn" << "|" << "grep" << "-i" << "vga");
    if (amdName.contains("AMD", Qt::CaseInsensitive)) {
        return "AMD GPU";
    }
    
    // Try Intel
    if (amdName.contains("Intel", Qt::CaseInsensitive)) {
        return "Intel GPU";
    }
    
    return "Unknown GPU";
}

qint64 SystemInfo::getTotalMemory()
{
    QFile file("/proc/meminfo");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("MemTotal:")) {
                QStringList parts = line.split(' ', Qt::SkipEmptyParts);
                if (parts.size() >= 2) {
                    return parts[1].toLongLong() * 1024; // Convert KB to bytes
                }
            }
        }
    }
    
    return 0;
}

int SystemInfo::getCpuCores()
{
    return QThread::idealThreadCount();
}

QString SystemInfo::getCpuName()
{
    QFile file("/proc/cpuinfo");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        while (!in.atEnd()) {
            QString line = in.readLine();
            if (line.startsWith("model name")) {
                QStringList parts = line.split(':');
                if (parts.size() >= 2) {
                    return parts[1].trimmed();
                }
            }
        }
    }
    
    return "Unknown CPU";
}

QString SystemInfo::getHuggingFaceCacheDir()
{
    QString homeDir = QDir::homePath();
    return homeDir + "/.cache/huggingface/hub";
}

QStringList SystemInfo::getDownloadedModels()
{
    QStringList models;
    QString cacheDir = getHuggingFaceCacheDir();
    
    QDir dir(cacheDir);
    if (dir.exists()) {
        QStringList entries = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        for (const QString &entry : entries) {
            if (entry.startsWith("models--")) {
                QString modelName = entry.mid(8); // Remove "models--" prefix
                modelName.replace("--", "/");
                models.append(modelName);
            }
        }
    }
    
    return models;
}
