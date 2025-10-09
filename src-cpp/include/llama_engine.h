#ifndef LLAMA_ENGINE_H
#define LLAMA_ENGINE_H

#include <QObject>
#include <QString>
#include <QThread>
#include <memory>

/**
 * LlamaEngine - Pure C++ wrapper for llama.cpp
 * 
 * Handles model loading, inference, and streaming generation
 * No Python, no REST API - direct C++ calls!
 */
class LlamaEngine : public QObject
{
    Q_OBJECT

public:
    explicit LlamaEngine(QObject *parent = nullptr);
    ~LlamaEngine();

    struct ModelInfo {
        QString name;
        QString path;
        qint64 size_mb;
        bool loaded;
        int context_size;
        int gpu_layers;
    };

    // Model management
    bool loadModel(const QString &modelPath, int nCtx = 4096, int nGpuLayers = -1);
    void unloadModel();
    bool isModelLoaded() const { return m_modelLoaded; }
    QString getCurrentModelPath() const { return m_currentModelPath; }

    // Inference (blocking - use in separate thread)
    QString generate(const QString &prompt, int maxTokens = 512, 
                    float temperature = 0.7, float topP = 0.9, int topK = 40);

    // Streaming inference (emits tokens via signal)
    void generateStreaming(const QString &prompt, int maxTokens = 512,
                          float temperature = 0.7, float topP = 0.9, int topK = 40);

    // Stop generation
    void stopGeneration();

signals:
    void tokenGenerated(const QString &token);
    void generationComplete();
    void generationError(const QString &error);
    void modelLoadProgress(int progress); // 0-100
    void modelLoaded(const QString &modelPath);
    void modelUnloaded();

private:
    // Internal state
    bool m_modelLoaded;
    QString m_currentModelPath;
    bool m_stopRequested;

    // llama.cpp context (opaque pointer - will be implemented)
    void *m_llamaContext;
    void *m_llamaModel;

    // Helper methods
    void initializeLlama();
    void cleanupLlama();
    int detectGpuLayers();
};

#endif // LLAMA_ENGINE_H

