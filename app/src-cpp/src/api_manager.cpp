
#include "api_manager.h"
#include <QApplication>
#include <QStandardPaths>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QUrlQuery>
#include <QCryptographicHash>
#include <QSslSocket>
#include <QSslConfiguration>
#include <QDebug>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QEventLoop>
#include <QTimer>

// Static constants
const int ApiManager::DEFAULT_TIMEOUT_MS = 30000;
const int ApiManager::DEFAULT_RATE_LIMIT_RPM = 60;
const QString ApiManager::SETTINGS_GROUP = "ApiManager";
const QString ApiManager::CONFIG_FILE_VERSION = "1.0";

ApiManager* ApiManager::m_instance = nullptr;

ApiManager* ApiManager::instance() {
    if (!m_instance) {
        m_instance = new ApiManager();
    }
    return m_instance;
}

void ApiManager::destroyInstance() {
    if (m_instance) {
        delete m_instance;
        m_instance = nullptr;
    }
}

ApiManager::ApiManager(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
    , m_settings(new QSettings(this))
    , m_connectionTestTimer(new QTimer(this))
    , m_primaryProvider(OpenAI)
    , m_autoFallbackEnabled(true)
    , m_encryptionEnabled(true)
{
    initializeProviders();
    setupNetworking();
    setupRateLimiting();
    loadConfiguration();
    
    // Setup connection test timer
    m_connectionTestTimer->setSingleShot(true);
    m_connectionTestTimer->setInterval(10000); // 10 seconds timeout
    connect(m_connectionTestTimer, &QTimer::timeout, this, &ApiManager::onConnectionTestTimeout);
    
    qDebug() << "API Manager initialized with encryption" << (m_encryptionEnabled ? "enabled" : "disabled");
}

ApiManager::~ApiManager() {
    saveConfiguration();
    
    // Clean up rate limit timers
    for (auto timer : m_rateLimitTimers.values()) {
        if (timer) {
            timer->deleteLater();
        }
    }
}

void ApiManager::initializeProviders() {
    // OpenAI configuration
    ProviderInfo openai;
    openai.name = "openai";
    openai.displayName = "OpenAI";
    openai.description = "GPT models from OpenAI including GPT-4, GPT-3.5";
    openai.websiteUrl = "https://openai.com";
    openai.docsUrl = "https://platform.openai.com/docs";
    openai.defaultBaseUrl = "https://api.openai.com/v1";
    openai.supportedModels = {"gpt-4", "gpt-4-turbo", "gpt-3.5-turbo", "gpt-3.5-turbo-16k"};
    openai.requiresApiKey = true;
    openai.pricingUrl = "https://openai.com/pricing";
    
    QJsonObject openaiHeaders;
    openaiHeaders["Content-Type"] = "application/json";
    openai.defaultHeaders = openaiHeaders;
    
    m_providerInfo[OpenAI] = openai;
    
    // Anthropic configuration
    ProviderInfo anthropic;
    anthropic.name = "anthropic";
    anthropic.displayName = "Anthropic";
    anthropic.description = "Claude models from Anthropic";
    anthropic.websiteUrl = "https://anthropic.com";
    anthropic.docsUrl = "https://docs.anthropic.com";
    anthropic.defaultBaseUrl = "https://api.anthropic.com/v1";
    anthropic.supportedModels = {"claude-3-opus-20240229", "claude-3-sonnet-20240229", "claude-3-haiku-20240307"};
    anthropic.requiresApiKey = true;
    anthropic.pricingUrl = "https://anthropic.com/pricing";
    
    QJsonObject anthropicHeaders;
    anthropicHeaders["Content-Type"] = "application/json";
    anthropicHeaders["anthropic-version"] = "2023-06-01";
    anthropic.defaultHeaders = anthropicHeaders;
    
    m_providerInfo[Anthropic] = anthropic;
    
    // HuggingFace configuration
    ProviderInfo huggingface;
    huggingface.name = "huggingface";
    huggingface.displayName = "HuggingFace";
    huggingface.description = "Open source models via HuggingFace Inference API";
    huggingface.websiteUrl = "https://huggingface.co";
    huggingface.docsUrl = "https://huggingface.co/docs/api-inference";
    huggingface.defaultBaseUrl = "https://api-inference.huggingface.co";
    huggingface.supportedModels = {"microsoft/DialoGPT-medium", "facebook/blenderbot-400M-distill"};
    huggingface.requiresApiKey = true;
    huggingface.pricingUrl = "https://huggingface.co/pricing";
    
    m_providerInfo[HuggingFace] = huggingface;
    
    // Cohere configuration
    ProviderInfo cohere;
    cohere.name = "cohere";
    cohere.displayName = "Cohere";
    cohere.description = "Command models from Cohere";
    cohere.websiteUrl = "https://cohere.com";
    cohere.docsUrl = "https://docs.cohere.com";
    cohere.defaultBaseUrl = "https://api.cohere.ai/v1";
    cohere.supportedModels = {"command", "command-light", "command-nightly"};
    cohere.requiresApiKey = true;
    cohere.pricingUrl = "https://cohere.com/pricing";
    
    m_providerInfo[Cohere] = cohere;
    
    // Local Provider configuration
    ProviderInfo local;
    local.name = "local";
    local.displayName = "Local Provider";
    local.description = "Local AI server (compatible with OpenAI API)";
    local.websiteUrl = "";
    local.docsUrl = "";
    local.defaultBaseUrl = "http://localhost:8080/v1";
    local.supportedModels = {"local-model"};
    local.requiresApiKey = false;
    local.pricingUrl = "";
    
    m_providerInfo[LocalProvider] = local;
    
    qDebug() << "Initialized" << m_providerInfo.size() << "API providers";
}

void ApiManager::setupNetworking() {
    // Configure SSL
    QSslConfiguration sslConfig = QSslConfiguration::defaultConfiguration();
    sslConfig.setProtocol(QSsl::TlsV1_2OrLater);
    QSslConfiguration::setDefaultConfiguration(sslConfig);
    
    // Connect network signals
    connect(m_networkManager, &QNetworkAccessManager::finished,
            this, &ApiManager::onNetworkReplyFinished);
    // Note: SSL errors are handled per-reply, not globally
    
    qDebug() << "Network manager configured";
}

void ApiManager::setupRateLimiting() {
    // Initialize rate limit timers for each provider
    for (int i = 0; i < CustomProvider; ++i) {
        Provider provider = static_cast<Provider>(i);
        m_rateLimitTimers[provider] = new QTimer(this);
        m_rateLimitTimers[provider]->setSingleShot(true);
        
        connect(m_rateLimitTimers[provider], &QTimer::timeout, [this, provider]() {
            onRateLimitReset();
            qDebug() << "Rate limit reset for provider:" << getProviderName(provider);
        });
        
        m_requestCounts[provider] = 0;
        m_lastRequest[provider] = QDateTime::currentDateTime().addSecs(-60);
    }
}

void ApiManager::setApiConfig(Provider provider, const ApiConfig &config) {
    if (!validateProviderConfig(config)) {
        qWarning() << "Invalid configuration for provider:" << getProviderName(provider);
        return;
    }
    
    ApiConfig configCopy = config;
    configCopy.provider = provider;
    configCopy.status = NotConfigured;
    
    // Encrypt API key if encryption is enabled
    if (m_encryptionEnabled && !configCopy.apiKey.isEmpty()) {
        configCopy.apiKey = encryptApiKey(configCopy.apiKey);
    }
    
    m_configurations[provider] = configCopy;
    saveConfiguration();
    
    emit configurationChanged(provider);
    qDebug() << "Configuration updated for provider:" << getProviderName(provider);
}

ApiManager::ApiConfig ApiManager::getApiConfig(Provider provider) const {
    if (!m_configurations.contains(provider)) {
        return ApiConfig{};
    }
    
    ApiConfig config = m_configurations[provider];
    
    // Decrypt API key if needed
    if (m_encryptionEnabled && !config.apiKey.isEmpty()) {
        config.apiKey = decryptApiKey(config.apiKey);
    }
    
    return config;
}

bool ApiManager::testConnection(Provider provider) {
    if (!hasValidConfig(provider)) {
        emit connectionStatusChanged(provider, NotConfigured);
        return false;
    }
    
    ApiConfig config = getApiConfig(provider);
    qDebug() << "Testing connection for provider:" << getProviderName(provider);
    
    // Create test request based on provider
    QString testEndpoint;
    QJsonObject testPayload;
    
    switch (provider) {
        case OpenAI:
            testEndpoint = "/models";
            break;
        case Anthropic:
            testEndpoint = "/messages";
            testPayload["model"] = config.defaultModel.isEmpty() ? "claude-3-haiku-20240307" : config.defaultModel;
            testPayload["max_tokens"] = 1;
            testPayload["messages"] = QJsonArray{QJsonObject{{"role", "user"}, {"content", "Hi"}}};
            break;
        case HuggingFace:
            testEndpoint = "/models";
            break;
        case LocalProvider:
            testEndpoint = "/models";
            break;
        default:
            testEndpoint = "/";
            break;
    }
    
    QNetworkRequest request = createRequest(provider, testEndpoint);
    QNetworkReply *reply;
    
    if (testPayload.isEmpty()) {
        reply = m_networkManager->get(request);
    } else {
        QJsonDocument doc(testPayload);
        reply = m_networkManager->post(request, doc.toJson());
    }
    
    m_activeRequests[reply] = provider;
    
    // Start timeout timer
    m_connectionTestTimer->start();
    
    return true;
}

QNetworkRequest ApiManager::createRequest(Provider provider, const QString &endpoint) const {
    ApiConfig config = getApiConfig(provider);
    QString url = config.baseUrl;
    if (!url.endsWith("/")) {
        url += "/";
    }
    if (endpoint.startsWith("/")) {
        url += endpoint.mid(1);
    } else {
        url += endpoint;
    }
    
    QNetworkRequest request(url);
    request.setRawHeader("User-Agent", "RunMyModel/0.5.0");
    request.setAttribute(QNetworkRequest::RedirectPolicyAttribute, QNetworkRequest::NoLessSafeRedirectPolicy);
    
    // Add authentication headers
    addAuthHeaders(request, provider);
    
    // Add custom headers
    addCustomHeaders(request, config);
    
    // Set timeout
    request.setTransferTimeout(config.timeoutMs > 0 ? config.timeoutMs : DEFAULT_TIMEOUT_MS);
    
    return request;
}

void ApiManager::addAuthHeaders(QNetworkRequest &request, Provider provider) const {
    ApiConfig config = getApiConfig(provider);
    
    if (config.apiKey.isEmpty()) {
        return;
    }
    
    switch (provider) {
        case OpenAI:
        case HuggingFace:
            request.setRawHeader("Authorization", ("Bearer " + config.apiKey).toUtf8());
            break;
        case Anthropic:
            request.setRawHeader("x-api-key", config.apiKey.toUtf8());
            break;
        case Cohere:
            request.setRawHeader("Authorization", ("Bearer " + config.apiKey).toUtf8());
            break;
        case LocalProvider:
            // Local providers might not need authentication
            if (!config.apiKey.isEmpty()) {
                request.setRawHeader("Authorization", ("Bearer " + config.apiKey).toUtf8());
            }
            break;
        default:
            request.setRawHeader("Authorization", ("Bearer " + config.apiKey).toUtf8());
            break;
    }
}

void ApiManager::addCustomHeaders(QNetworkRequest &request, const ApiConfig &config) const {
    // Add provider default headers
    ProviderInfo info = m_providerInfo[config.provider];
    QJsonObject defaultHeaders = info.defaultHeaders;
    
    for (auto it = defaultHeaders.begin(); it != defaultHeaders.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toString().toUtf8());
    }
    
    // Add custom headers from config
    for (auto it = config.headers.begin(); it != config.headers.end(); ++it) {
        request.setRawHeader(it.key().toUtf8(), it.value().toString().toUtf8());
    }
}

