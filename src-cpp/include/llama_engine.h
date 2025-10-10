#ifndef LLAMA_ENGINE_H
#define LLAMA_ENGINE_H

#include <QString>
#include <QObject>
#include <QThread>
#include <atomic>
#include <memory>

// Forward declarations to avoid including llama.h in header
struct llama_model;
struct llama_context;
struct llama_model_params;
struct llama_context_params;
struct llama_sampler;

class LlamaEngine : public QObject {
    Q_OBJECT

public:
    explicit LlamaEngine(QObject *parent = nullptr);
    ~LlamaEngine();
    
    bool loadModel(const QString &modelPath, int nCtx = 2048, int nThreads = 4);
    void generateResponse(const QString &prompt, int maxTokens = 512);
    void stop();
    bool isLoaded() const { return m_modelLoaded; }
    
signals:
    void tokenGenerated(const QString &token);
    void responseComplete();
    void error(const QString &message);
    void loadProgress(int percent);
    
private:
    void generateInThread(const QString &prompt, int maxTokens);
    void cleanup();

    llama_model *m_model = nullptr;
    llama_context *m_ctx = nullptr;
    llama_sampler *m_sampler = nullptr;
    
    QString m_modelPath;
    std::atomic<bool> m_modelLoaded{false};
    std::atomic<bool> m_shouldStop{false};
};

#endif // LLAMA_ENGINE_H
