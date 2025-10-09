#ifndef GPU_DETECTOR_H
#define GPU_DETECTOR_H

#include <QObject>
#include <QString>
#include <QProcess>

class GPUDetector : public QObject
{
    Q_OBJECT

public:
    explicit GPUDetector(QObject *parent = nullptr);
    
    struct GPUInfo {
        QString name;
        qint64 vramMB;  // VRAM in megabytes
        bool detected;
    };
    
    GPUInfo detectGPU();
    qint64 getVRAM();  // Returns VRAM in MB, or 0 if not detected
    
private:
    qint64 detectNvidiaVRAM();
    qint64 detectAMDVRAM();
    qint64 detectIntelVRAM();
};

#endif // GPU_DETECTOR_H