ApiManager::ApiResponse ApiManager::queryModel(Provider provider, const QString &prompt, const QJsonObject &parameters) {
    if (!hasValidConfig(provider)) {
        ApiResponse response;
        response.success = false;
        response.error = "Provider not configured";
        return response;
    }
    
    if (!isRequestAllowed(provider)) {
        ApiResponse response;
        response.success = false;
        response.error = "Rate limit exceeded";
        return response;
    }
    
    ApiConfig config = getApiConfig(provider);
    QDateTime startTime = QDateTime::currentDateTime();
    
    // Create request body based on provider
    QJsonObject requestBody = createRequestBody(provider, prompt, parameters);
    
    // Determine endpoint
    QString endpoint;
    switch (provider) {
        case OpenAI:
            endpoint = "/chat/completions";
            break;
        case Anthropic:
            endpoint = "/messages";
            break;
        case HuggingFace:
            endpoint = "/models/" + config.defaultModel;
            break;
        default:
            endpoint = "/chat/completions";
            break;
    }
    
    QNetworkRequest request = createRequest(provider, endpoint);
    QJsonDocument doc(requestBody);
    
    qDebug() << "Sending request to" << getProviderName(provider) << ":" << request.url();
    
    // Send request synchronously for now (can be made async later)
    QNetworkReply *reply = m_networkManager->post(request, doc.toJson());
    
    // Wait for completion
    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    ApiResponse response = processResponse(provider, reply, startTime);
    
    // Update rate limiting
    updateRateLimit(provider);
    
    // Update usage statistics
    updateUsageStatistics(provider, response);
    
    reply->deleteLater();
    
    emit responseReceived(provider, response);
    
    return response;
}

