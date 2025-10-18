#include "rag_system.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QHash>
#include <QSet>
#include <QDateTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QProcess>
#include <QThread>
#include <QMutexLocker>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QTextStream>
#include <QDebug>
#include <QLoggingCategory>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QTextDocument>
#include <QTextCursor>
#include <QTextBlock>
#include <QHash>
#include <QSet>
#include <QDateTime>
#include <QTimer>
#include <QElapsedTimer>
#include <QCoreApplication>
#include <QProcess>
#include <QThread>
#include <QMutexLocker>
#include <QFutureWatcher>
#include <QtConcurrent>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QTextStream>
#include <QDebug>
#include <QLoggingCategory>

Q_LOGGING_CATEGORY(ragSystem, "rag.system")

RAGSystem::RAGSystem(QObject *parent)
    : QObject(parent)
    , m_maxContextLength(2000)
    , m_relevanceThreshold(0.3)
    , m_embeddingModel("simple")
{
    initializeKnowledgeBase();
    qCDebug(ragSystem) << "RAG System initialized";
}

RAGSystem::~RAGSystem()
{
    saveKnowledgeBase();
    qCDebug(ragSystem) << "RAG System destroyed";
}

void RAGSystem::initializeKnowledgeBase()
{
    // Set up knowledge base path
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir().mkpath(dataPath);
    m_knowledgeBasePath = dataPath + "/knowledge_base.json";
    
    // Load existing knowledge base
    loadKnowledgeBase();
    
    qCDebug(ragSystem) << "Knowledge base initialized at:" << m_knowledgeBasePath;
}

bool RAGSystem::addDocument(const QString &filePath, const QString &title)
{
    QMutexLocker locker(&m_mutex);
    
    if (!QFile::exists(filePath)) {
        emit errorOccurred(QString("File does not exist: %1").arg(filePath));
        return false;
    }
    
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        emit errorOccurred(QString("Cannot open file: %1").arg(filePath));
        return false;
    }
    
    QTextStream stream(&file);
    // stream.setCodec("UTF-8"); // Not needed in Qt6
    QString content = stream.readAll();
    file.close();
    
    QString docTitle = title.isEmpty() ? QFileInfo(filePath).baseName() : title;
    
    // Process document asynchronously
    QFuture<void> future = QtConcurrent::run([this, content, docTitle]() {
        processDocumentAsync(content, docTitle);
    });
    
    return true;
}

bool RAGSystem::addText(const QString &text, const QString &title)
{
    QMutexLocker locker(&m_mutex);
    
    if (text.isEmpty()) {
        emit errorOccurred("Text content is empty");
        return false;
    }
    
    QString docTitle = title.isEmpty() ? QString("Text_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss")) : title;
    
    // Process text asynchronously
    QFuture<void> future = QtConcurrent::run([this, text, docTitle]() {
        processDocumentAsync(text, docTitle);
    });
    
    return true;
}

void RAGSystem::processDocumentAsync(const QString &content, const QString &title)
{
    emit processingProgress(10);
    
    // Clean and prepare text
    QString cleanContent = cleanText(content);
    emit processingProgress(30);
    
    // Extract metadata
    QMap<QString, QVariant> metadata = extractMetadata(cleanContent);
    emit processingProgress(50);
    
    // Create knowledge entry
    KnowledgeEntry entry;
    entry.title = title;
    entry.content = cleanContent;
    entry.metadata = metadata;
    entry.lastModified = QDateTime::currentDateTime();
    
    // Chunk the content
    QStringList chunks = chunkText(cleanContent);
    entry.chunks = chunks;
    emit processingProgress(70);
    
    // Generate embeddings for each chunk
    for (int i = 0; i < chunks.size(); ++i) {
        DocumentChunk chunk;
        chunk.content = chunks[i];
        chunk.title = title;
        chunk.chunkId = generateChunkId(title, i);
        chunk.metadata = metadata;
        
        // Generate simple embedding (word frequency based)
        chunk.embedding = generateEmbedding(chunks[i]);
        
        m_documentChunks.append(chunk);
    }
    
    // Store in knowledge base
    {
        QMutexLocker locker(&m_mutex);
        m_knowledgeBase[title] = entry;
    }
    
    emit processingProgress(100);
    emit documentAdded(title);
    
    qCDebug(ragSystem) << "Processed document:" << title << "with" << chunks.size() << "chunks";
}

