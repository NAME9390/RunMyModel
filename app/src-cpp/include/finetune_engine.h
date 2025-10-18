#ifndef FINETUNE_ENGINE_H
#define FINETUNE_ENGINE_H

#include <QString>
#include <QObject>
#include <QThread>
#include <QProcess>
#include <atomic>
#include <memory>

/**
 * Fine-tuning engine for GGUF models
 * Integrates with llama.cpp's fine-tuning capabilities
 */
class FineTuneEngine : public QObject {
    Q_OBJECT

public:
    struct FineTuneConfig {
        QString baseModelPath;
        QString trainingDataPath;
        QString outputModelPath;
        
        // Training parameters
        int nCtx = 512;
        int nBatch = 512;
        int nUBatch = 512;
        int nGpuLayers = 999;  // Max GPU offload
        int epochs = 2;
        float learningRate = 0.001f;
        float validationSplit = 0.1f;
        
        // Advanced options
        bool useCheckpointing = true;
        int checkpointInterval = 100;
        bool useGradientCheckpointing = true;
    };

    explicit FineTuneEngine(QObject *parent = nullptr);
    ~FineTuneEngine();
    
    /**
     * Start fine-tuning process
     * @param config Fine-tuning configuration
     * @return true if started successfully
     */
    bool startFineTuning(const FineTuneConfig &config);
    
    /**
     * Stop ongoing fine-tuning
     */
    void stop();
    
    /**
     * Check if fine-tuning is in progress
     */
    bool isRunning() const { return m_isRunning; }
    
    /**
     * Get progress percentage (0-100)
     */
    int getProgress() const { return m_progress; }
    
    /**
     * Prepare training data from JSONL format
     * @param jsonlPath Path to JSONL training data
     * @param outputPath Path to output prepared data
     * @return true if successful
     */
    bool prepareTrainingData(const QString &jsonlPath, const QString &outputPath);
    
    /**
     * Validate a GGUF model file
     * @param modelPath Path to model
     * @return true if valid
     */
    static bool validateModel(const QString &modelPath);
    
signals:
    // Emitted when training starts
    void trainingStarted();
    
    // Emitted with progress updates (0-100)
    void progressUpdated(int percent, const QString &message);
    
    // Emitted when an epoch completes
    void epochCompleted(int epoch, int totalEpochs, float loss);
    
    // Emitted when training completes successfully
    void trainingCompleted(const QString &outputModelPath);
    
    // Emitted when training fails
    void trainingFailed(const QString &error);
    
    // Emitted with log messages
    void logMessage(const QString &message);
    
private slots:
    void onProcessReadyRead();
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);
    void onProcessError(QProcess::ProcessError error);
    
private:
    void parseProgress(const QString &line);
    QString findLlamaFinetuneBinary();
    
    QProcess *m_process = nullptr;
    FineTuneConfig m_config;
    std::atomic<bool> m_isRunning{false};
    std::atomic<int> m_progress{0};
    int m_currentEpoch = 0;
    QString m_llamaCppPath;
};

#endif // FINETUNE_ENGINE_H