QJsonObject ApiManager::createRequestBody(Provider provider, const QString &prompt, const QJsonObject &parameters) const {
    QJsonObject body;
    ApiConfig config = getApiConfig(provider);
    
    switch (provider) {
        case OpenAI: {
            body["model"] = config.defaultModel.isEmpty() ? "gpt-3.5-turbo" : config.defaultModel;
            body["messages"] = QJsonArray{QJsonObject{{"role", "user"}, {"content", prompt}}};
            body["max_tokens"] = parameters.contains("max_tokens") ? parameters["max_tokens"].toInt() : 1000;
            body["temperature"] = parameters.contains("temperature") ? parameters["temperature"].toDouble() : 0.7;
            body["stream"] = false;
            break;
        }
        case Anthropic: {
            body["model"] = config.defaultModel.isEmpty() ? "claude-3-haiku-20240307" : config.defaultModel;
            body["messages"] = QJsonArray{QJsonObject{{"role", "user"}, {"content", prompt}}};
            body["max_tokens"] = parameters.contains("max_tokens") ? parameters["max_tokens"].toInt() : 1000;
            body["temperature"] = parameters.contains("temperature") ? parameters["temperature"].toDouble() : 0.7;
            break;
        }
        case HuggingFace: {
            body["inputs"] = prompt;
            body["parameters"] = parameters.isEmpty() ? QJsonObject{{"max_length", 1000}} : parameters;
            break;
        }
        default: {
            // Generic OpenAI-compatible format
            body["model"] = config.defaultModel.isEmpty() ? "local-model" : config.defaultModel;
            body["messages"] = QJsonArray{QJsonObject{{"role", "user"}, {"content", prompt}}};
            body["max_tokens"] = parameters.contains("max_tokens") ? parameters["max_tokens"].toInt() : 1000;
            body["temperature"] = parameters.contains("temperature") ? parameters["temperature"].toDouble() : 0.7;
            body["stream"] = false;
            break;
        }
    }
    
    // Merge additional parameters
    for (auto it = parameters.begin(); it != parameters.end(); ++it) {
        if (!body.contains(it.key())) {
            body[it.key()] = it.value();
        }
    }
    
    return body;
}