bool RAGSystem::removeDocument(const QString &title)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_knowledgeBase.contains(title)) {
        emit errorOccurred(QString("Document not found: %1").arg(title));
        return false;
    }
    
    // Remove from knowledge base
    m_knowledgeBase.remove(title);
    
    // Remove associated chunks
    m_documentChunks.erase(
        std::remove_if(m_documentChunks.begin(), m_documentChunks.end(),
            [title](const DocumentChunk &chunk) {
                return chunk.title == title;
            }),
        m_documentChunks.end()
    );
    
    emit documentRemoved(title);
    qCDebug(ragSystem) << "Removed document:" << title;
    
    return true;
}

QStringList RAGSystem::getDocumentTitles() const
{
    QMutexLocker locker(&m_mutex);
    return m_knowledgeBase.keys();
}

int RAGSystem::getDocumentCount() const
{
    QMutexLocker locker(&m_mutex);
    return m_knowledgeBase.size();
}

void RAGSystem::clearKnowledgeBase()
{
    QMutexLocker locker(&m_mutex);
    
    m_knowledgeBase.clear();
    m_documentChunks.clear();
    
    emit knowledgeBaseCleared();
    qCDebug(ragSystem) << "Knowledge base cleared";
}

QStringList RAGSystem::retrieveRelevantContext(const QString &query, int maxResults)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_documentChunks.isEmpty()) {
        return QStringList();
    }
    
    QString cleanQuery = cleanText(query);
    QVector<double> queryEmbedding = generateEmbedding(cleanQuery);
    
    // Calculate relevance scores for all chunks
    QVector<QPair<double, QString>> scoredChunks;
    
    for (const DocumentChunk &chunk : m_documentChunks) {
        double score = cosineSimilarity(queryEmbedding, chunk.embedding);
        if (score >= m_relevanceThreshold) {
            scoredChunks.append(qMakePair(score, chunk.content));
        }
    }
    
    // Sort by relevance score (descending)
    std::sort(scoredChunks.begin(), scoredChunks.end(),
        [](const QPair<double, QString> &a, const QPair<double, QString> &b) {
            return a.first > b.first;
        });
    
    // Return top results
    QStringList results;
    int count = qMin(maxResults, scoredChunks.size());
    for (int i = 0; i < count; ++i) {
        results.append(scoredChunks[i].second);
    }
    
    qCDebug(ragSystem) << "Retrieved" << results.size() << "relevant chunks for query:" << query;
    return results;
}

QString RAGSystem::generateContextualPrompt(const QString &query, const QString &basePrompt)
{
    QStringList relevantContext = retrieveRelevantContext(query, 3);
    
    if (relevantContext.isEmpty()) {
        return basePrompt.isEmpty() ? query : basePrompt + "\n\n" + query;
    }
    
    QString contextPrompt = "Based on the following context:\n\n";
    for (int i = 0; i < relevantContext.size(); ++i) {
        contextPrompt += QString("Context %1:\n%2\n\n").arg(i + 1).arg(relevantContext[i]);
    }
    
    contextPrompt += "Please answer the following question:\n" + query;
    
    if (!basePrompt.isEmpty()) {
        contextPrompt = basePrompt + "\n\n" + contextPrompt;
    }
    
    return contextPrompt;
}

double RAGSystem::calculateRelevanceScore(const QString &query, const QString &text)
{
    QVector<double> queryEmbedding = generateEmbedding(cleanText(query));
    QVector<double> textEmbedding = generateEmbedding(cleanText(text));
    
    return cosineSimilarity(queryEmbedding, textEmbedding);
}

