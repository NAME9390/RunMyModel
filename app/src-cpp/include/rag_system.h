#ifndef RAG_SYSTEM_H
#define RAG_SYSTEM_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVector>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QThread>
#include <QMutex>
#include <QFuture>
#include <QtConcurrent>
#include <QRegularExpression>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>

/**
 * @brief RAG (Retrieval-Augmented Generation) System for knowledge ingestion and retrieval
 * 
 * This class implements a complete RAG system that can:
 * - Ingest documents and text files
 * - Create vector embeddings for semantic search
 * - Retrieve relevant context for LLM queries
 * - Manage knowledge base with metadata
 */
class RAGSystem : public QObject
{
    Q_OBJECT

public:
    explicit RAGSystem(QObject *parent = nullptr);
    ~RAGSystem();

    // Knowledge Base Management
    bool addDocument(const QString &filePath, const QString &title = "");
    bool addText(const QString &text, const QString &title = "");
    bool removeDocument(const QString &title);
    QStringList getDocumentTitles() const;
    int getDocumentCount() const;
    void clearKnowledgeBase();

    // RAG Operations
    QStringList retrieveRelevantContext(const QString &query, int maxResults = 5);
    QString generateContextualPrompt(const QString &query, const QString &basePrompt = "");
    double calculateRelevanceScore(const QString &query, const QString &text);

    // Knowledge Base Persistence
    bool saveKnowledgeBase(const QString &filePath = "");
    bool loadKnowledgeBase(const QString &filePath = "");
    QString getKnowledgeBasePath() const;

    // Configuration
    void setMaxContextLength(int length);
    void setRelevanceThreshold(double threshold);
    void setEmbeddingModel(const QString &model);

signals:
    void documentAdded(const QString &title);
    void documentRemoved(const QString &title);
    void knowledgeBaseCleared();
    void processingProgress(int percentage);
    void errorOccurred(const QString &error);

private slots:
    void processDocumentAsync(const QString &filePath, const QString &title);

private:
    struct DocumentChunk {
        QString content;
        QString title;
        QString chunkId;
        QVector<double> embedding;
        QMap<QString, QVariant> metadata;
    };

    struct KnowledgeEntry {
        QString title;
        QString content;
        QVector<QString> chunks;
        QMap<QString, QVariant> metadata;
        QDateTime lastModified;
    };

    // Core data structures
    QMap<QString, KnowledgeEntry> m_knowledgeBase;
    QVector<DocumentChunk> m_documentChunks;
    mutable QMutex m_mutex;

    // Configuration
    int m_maxContextLength;
    double m_relevanceThreshold;
    QString m_embeddingModel;
    QString m_knowledgeBasePath;

    // Helper methods
    QStringList chunkText(const QString &text, int chunkSize = 500);
    QVector<double> generateEmbedding(const QString &text);
    QStringList extractKeywords(const QString &text);
    double cosineSimilarity(const QVector<double> &vec1, const QVector<double> &vec2);
    QString cleanText(const QString &text);
    QMap<QString, QVariant> extractMetadata(const QString &text);
    void initializeKnowledgeBase();
    QString generateChunkId(const QString &title, int index);
};

#endif // RAG_SYSTEM_H