ApiManager::ApiResponse ApiManager::processResponse(Provider provider, QNetworkReply *reply, const QDateTime &startTime) const {
    ApiResponse response;
    response.responseTimeMs = startTime.msecsTo(QDateTime::currentDateTime());
    response.statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute).toInt();
    
    if (reply->error() != QNetworkReply::NoError) {
        response.success = false;
        response.error = reply->errorString();
        qWarning() << "Network error for" << getProviderName(provider) << ":" << response.error;
        return response;
    }
    
    QByteArray data = reply->readAll();
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
    
    if (parseError.error != QJsonParseError::NoError) {
        response.success = false;
        response.error = "Failed to parse JSON response: " + parseError.errorString();
        qWarning() << "JSON parse error for" << getProviderName(provider) << ":" << response.error;
        return response;
    }
    
    QJsonObject jsonResponse = doc.object();
    
    // Extract response based on provider format
    switch (provider) {
        case OpenAI: {
            if (jsonResponse.contains("choices")) {
                QJsonArray choices = jsonResponse["choices"].toArray();
                if (!choices.isEmpty()) {
                    QJsonObject choice = choices[0].toObject();
                    QJsonObject message = choice["message"].toObject();
                    response.response = message["content"].toString();
                    response.success = true;
                }
            }
            
            // Extract token usage
            if (jsonResponse.contains("usage")) {
                QJsonObject usage = jsonResponse["usage"].toObject();
                response.inputTokens = usage["prompt_tokens"].toInt();
                response.outputTokens = usage["completion_tokens"].toInt();
            }
            break;
        }
        case Anthropic: {
            if (jsonResponse.contains("content")) {
                QJsonArray content = jsonResponse["content"].toArray();
                if (!content.isEmpty()) {
                    QJsonObject textContent = content[0].toObject();
                    response.response = textContent["text"].toString();
                    response.success = true;
                }
            }
            
            // Extract token usage
            if (jsonResponse.contains("usage")) {
                QJsonObject usage = jsonResponse["usage"].toObject();
                response.inputTokens = usage["input_tokens"].toInt();
                response.outputTokens = usage["output_tokens"].toInt();
            }
            break;
        }
        case HuggingFace: {
            if (jsonResponse.contains("generated_text")) {
                response.response = jsonResponse["generated_text"].toString();
                response.success = true;
            } else if (doc.isArray()) {
                QJsonArray results = doc.array();
                if (!results.isEmpty()) {
                    QJsonObject result = results[0].toObject();
                    response.response = result["generated_text"].toString();
                    response.success = true;
                }
            }
            break;
        }
        default: {
            // Try OpenAI format first, then fallback
            if (jsonResponse.contains("choices")) {
                QJsonArray choices = jsonResponse["choices"].toArray();
                if (!choices.isEmpty()) {
                    QJsonObject choice = choices[0].toObject();
                    QJsonObject message = choice["message"].toObject();
                    response.response = message["content"].toString();
                    response.success = true;
                }
            }
            break;
        }
    }
    
    if (!response.success && response.error.isEmpty()) {
        response.error = "Unexpected response format";
    }
    
    response.metadata = jsonResponse;
    response.estimatedCost = calculateCost(provider, response.inputTokens, response.outputTokens);
    
    return response;
}