bool RAGSystem::saveKnowledgeBase(const QString &filePath)
{
    QString savePath = filePath.isEmpty() ? m_knowledgeBasePath : filePath;
    
    QJsonObject root;
    QJsonArray documents;
    
    QMutexLocker locker(&m_mutex);
    
    for (auto it = m_knowledgeBase.begin(); it != m_knowledgeBase.end(); ++it) {
        QJsonObject doc;
        doc["title"] = it.key();
        doc["content"] = it.value().content;
        doc["lastModified"] = it.value().lastModified.toString(Qt::ISODate);
        
        QJsonArray chunks;
        for (const QString &chunk : it.value().chunks) {
            chunks.append(chunk);
        }
        doc["chunks"] = chunks;
        
        QJsonObject metadata;
        for (auto metaIt = it.value().metadata.begin(); metaIt != it.value().metadata.end(); ++metaIt) {
            metadata[metaIt.key()] = QJsonValue::fromVariant(metaIt.value());
        }
        doc["metadata"] = metadata;
        
        documents.append(doc);
    }
    
    root["documents"] = documents;
    root["version"] = "1.0";
    root["lastSaved"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    
    QJsonDocument doc(root);
    
    QFile file(savePath);
    if (!file.open(QIODevice::WriteOnly)) {
        emit errorOccurred(QString("Cannot save knowledge base: %1").arg(savePath));
        return false;
    }
    
    file.write(doc.toJson());
    file.close();
    
    qCDebug(ragSystem) << "Knowledge base saved to:" << savePath;
    return true;
}

bool RAGSystem::loadKnowledgeBase(const QString &filePath)
{
    QString loadPath = filePath.isEmpty() ? m_knowledgeBasePath : filePath;
    
    if (!QFile::exists(loadPath)) {
        qCDebug(ragSystem) << "Knowledge base file does not exist:" << loadPath;
        return true; // Not an error if file doesn't exist
    }
    
    QFile file(loadPath);
    if (!file.open(QIODevice::ReadOnly)) {
        emit errorOccurred(QString("Cannot load knowledge base: %1").arg(loadPath));
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        emit errorOccurred(QString("JSON parse error: %1").arg(error.errorString()));
        return false;
    }
    
    QJsonObject root = doc.object();
    QJsonArray documents = root["documents"].toArray();
    
    QMutexLocker locker(&m_mutex);
    
    m_knowledgeBase.clear();
    m_documentChunks.clear();
    
    for (const QJsonValue &value : documents) {
        QJsonObject docObj = value.toObject();
        
        KnowledgeEntry entry;
        entry.title = docObj["title"].toString();
        entry.content = docObj["content"].toString();
        entry.lastModified = QDateTime::fromString(docObj["lastModified"].toString(), Qt::ISODate);
        
        QJsonArray chunks = docObj["chunks"].toArray();
        for (const QJsonValue &chunkValue : chunks) {
            entry.chunks.append(chunkValue.toString());
        }
        
        QJsonObject metadata = docObj["metadata"].toObject();
        for (auto it = metadata.begin(); it != metadata.end(); ++it) {
            entry.metadata[it.key()] = it.value().toVariant();
        }
        
        m_knowledgeBase[entry.title] = entry;
        
        // Recreate document chunks
        for (int i = 0; i < entry.chunks.size(); ++i) {
            DocumentChunk chunk;
            chunk.content = entry.chunks[i];
            chunk.title = entry.title;
            chunk.chunkId = generateChunkId(entry.title, i);
            chunk.metadata = entry.metadata;
            chunk.embedding = generateEmbedding(entry.chunks[i]);
            
            m_documentChunks.append(chunk);
        }
    }
    
    qCDebug(ragSystem) << "Loaded knowledge base with" << m_knowledgeBase.size() << "documents";
    return true;
}

QString RAGSystem::getKnowledgeBasePath() const
{
    return m_knowledgeBasePath;
}

void RAGSystem::setMaxContextLength(int length)
{
    m_maxContextLength = qMax(100, length);
    qCDebug(ragSystem) << "Max context length set to:" << m_maxContextLength;
}

void RAGSystem::setRelevanceThreshold(double threshold)
{
    m_relevanceThreshold = qBound(0.0, threshold, 1.0);
    qCDebug(ragSystem) << "Relevance threshold set to:" << m_relevanceThreshold;
}

void RAGSystem::setEmbeddingModel(const QString &model)
{
    m_embeddingModel = model;
    qCDebug(ragSystem) << "Embedding model set to:" << model;
}

QStringList RAGSystem::chunkText(const QString &text, int chunkSize)
{
    QStringList chunks;
    QStringList sentences = text.split(QRegularExpression("[.!?]+"), Qt::SkipEmptyParts);
    
    QString currentChunk;
    for (const QString &sentence : sentences) {
        QString trimmedSentence = sentence.trimmed();
        if (trimmedSentence.isEmpty()) continue;
        
        if (currentChunk.length() + trimmedSentence.length() + 1 <= chunkSize) {
            if (!currentChunk.isEmpty()) {
                currentChunk += " ";
            }
            currentChunk += trimmedSentence;
        } else {
            if (!currentChunk.isEmpty()) {
                chunks.append(currentChunk.trimmed());
            }
            currentChunk = trimmedSentence;
        }
    }
    
    if (!currentChunk.isEmpty()) {
        chunks.append(currentChunk.trimmed());
    }
    
    return chunks;
}

QVector<double> RAGSystem::generateEmbedding(const QString &text)
{
    // Simple word frequency-based embedding
    QHash<QString, int> wordCounts;
    QStringList words = text.toLower().split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
    
    for (const QString &word : words) {
        if (word.length() > 2) { // Skip very short words
            wordCounts[word]++;
        }
    }
    
    // Create a fixed-size embedding vector (100 dimensions)
    QVector<double> embedding(100, 0.0);
    
    // Use hash of words to distribute across dimensions
    int index = 0;
    for (auto it = wordCounts.begin(); it != wordCounts.end(); ++it) {
        uint hash = qHash(it.key());
        int dim = hash % 100;
        embedding[dim] += it.value();
        index++;
    }
    
    // Normalize the vector
    double norm = 0.0;
    for (double val : embedding) {
        norm += val * val;
    }
    norm = qSqrt(norm);
    
    if (norm > 0) {
        for (double &val : embedding) {
            val /= norm;
        }
    }
    
    return embedding;
}

QStringList RAGSystem::extractKeywords(const QString &text)
{
    QHash<QString, int> wordCounts;
    QStringList words = text.toLower().split(QRegularExpression("\\W+"), Qt::SkipEmptyParts);
    
    // Common stop words to filter out
    QSet<QString> stopWords = {
        "the", "a", "an", "and", "or", "but", "in", "on", "at", "to", "for", "of", "with", "by",
        "is", "are", "was", "were", "be", "been", "being", "have", "has", "had", "do", "does", "did",
        "will", "would", "could", "should", "may", "might", "must", "can", "this", "that", "these", "those"
    };
    
    for (const QString &word : words) {
        if (word.length() > 3 && !stopWords.contains(word)) {
            wordCounts[word]++;
        }
    }
    
    // Sort by frequency and return top keywords
    QVector<QPair<int, QString>> sortedWords;
    for (auto it = wordCounts.begin(); it != wordCounts.end(); ++it) {
        sortedWords.append(qMakePair(it.value(), it.key()));
    }
    
    std::sort(sortedWords.begin(), sortedWords.end(),
        [](const QPair<int, QString> &a, const QPair<int, QString> &b) {
            return a.first > b.first;
        });
    
    QStringList keywords;
    int count = qMin(10, sortedWords.size());
    for (int i = 0; i < count; ++i) {
        keywords.append(sortedWords[i].second);
    }
    
    return keywords;
}

double RAGSystem::cosineSimilarity(const QVector<double> &vec1, const QVector<double> &vec2)
{
    if (vec1.size() != vec2.size()) {
        return 0.0;
    }
    
    double dotProduct = 0.0;
    double norm1 = 0.0;
    double norm2 = 0.0;
    
    for (int i = 0; i < vec1.size(); ++i) {
        dotProduct += vec1[i] * vec2[i];
        norm1 += vec1[i] * vec1[i];
        norm2 += vec2[i] * vec2[i];
    }
    
    if (norm1 == 0.0 || norm2 == 0.0) {
        return 0.0;
    }
    
    return dotProduct / (qSqrt(norm1) * qSqrt(norm2));
}

QString RAGSystem::cleanText(const QString &text)
{
    QString cleaned = text;
    
    // Remove extra whitespace
    cleaned = cleaned.simplified();
    
    // Remove special characters but keep punctuation
    cleaned = cleaned.replace(QRegularExpression("[\\r\\n]+"), " ");
    
    // Remove multiple spaces
    cleaned = cleaned.replace(QRegularExpression("\\s+"), " ");
    
    return cleaned.trimmed();
}

QMap<QString, QVariant> RAGSystem::extractMetadata(const QString &text)
{
    QMap<QString, QVariant> metadata;
    
    metadata["wordCount"] = text.split(QRegularExpression("\\W+"), Qt::SkipEmptyParts).size();
    metadata["charCount"] = text.length();
    metadata["sentenceCount"] = text.split(QRegularExpression("[.!?]+"), Qt::SkipEmptyParts).size();
    metadata["created"] = QDateTime::currentDateTime();
    
    // Extract keywords
    QStringList keywords = extractKeywords(text);
    metadata["keywords"] = keywords;
    
    return metadata;
}

QString RAGSystem::generateChunkId(const QString &title, int index)
{
    return QString("%1_chunk_%2").arg(title).arg(index);
}
