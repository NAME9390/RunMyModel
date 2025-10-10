#include "gpu_detector.h"
#include <QDebug>
#include <QFile>
#include <QRegularExpression>

GPUDetector::GPUDetector(QObject *parent)
    : QObject(parent)
{
}

GPUDetector::GPUInfo GPUDetector::detectGPU()
{
    GPUInfo info;
    info.detected = false;
    info.vramMB = 0;
    
    // Try NVIDIA first
    qint64 vram = detectNvidiaVRAM();
    if (vram > 0) {
        info.name = "NVIDIA GPU";
        info.vramMB = vram;
        info.detected = true;
        qDebug() << "✅ Detected NVIDIA GPU with" << vram << "MB VRAM";
        return info;
    }
    
    // Try AMD
    vram = detectAMDVRAM();
    if (vram > 0) {
        info.name = "AMD GPU";
        info.vramMB = vram;
        info.detected = true;
        qDebug() << "✅ Detected AMD GPU with" << vram << "MB VRAM";
        return info;
    }
    
    // Try Intel
    vram = detectIntelVRAM();
    if (vram > 0) {
        info.name = "Intel GPU";
        info.vramMB = vram;
        info.detected = true;
        qDebug() << "✅ Detected Intel GPU with" << vram << "MB VRAM";
        return info;
    }
    
    qDebug() << "⚠️ No GPU detected, showing all models";
    return info;
}

qint64 GPUDetector::getVRAM()
{
    return detectGPU().vramMB;
}

qint64 GPUDetector::detectNvidiaVRAM()
{
    // Try nvidia-smi
    QProcess process;
    process.start("nvidia-smi", QStringList() << "--query-gpu=memory.total" << "--format=csv,noheader,nounits");
    
    if (!process.waitForFinished(3000)) {
        return 0;
    }
    
    QString output = process.readAllStandardOutput().trimmed();
    bool ok;
    qint64 vram = output.toLongLong(&ok);
    
    return ok ? vram : 0;
}

qint64 GPUDetector::detectAMDVRAM()
{
    // Try rocm-smi
    QProcess process;
    process.start("rocm-smi", QStringList() << "--showmeminfo" << "vram");
    
    if (!process.waitForFinished(3000)) {
        return 0;
    }
    
    QString output = process.readAllStandardOutput();
    
    // Parse output for VRAM size
    QRegularExpression re("(\\d+)\\s*MB");
    QRegularExpressionMatch match = re.match(output);
    
    if (match.hasMatch()) {
        return match.captured(1).toLongLong();
    }
    
    // Try reading from sysfs
    QFile file("/sys/class/drm/card0/device/mem_info_vram_total");
    if (file.open(QIODevice::ReadOnly)) {
        QString content = file.readAll().trimmed();
        bool ok;
        qint64 bytes = content.toLongLong(&ok);
        file.close();
        
        if (ok) {
            return bytes / (1024 * 1024); // Convert bytes to MB
        }
    }
    
    return 0;
}

qint64 GPUDetector::detectIntelVRAM()
{
    // Intel integrated graphics - estimate from system RAM
    // Typically allocates 20-50% of system RAM dynamically
    
    QFile file("/proc/meminfo");
    if (!file.open(QIODevice::ReadOnly)) {
        return 0;
    }
    
    QString content = file.readAll();
    file.close();
    
    QRegularExpression re("MemTotal:\\s+(\\d+)\\s+kB");
    QRegularExpressionMatch match = re.match(content);
    
    if (match.hasMatch()) {
        qint64 totalRAM_KB = match.captured(1).toLongLong();
        qint64 totalRAM_MB = totalRAM_KB / 1024;
        
        // Estimate 25% of RAM for Intel integrated graphics
        // But cap at 8GB for safety
        qint64 estimatedVRAM = qMin(totalRAM_MB / 4, 8192LL);
        
        // Only return if we detect Intel GPU in lspci
        QProcess lspci;
        lspci.start("lspci");
        if (lspci.waitForFinished(2000)) {
            QString pciOutput = lspci.readAllStandardOutput();
            if (pciOutput.contains("Intel", Qt::CaseInsensitive) && 
                pciOutput.contains("VGA", Qt::CaseInsensitive)) {
                return estimatedVRAM;
            }
        }
    }
    
    return 0;
}

