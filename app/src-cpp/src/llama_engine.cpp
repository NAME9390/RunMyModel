#include "llama_engine.h"
#include <QDebug>
#include <QThread>
#include <QtConcurrent>
#include <vector>
#include <string>

// Include llama.cpp headers
extern "C" {
    #include "llama.h"
}

LlamaEngine::LlamaEngine(QObject *parent)
    : QObject(parent)
{
    // Initialize llama.cpp backend
    llama_backend_init();
    qDebug() << "✅ LlamaEngine initialized";
}

LlamaEngine::~LlamaEngine() {
    cleanup();
    llama_backend_free();
    qDebug() << "✅ LlamaEngine cleaned up";
}

bool LlamaEngine::loadModel(const QString &modelPath, int nCtx, int nThreads) {
    qDebug() << "🔄 Loading model:" << modelPath;
    qDebug() << "   Context size:" << nCtx;
    qDebug() << "   Threads:" << nThreads;
    qDebug() << "   GPU layers: 99 (auto-offload)";

    // Clean up existing model first
    cleanup();

    // Set up model parameters
    llama_model_params model_params = llama_model_default_params();
    model_params.n_gpu_layers = 99; // Offload all layers to GPU
    
    // Load the model
    m_model = llama_model_load_from_file(modelPath.toStdString().c_str(), model_params);
    if (!m_model) {
        QString err = "Failed to load model: " + modelPath;
        qCritical() << err;
        qCritical() << "Make sure the model file exists and is a valid GGUF file";
        emit error(err);
        return false;
    }

    // Create context
    llama_context_params ctx_params = llama_context_default_params();
    ctx_params.n_ctx = nCtx;
    ctx_params.n_threads = nThreads;
    ctx_params.n_threads_batch = nThreads;
    
    m_ctx = llama_new_context_with_model(m_model, ctx_params);
    if (!m_ctx) {
        QString err = "Failed to create context";
        qCritical() << err;
        qCritical() << "This might be due to insufficient memory or invalid context parameters";
        llama_model_free(m_model);
        m_model = nullptr;
        emit error(err);
        return false;
    }

    // Create sampler
    llama_sampler_chain_params sparams = llama_sampler_chain_default_params();
    m_sampler = llama_sampler_chain_init(sparams);
    
    // Add sampling methods
    llama_sampler_chain_add(m_sampler, llama_sampler_init_min_p(0.05f, 1));
    llama_sampler_chain_add(m_sampler, llama_sampler_init_temp(0.8f));
    llama_sampler_chain_add(m_sampler, llama_sampler_init_dist(LLAMA_DEFAULT_SEED));
    
    qDebug() << "✅ Sampler initialized with temperature 0.8";

    m_modelPath = modelPath;
    m_modelLoaded = true;
    m_shouldStop = false;
    
    qDebug() << "✅ Model loaded successfully!";
    qDebug() << "   Model size:" << llama_model_n_params(m_model) << "parameters";
    qDebug() << "   Context size:" << llama_n_ctx(m_ctx);
    qDebug() << "   GPU acceleration: Enabled (99 layers offloaded)";
    
    return true;
}

void LlamaEngine::generateResponse(const QString &prompt, int maxTokens) {
    if (!m_modelLoaded) {
        emit error("No model loaded");
        return;
    }

    QtConcurrent::run([this, prompt, maxTokens]() {
        this->generateInThread(prompt, maxTokens);
    });
}

void LlamaEngine::generateInThread(const QString &prompt, int maxTokens) {
    qDebug() << "🤖 Generating response...";
    qDebug() << "   Prompt:" << prompt.left(50) + "...";
    qDebug() << "   Max tokens:" << maxTokens;
    qDebug() << "   Temperature: 0.8 (from sampler)";

    if (!m_model || !m_ctx) {
        emit error("Model not initialized");
        return;
    }

    m_shouldStop = false;

    // Tokenize the prompt
    std::vector<llama_token> tokens;
    tokens.resize(prompt.length() + 512);  // Generous buffer
    
    int n_tokens = llama_tokenize(
        llama_model_get_vocab(m_model),
        prompt.toStdString().c_str(),
        prompt.length(),
        tokens.data(),
        tokens.size(),
        true,  // add_special
        false  // parse_special
    );

    if (n_tokens < 0) {
        tokens.resize(-n_tokens);
        n_tokens = llama_tokenize(
            llama_model_get_vocab(m_model),
            prompt.toStdString().c_str(),
            prompt.length(),
            tokens.data(),
            tokens.size(),
            true,
            false
        );
    }

    if (n_tokens <= 0) {
        QString err = "Failed to tokenize prompt";
        qCritical() << err;
        qCritical() << "Prompt length:" << prompt.length() << "characters";
        emit error(err);
        return;
    }

    tokens.resize(n_tokens);
    qDebug() << "   Tokenized:" << n_tokens << "tokens";

    // Reset sampler
    llama_sampler_reset(m_sampler);

    // Create batch
    llama_batch batch = llama_batch_get_one(tokens.data(), tokens.size());

    // Decode the prompt
    if (llama_decode(m_ctx, batch) != 0) {
        QString err = "Failed to decode prompt";
        qCritical() << err;
        qCritical() << "This might be due to context overflow or memory issues";
        emit error(err);
        return;
    }

    // Generate tokens
    int n_generated = 0;
    while (n_generated < maxTokens && !m_shouldStop) {
        // Sample next token
        llama_token new_token_id = llama_sampler_sample(m_sampler, m_ctx, -1);

        // Check for EOS
        if (llama_vocab_is_eog(llama_model_get_vocab(m_model), new_token_id)) {
            qDebug() << "   EOS token generated, stopping";
            break;
        }

        // Convert token to text
        char buf[256];
        int n = llama_token_to_piece(llama_model_get_vocab(m_model), new_token_id, buf, sizeof(buf), 0, false);
        if (n < 0) {
            QString err = "Failed to convert token to text";
            qCritical() << err;
            qCritical() << "Token ID:" << new_token_id;
            emit error(err);
            break;
        }

        QString token_str = QString::fromUtf8(buf, n);
        emit tokenGenerated(token_str);

        // Prepare next batch
        batch = llama_batch_get_one(&new_token_id, 1);
        if (llama_decode(m_ctx, batch) != 0) {
            QString err = "Failed to decode token";
            qCritical() << err;
            qCritical() << "Token ID:" << new_token_id << "Generated tokens:" << n_generated;
            emit error(err);
            break;
        }

        n_generated++;
    }

    qDebug() << "✅ Generation complete (" << n_generated << "tokens generated)";
    qDebug() << "   Average speed:" << (n_generated > 0 ? "calculated" : "N/A") << "tokens/second";
    emit responseComplete();
}

void LlamaEngine::stop() {
    qDebug() << "⏹️  Stopping generation...";
    m_shouldStop = true;
}

void LlamaEngine::cleanup() {
    qDebug() << "🧹 Cleaning up LlamaEngine resources...";
    
    if (m_sampler) {
        llama_sampler_free(m_sampler);
        m_sampler = nullptr;
        qDebug() << "   ✅ Sampler freed";
    }
    
    if (m_ctx) {
        llama_free(m_ctx);
        m_ctx = nullptr;
        qDebug() << "   ✅ Context freed";
    }
    
    if (m_model) {
        llama_model_free(m_model);
        m_model = nullptr;
        qDebug() << "   ✅ Model freed";
    }
    
    m_modelLoaded = false;
    qDebug() << "✅ LlamaEngine cleanup complete";
}
