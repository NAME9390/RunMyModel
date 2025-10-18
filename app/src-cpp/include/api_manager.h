#ifndef API_MANAGER_H
#define API_MANAGER_H

#include <QObject>
#include <QString>
#include <QVariant>
#include <QJsonObject>
#include <QJsonArray>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QTimer>
#include <QSettings>
#include <QSslConfiguration>

/**
 * @brief Manages external API integrations and secure key storage
 * 
 * This class handles API key management, configuration, and communication
 * with external AI providers like OpenAI, Anthropic, HuggingFace, etc.
 */
class ApiManager : public QObject {
    Q_OBJECT

public:
    enum Provider {
        OpenAI = 0,
        Anthropic,
        HuggingFace,
        Cohere,
        Together,
        Perplexity,
        LocalProvider,
        CustomProvider
    };
    Q_ENUM(Provider)

    enum ConnectionStatus {
        NotConfigured = 0,
        Configured,
        Connected,
        Authenticated,
        Failed,
        RateLimited,
        Expired
    };
    Q_ENUM(ConnectionStatus)

    struct ApiConfig {
        Provider provider;
        QString name;               // User-friendly name
        QString apiKey;            // Encrypted storage
        QString baseUrl;           // API endpoint
        QString defaultModel;      // Default model for this provider
        QJsonObject headers;       // Custom headers
        QJsonObject parameters;    // Default parameters
        bool enabled;
        bool useProxy;
        QString proxyUrl;
        int timeoutMs;
        int rateLimitRpm;          // Requests per minute
        QDateTime lastUsed;
        ConnectionStatus status;
        QString statusMessage;
        
        // Usage statistics
        qint64 totalRequests;
        qint64 totalTokens;
        double totalCost;          // In USD
        QDateTime createdDate;
    };

    struct ApiResponse {
        bool success;
        QString response;
        QString error;
        QJsonObject metadata;
        int statusCode;
        qint64 responseTimeMs;
        int inputTokens;
        int outputTokens;
        double estimatedCost;
    };

    struct ProviderInfo {
        QString name;
        QString displayName;
        QString description;
        QString websiteUrl;
        QString docsUrl;
        QString defaultBaseUrl;
        QStringList supportedModels;
        QJsonObject defaultHeaders;
        QJsonObject parameterSchema;
        bool requiresApiKey;
        QString pricingUrl;
    };

    static ApiManager* instance();
    static void destroyInstance();
    
    // Provider management
    QList<Provider> getAvailableProviders() const;
    ProviderInfo getProviderInfo(Provider provider) const;
    QString getProviderName(Provider provider) const;
    Provider getProviderByName(const QString &name) const;
    
    // Configuration management
    void setApiConfig(Provider provider, const ApiConfig &config);
    ApiConfig getApiConfig(Provider provider) const;
    bool hasValidConfig(Provider provider) const;
    bool removeApiConfig(Provider provider);
    QList<ApiConfig> getAllConfigurations() const;
    
    // Connection testing and validation
    bool testConnection(Provider provider);
    bool validateApiKey(Provider provider, const QString &apiKey);
    ConnectionStatus getConnectionStatus(Provider provider) const;
    QString getStatusMessage(Provider provider) const;
    
    // API communication
    ApiResponse queryModel(Provider provider, const QString &prompt, 
                          const QJsonObject &parameters = QJsonObject());
    ApiResponse queryModelWithHistory(Provider provider, 
                                    const QJsonArray &messageHistory,
                                    const QJsonObject &parameters = QJsonObject());
    ApiResponse queryCustomModel(Provider provider, const QString &model,
                               const QString &prompt,
                               const QJsonObject &parameters = QJsonObject());
    
    // Model information
    QStringList getAvailableModels(Provider provider) const;
    QJsonObject getModelInfo(Provider provider, const QString &model) const;
    bool refreshModelList(Provider provider);
    
    // Streaming support
    bool supportsStreaming(Provider provider) const;
    void startStreamingQuery(Provider provider, const QString &prompt,
                           const QJsonObject &parameters = QJsonObject());
    void cancelStreamingQuery(Provider provider);
    
    // Usage tracking and analytics
    void updateUsageStatistics(Provider provider, const ApiResponse &response);
    QJsonObject getUsageStatistics(Provider provider) const;
    QJsonObject getTotalUsageStatistics() const;
    double getEstimatedMonthlyCost(Provider provider) const;
    
