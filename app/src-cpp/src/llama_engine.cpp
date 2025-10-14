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

    m_modelPath = modelPath;
    m_modelLoaded = true;
    m_shouldStop = false;
    
    qDebug() << "✅ Model loaded successfully!";
    qDebug() << "   Model size:" << llama_model_n_params(m_model) << "parameters";
    qDebug() << "   Context size:" << llama_n_ctx(m_ctx);
    
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
        emit error("Failed to tokenize prompt");
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
        emit error("Failed to decode prompt");
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
            emit error("Failed to convert token to text");
            break;
        }

        QString token_str = QString::fromUtf8(buf, n);
        emit tokenGenerated(token_str);

        // Prepare next batch
        batch = llama_batch_get_one(&new_token_id, 1);
        if (llama_decode(m_ctx, batch) != 0) {
            emit error("Failed to decode token");
            break;
        }

        n_generated++;
    }

    qDebug() << "✅ Generation complete (" << n_generated << "tokens generated)";
    emit responseComplete();
}

void LlamaEngine::stop() {
    qDebug() << "⏹️  Stopping generation...";
    m_shouldStop = true;
}

void LlamaEngine::cleanup() {
    if (m_sampler) {
        llama_sampler_free(m_sampler);
        m_sampler = nullptr;
    }
    
    if (m_ctx) {
        llama_free(m_ctx);
        m_ctx = nullptr;
    }
    
    if (m_model) {
        llama_model_free(m_model);
        m_model = nullptr;
    }
    
    m_modelLoaded = false;
}