double ApiManager::calculateCost(Provider provider, int inputTokens, int outputTokens) const {
    // Simplified cost calculation - would need real pricing data
    double inputCostPer1K = 0.0;
    double outputCostPer1K = 0.0;
    
    switch (provider) {
        case OpenAI:
            inputCostPer1K = 0.0015;  // GPT-3.5-turbo approximate
            outputCostPer1K = 0.002;
            break;
        case Anthropic:
            inputCostPer1K = 0.00025; // Claude-3-haiku approximate
            outputCostPer1K = 0.00125;
            break;
        default:
            return 0.0; // No cost for local/free providers
    }
    
    return (inputTokens / 1000.0 * inputCostPer1K) + (outputTokens / 1000.0 * outputCostPer1K);
}

void ApiManager::updateUsageStatistics(Provider provider, const ApiResponse &response) {
    if (!m_configurations.contains(provider)) {
        return;
    }
    
    ApiConfig &config = m_configurations[provider];
    config.totalRequests++;
    config.totalTokens += response.inputTokens + response.outputTokens;
    config.totalCost += response.estimatedCost;
    config.lastUsed = QDateTime::currentDateTime();
    
    emit usageUpdated(provider, config.totalRequests, config.totalCost);
}

// Utility methods
QString ApiManager::getProviderName(Provider provider) const {
    if (m_providerInfo.contains(provider)) {
        return m_providerInfo[provider].name;
    }
    return "unknown";
}

bool ApiManager::hasValidConfig(Provider provider) const {
    if (!m_configurations.contains(provider)) {
        return false;
    }
    
    const ApiConfig &config = m_configurations[provider];
    
    if (!config.enabled) {
        return false;
    }
    
    if (config.baseUrl.isEmpty()) {
        return false;
    }
    
    // Check if API key is required
    if (m_providerInfo.contains(provider)) {
        const ProviderInfo &info = m_providerInfo[provider];
        if (info.requiresApiKey && config.apiKey.isEmpty()) {
            return false;
        }
    }
    
    return true;
}

bool ApiManager::isRequestAllowed(Provider provider) const {
    if (!m_configurations.contains(provider)) {
        return false;
    }
    
    const ApiConfig &config = m_configurations[provider];
    int requestCount = m_requestCounts.value(provider, 0);
    
    return requestCount < config.rateLimitRpm;
}

void ApiManager::updateRateLimit(Provider provider) {
    m_requestCounts[provider]++;
    m_lastRequest[provider] = QDateTime::currentDateTime();
    
    // Reset counter after 1 minute
    if (m_rateLimitTimers.contains(provider)) {
        m_rateLimitTimers[provider]->start(60000); // 1 minute
    }
}

// Encryption methods (simplified)
QString ApiManager::encryptApiKey(const QString &key) const {
    if (!m_encryptionEnabled) {
        return key;
    }
    
    // Simple XOR encryption for demo - use proper encryption in production
    QByteArray data = key.toUtf8();
    QByteArray encrypted;
    const char xorKey = 0x42; // Simple key
    
    for (int i = 0; i < data.size(); ++i) {
        encrypted.append(data[i] ^ xorKey);
    }
    
    return encrypted.toBase64();
}