    // Security and encryption
    bool isApiKeyEncrypted(Provider provider) const;
    void enableApiKeyEncryption(bool enable);
    bool exportConfiguration(const QString &filePath, bool includeKeys = false) const;
    bool importConfiguration(const QString &filePath);
    
    // Rate limiting and quotas
    bool isRateLimited(Provider provider) const;
    int getRemainingRequests(Provider provider) const;
    QDateTime getNextAllowedRequest(Provider provider) const;
    void setRateLimit(Provider provider, int requestsPerMinute);
    
    // Fallback and load balancing
    void setPrimaryProvider(Provider provider);
    Provider getPrimaryProvider() const;
    void setFallbackProviders(const QList<Provider> &providers);
    QList<Provider> getFallbackProviders() const;
    bool enableAutoFallback(bool enable);
    
    // Custom providers
    bool addCustomProvider(const QString &name, const QString &baseUrl,
                          const QJsonObject &config);
    bool removeCustomProvider(const QString &name);
    QStringList getCustomProviders() const;

signals:
    void configurationChanged(Provider provider);
    void connectionStatusChanged(Provider provider, ConnectionStatus status);
    void responseReceived(Provider provider, const ApiResponse &response);
    void streamingDataReceived(Provider provider, const QString &data);
    void streamingCompleted(Provider provider);
    void streamingError(Provider provider, const QString &error);
    void rateLimitHit(Provider provider, int resetTimeSeconds);
    void usageUpdated(Provider provider, qint64 totalRequests, double totalCost);
    void apiError(Provider provider, const QString &error);

private slots:
    void onNetworkReplyFinished();
    void onNetworkError(QNetworkReply::NetworkError error);
    void onConnectionTestTimeout();
    void onRateLimitReset();

private:
    explicit ApiManager(QObject *parent = nullptr);
    ~ApiManager();
    
    void initializeProviders();
    void loadConfiguration();
    void saveConfiguration();
    void setupNetworking();
    void setupRateLimiting();
    
    // Encryption/decryption
    QString encryptApiKey(const QString &key) const;
    QString decryptApiKey(const QString &encryptedKey) const;
    
    // Network helpers
    QNetworkRequest createRequest(Provider provider, const QString &endpoint) const;
    QJsonObject createRequestBody(Provider provider, const QString &prompt,
                                 const QJsonObject &parameters) const;
    void addAuthHeaders(QNetworkRequest &request, Provider provider) const;
    void addCustomHeaders(QNetworkRequest &request, const ApiConfig &config) const;
    
    // Response processing
    ApiResponse processResponse(Provider provider, QNetworkReply *reply,
                              const QDateTime &startTime) const;
    double calculateCost(Provider provider, int inputTokens, int outputTokens) const;
    int extractTokenCount(const QJsonObject &response, const QString &field) const;
    
    // Rate limiting
    void updateRateLimit(Provider provider);
    bool isRequestAllowed(Provider provider) const;
    void scheduleRateLimitReset(Provider provider, int seconds);
    
    // Validation
    bool validateProviderConfig(const ApiConfig &config) const;
    bool isValidApiKey(Provider provider, const QString &key) const;
    bool isValidUrl(const QString &url) const;
    
    static ApiManager *m_instance;
    
    QNetworkAccessManager *m_networkManager;
    QSettings *m_settings;
    QTimer *m_connectionTestTimer;
    
    QMap<Provider, ApiConfig> m_configurations;
    QMap<Provider, ProviderInfo> m_providerInfo;
    QMap<Provider, QStringList> m_availableModels;
    QMap<Provider, QDateTime> m_lastRequest;
    QMap<Provider, int> m_requestCounts;
    QMap<Provider, QTimer*> m_rateLimitTimers;
    QMap<QNetworkReply*, Provider> m_activeRequests;
    
    Provider m_primaryProvider;
    QList<Provider> m_fallbackProviders;
    bool m_autoFallbackEnabled;
    bool m_encryptionEnabled;
    QString m_encryptionKey;
    
    // Constants
    static const int DEFAULT_TIMEOUT_MS;
    static const int DEFAULT_RATE_LIMIT_RPM;
    static const QString SETTINGS_GROUP;
    static const QString CONFIG_FILE_VERSION;
};

#endif // API_MANAGER_H