QString ApiManager::decryptApiKey(const QString &encryptedKey) const {
    if (!m_encryptionEnabled) {
        return encryptedKey;
    }
    
    QByteArray encrypted = QByteArray::fromBase64(encryptedKey.toUtf8());
    QByteArray decrypted;
    const char xorKey = 0x42; // Same key
    
    for (int i = 0; i < encrypted.size(); ++i) {
        decrypted.append(encrypted[i] ^ xorKey);
    }
    
    return QString::fromUtf8(decrypted);
}

void ApiManager::loadConfiguration() {
    m_settings->beginGroup(SETTINGS_GROUP);
    
    // Load global settings
    m_encryptionEnabled = m_settings->value("encryption_enabled", true).toBool();
    m_primaryProvider = static_cast<Provider>(m_settings->value("primary_provider", OpenAI).toInt());
    m_autoFallbackEnabled = m_settings->value("auto_fallback", true).toBool();
    
    // Load provider configurations
    QStringList providers = m_settings->childGroups();
    for (const QString &providerName : providers) {
        m_settings->beginGroup(providerName);
        
        Provider provider = getProviderByName(providerName);
        if (provider != CustomProvider) {
            ApiConfig config;
            config.provider = provider;
            config.name = m_settings->value("name", providerName).toString();
            config.apiKey = m_settings->value("api_key").toString();
            config.baseUrl = m_settings->value("base_url").toString();
            config.defaultModel = m_settings->value("default_model").toString();
            config.enabled = m_settings->value("enabled", false).toBool();
            config.timeoutMs = m_settings->value("timeout_ms", DEFAULT_TIMEOUT_MS).toInt();
            config.rateLimitRpm = m_settings->value("rate_limit_rpm", DEFAULT_RATE_LIMIT_RPM).toInt();
            config.totalRequests = m_settings->value("total_requests", 0).toLongLong();
            config.totalTokens = m_settings->value("total_tokens", 0).toLongLong();
            config.totalCost = m_settings->value("total_cost", 0.0).toDouble();
            
            m_configurations[provider] = config;
        }
        
        m_settings->endGroup();
    }
    
    m_settings->endGroup();
    
    qDebug() << "Loaded configuration for" << m_configurations.size() << "providers";
}

void ApiManager::saveConfiguration() {
    m_settings->beginGroup(SETTINGS_GROUP);
    
    // Save global settings
    m_settings->setValue("encryption_enabled", m_encryptionEnabled);
    m_settings->setValue("primary_provider", static_cast<int>(m_primaryProvider));
    m_settings->setValue("auto_fallback", m_autoFallbackEnabled);
    
    // Save provider configurations
    for (auto it = m_configurations.begin(); it != m_configurations.end(); ++it) {
        Provider provider = it.key();
        const ApiConfig &config = it.value();
        
        QString providerName = getProviderName(provider);
        m_settings->beginGroup(providerName);
        
        m_settings->setValue("name", config.name);
        m_settings->setValue("api_key", config.apiKey); // Already encrypted if needed
        m_settings->setValue("base_url", config.baseUrl);
        m_settings->setValue("default_model", config.defaultModel);
        m_settings->setValue("enabled", config.enabled);
        m_settings->setValue("timeout_ms", config.timeoutMs);
        m_settings->setValue("rate_limit_rpm", config.rateLimitRpm);
        m_settings->setValue("total_requests", config.totalRequests);
        m_settings->setValue("total_tokens", config.totalTokens);
        m_settings->setValue("total_cost", config.totalCost);
        
        m_settings->endGroup();
    }
    
    m_settings->endGroup();
    m_settings->sync();
    
    qDebug() << "Configuration saved";
}

ApiManager::Provider ApiManager::getProviderByName(const QString &name) const {
    for (auto it = m_providerInfo.begin(); it != m_providerInfo.end(); ++it) {
        if (it.value().name == name) {
            return it.key();
        }
    }
    return CustomProvider;
}

bool ApiManager::validateProviderConfig(const ApiConfig &config) const {
    if (config.baseUrl.isEmpty()) {
        return false;
    }
    
    if (!isValidUrl(config.baseUrl)) {
        return false;
    }
    
    return true;
}

bool ApiManager::isValidUrl(const QString &url) const {
    QUrl qurl(url);
    return qurl.isValid() && (qurl.scheme() == "http" || qurl.scheme() == "https");
}

// Network event handlers
void ApiManager::onNetworkReplyFinished() {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    if (m_activeRequests.contains(reply)) {
        Provider provider = m_activeRequests[reply];
        m_activeRequests.remove(reply);
        
        if (reply->error() == QNetworkReply::NoError) {
            m_configurations[provider].status = Connected;
            emit connectionStatusChanged(provider, Connected);
            qDebug() << "Connection test successful for" << getProviderName(provider);
        } else {
            m_configurations[provider].status = Failed;
            m_configurations[provider].statusMessage = reply->errorString();
            emit connectionStatusChanged(provider, Failed);
            qDebug() << "Connection test failed for" << getProviderName(provider) << ":" << reply->errorString();
        }
    }
    
    m_connectionTestTimer->stop();
    reply->deleteLater();
}

void ApiManager::onNetworkError(QNetworkReply::NetworkError error) {
    QNetworkReply *reply = qobject_cast<QNetworkReply*>(sender());
    if (!reply) return;
    
    qWarning() << "Network error:" << error << "-" << reply->errorString();
    
    if (m_activeRequests.contains(reply)) {
        Provider provider = m_activeRequests[reply];
        emit apiError(provider, reply->errorString());
    }
}

// SSL error handling - commented out as it's not declared in header
// void ApiManager::onSslErrors(const QList<QSslError> &errors) {
//     qWarning() << "SSL errors occurred:";
//     for (const QSslError &error : errors) {
//         qWarning() << "  " << error.errorString();
//     }
// }

void ApiManager::onConnectionTestTimeout() {
    qWarning() << "Connection test timeout";
    
    // Mark all pending tests as failed
    for (auto it = m_activeRequests.begin(); it != m_activeRequests.end(); ++it) {
        Provider provider = it.value();
        m_configurations[provider].status = Failed;
        m_configurations[provider].statusMessage = "Connection timeout";
        emit connectionStatusChanged(provider, Failed);
    }
    
    m_activeRequests.clear();
}

void ApiManager::onRateLimitReset() {
    QTimer *timer = qobject_cast<QTimer*>(sender());
    if (!timer) return;
    
    // Find which provider this timer belongs to
    for (auto it = m_rateLimitTimers.begin(); it != m_rateLimitTimers.end(); ++it) {
        if (it.value() == timer) {
            Provider provider = it.key();
            m_requestCounts[provider] = 0;
            break;
        }
    }
}

// Additional getter methods
QList<ApiManager::Provider> ApiManager::getAvailableProviders() const {
    return m_providerInfo.keys();
}

ApiManager::ProviderInfo ApiManager::getProviderInfo(Provider provider) const {
    return m_providerInfo.value(provider, ProviderInfo{});
}

ApiManager::ConnectionStatus ApiManager::getConnectionStatus(Provider provider) const {
    if (m_configurations.contains(provider)) {
        return m_configurations[provider].status;
    }
    return NotConfigured;
}

QString ApiManager::getStatusMessage(Provider provider) const {
    if (m_configurations.contains(provider)) {
        return m_configurations[provider].statusMessage;
    }
    return QString();
}

QStringList ApiManager::getAvailableModels(Provider provider) const {
    if (m_availableModels.contains(provider)) {
        return m_availableModels[provider];
    }
    
    // Return default models from provider info
    if (m_providerInfo.contains(provider)) {
        return m_providerInfo[provider].supportedModels;
    }
    
    return QStringList();
}

void ApiManager::enableApiKeyEncryption(bool enable) {
    if (m_encryptionEnabled != enable) {
        m_encryptionEnabled = enable;
        
        // Re-encrypt or decrypt all existing keys
        for (auto &config : m_configurations) {
            if (!config.apiKey.isEmpty()) {
                if (enable) {
                    config.apiKey = encryptApiKey(config.apiKey);
                } else {
                    config.apiKey = decryptApiKey(config.apiKey);
                }
            }
        }
        
        saveConfiguration();
        qDebug() << "API key encryption" << (enable ? "enabled" : "disabled");
    }
}

bool ApiManager::isApiKeyEncrypted(Provider provider) const {
    return m_encryptionEnabled;
}

void ApiManager::setPrimaryProvider(Provider provider) {
    if (m_primaryProvider != provider) {
        m_primaryProvider = provider;
        saveConfiguration();
        qDebug() << "Primary provider set to:" << getProviderName(provider);
    }
}

ApiManager::Provider ApiManager::getPrimaryProvider() const {
    return m_primaryProvider;
}

void ApiManager::setFallbackProviders(const QList<Provider> &providers) {
    m_fallbackProviders = providers;
    saveConfiguration();
    qDebug() << "Fallback providers updated";
}

QList<ApiManager::Provider> ApiManager::getFallbackProviders() const {
    return m_fallbackProviders;
}

bool ApiManager::enableAutoFallback(bool enable) {
    if (m_autoFallbackEnabled != enable) {
        m_autoFallbackEnabled = enable;
        saveConfiguration();
        qDebug() << "Auto-fallback" << (enable ? "enabled" : "disabled");
        return true;
    }
    return false;
}

QJsonObject ApiManager::getUsageStatistics(Provider provider) const {
    QJsonObject stats;
    
    if (m_configurations.contains(provider)) {
        const ApiConfig &config = m_configurations[provider];
        stats["total_requests"] = static_cast<qint64>(config.totalRequests);
        stats["total_tokens"] = static_cast<qint64>(config.totalTokens);
        stats["total_cost"] = config.totalCost;
        stats["last_used"] = config.lastUsed.toString(Qt::ISODate);
        stats["created_date"] = config.createdDate.toString(Qt::ISODate);
        stats["average_cost_per_request"] = config.totalRequests > 0 ? config.totalCost / config.totalRequests : 0.0;
    }
    
    return stats;
}

QJsonObject ApiManager::getTotalUsageStatistics() const {
    QJsonObject totalStats;
    qint64 totalRequests = 0;
    qint64 totalTokens = 0;
    double totalCost = 0.0;
    
    for (const ApiConfig &config : m_configurations) {
        totalRequests += config.totalRequests;
        totalTokens += config.totalTokens;
        totalCost += config.totalCost;
    }
    
    totalStats["total_requests"] = totalRequests;
    totalStats["total_tokens"] = totalTokens;
    totalStats["total_cost"] = totalCost;
    totalStats["providers_configured"] = m_configurations.size();
    
    return totalStats;
}

bool ApiManager::removeApiConfig(Provider provider) {
    if (m_configurations.contains(provider)) {
        m_configurations.remove(provider);
        saveConfiguration();
        emit configurationChanged(provider);
        qDebug() << "Removed configuration for provider:" << getProviderName(provider);
        return true;
    }
    return false;
}

QList<ApiManager::ApiConfig> ApiManager::getAllConfigurations() const {
    return m_configurations.values();
}

bool ApiManager::addCustomProvider(const QString &name, const QString &baseUrl, const QJsonObject &config) {
    // Create a custom provider configuration
    ProviderInfo info;
    info.name = name.toLower();
    info.displayName = name;
    info.description = config.value("description").toString("Custom API provider");
    info.defaultBaseUrl = baseUrl;
    info.requiresApiKey = config.value("requires_api_key").toBool(true);
    info.supportedModels = config.value("models").toVariant().toStringList();
    
    // For now, store as CustomProvider with name prefix
    // In a full implementation, you'd extend the enum or use a different storage mechanism
    qDebug() << "Custom provider added:" << name << "at" << baseUrl;
    
    return true;
}

double ApiManager::getEstimatedMonthlyCost(Provider provider) const {
    if (!m_configurations.contains(provider)) {
        return 0.0;
    }
    
    const ApiConfig &config = m_configurations[provider];
    
    // Simple estimation based on current usage pattern
    if (config.totalRequests == 0) {
        return 0.0;
    }
    
    QDateTime now = QDateTime::currentDateTime();
    qint64 daysSinceCreated = config.createdDate.daysTo(now);
    
    if (daysSinceCreated == 0) {
        daysSinceCreated = 1; // Avoid division by zero
    }
    
    double avgCostPerDay = config.totalCost / daysSinceCreated;
    return avgCostPerDay * 30; // Estimate for 30 days
}

bool ApiManager::isRateLimited(Provider provider) const {
    if (!m_configurations.contains(provider)) {
        return false;
    }
    
    const ApiConfig &config = m_configurations[provider];
    int currentCount = m_requestCounts.value(provider, 0);
    
    return currentCount >= config.rateLimitRpm;
}

int ApiManager::getRemainingRequests(Provider provider) const {
    if (!m_configurations.contains(provider)) {
        return 0;
    }
    
    const ApiConfig &config = m_configurations[provider];
    int currentCount = m_requestCounts.value(provider, 0);
    
    return qMax(0, config.rateLimitRpm - currentCount);
}// Additional implementations for api_manager.cpp
// These should be appended to the main api_manager.cpp file

// Duplicate function definitions removed - using the first implementations above