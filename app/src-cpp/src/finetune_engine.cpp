#include "finetune_engine.h"
#include <QDebug>
#include <QFile>
#include <QFileInfo>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QRegularExpression>
#include <QStandardPaths>

FineTuneEngine::FineTuneEngine(QObject *parent)
    : QObject(parent)
{
    // Find llama.cpp path
    m_llamaCppPath = findLlamaFinetuneBinary();
    
    if (m_llamaCppPath.isEmpty()) {
        qWarning() << "⚠️  llama-finetune binary not found. Fine-tuning will not be available.";
    } else {
        qDebug() << "✅ Found llama-finetune at:" << m_llamaCppPath;
    }
}

FineTuneEngine::~FineTuneEngine() {
    if (m_process) {
        stop();
        m_process->deleteLater();
    }
}

QString FineTuneEngine::findLlamaFinetuneBinary() {
    // Check possible locations for llama-finetune binary
    QStringList possiblePaths = {
        // Relative to app
        QDir::currentPath() + "/lib/llama.cpp/build/bin/llama-finetune",
        QDir::currentPath() + "/../lib/llama.cpp/build/bin/llama-finetune",
        
        // System paths
        "/usr/local/bin/llama-finetune",
        "/usr/bin/llama-finetune",
        
        // Home directory
        QDir::homePath() + "/llama.cpp/build/bin/llama-finetune",
    };
    
    for (const QString &path : possiblePaths) {
        QFileInfo fileInfo(path);
        if (fileInfo.exists() && fileInfo.isExecutable()) {
            return path;
        }
    }
    
    return QString();
}

bool FineTuneEngine::prepareTrainingData(const QString &jsonlPath, const QString &outputPath) {
    qDebug() << "📝 Preparing training data...";
    qDebug() << "   Input:" << jsonlPath;
    qDebug() << "   Output:" << outputPath;
    
    QFile inputFile(jsonlPath);
    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qCritical() << "Failed to open input file:" << jsonlPath;
        return false;
    }
    
    QFile outputFile(outputPath);
    if (!outputFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Failed to open output file:" << outputPath;
        return false;
    }
    
    QTextStream in(&inputFile);
    QTextStream out(&outputFile);
    
    int lineCount = 0;
    while (!in.atEnd()) {
        QString line = in.readLine();
        
        // Parse JSON
        QJsonDocument doc = QJsonDocument::fromJson(line.toUtf8());
        if (doc.isNull() || !doc.isObject()) {
            qWarning() << "Skipping invalid JSON line:" << line.left(50);
            continue;
        }
        
        QJsonObject obj = doc.object();
        QString text;
        
        // Handle different formats
        if (obj.contains("messages")) {
            // Chat format
            QJsonArray messages = obj["messages"].toArray();
            for (const QJsonValue &msg : messages) {
                QJsonObject msgObj = msg.toObject();
                QString role = msgObj["role"].toString();
                QString content = msgObj["content"].toString();
                text += QString("<|%1|>\n%2\n").arg(role, content);
            }
        } else if (obj.contains("instruction")) {
            // Alpaca format
            QString instruction = obj["instruction"].toString();
            QString input = obj["input"].toString();
            QString output = obj["output"].toString();
            
            if (!input.isEmpty()) {
                text = QString("### Instruction:\n%1\n\n### Input:\n%2\n\n### Response:\n%3")
                    .arg(instruction, input, output);
            } else {
                text = QString("### Instruction:\n%1\n\n### Response:\n%2")
                    .arg(instruction, output);
            }
        } else if (obj.contains("text")) {
            // Raw text format
            text = obj["text"].toString();
        }
        
        if (!text.isEmpty()) {
            out << text << "\n\n";
            lineCount++;
        }
    }
    
    inputFile.close();
    outputFile.close();
    
    qDebug() << "✅ Prepared" << lineCount << "training examples";
    return true;
}

bool FineTuneEngine::startFineTuning(const FineTuneConfig &config) {
    if (m_llamaCppPath.isEmpty()) {
        emit trainingFailed("llama-finetune binary not found");
        return false;
    }
    
    if (m_isRunning) {
        qWarning() << "Fine-tuning already in progress";
        return false;
    }
    
    // Validate inputs
    if (!QFile::exists(config.baseModelPath)) {
        emit trainingFailed("Base model not found: " + config.baseModelPath);
        return false;
    }
    
    if (!QFile::exists(config.trainingDataPath)) {
        emit trainingFailed("Training data not found: " + config.trainingDataPath);
        return false;
    }
    
    m_config = config;
    m_currentEpoch = 0;
    m_progress = 0;
    
    qDebug() << "🚀 Starting fine-tuning...";
    qDebug() << "   Base model:" << config.baseModelPath;
    qDebug() << "   Training data:" << config.trainingDataPath;
    qDebug() << "   Output model:" << config.outputModelPath;
    qDebug() << "   Epochs:" << config.epochs;
    qDebug() << "   Context size:" << config.nCtx;
    qDebug() << "   GPU layers:" << config.nGpuLayers;
    
    // Create output directory
    QFileInfo outputInfo(config.outputModelPath);
    QDir().mkpath(outputInfo.absolutePath());
    
    // Build command
    QStringList args;
    args << "--model" << config.baseModelPath;
    args << "--file" << config.trainingDataPath;
    args << "-o" << config.outputModelPath;
    args << "-c" << QString::number(config.nCtx);
    args << "-b" << QString::number(config.nBatch);
    args << "-ub" << QString::number(config.nUBatch);
    args << "-ngl" << QString::number(config.nGpuLayers);
    args << "--epochs" << QString::number(config.epochs);
    
    // Create process
    if (m_process) {
        delete m_process;
    }
    
    m_process = new QProcess(this);
    
    // Connect signals
    connect(m_process, &QProcess::readyReadStandardOutput, this, &FineTuneEngine::onProcessReadyRead);
    connect(m_process, &QProcess::readyReadStandardError, this, &FineTuneEngine::onProcessReadyRead);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &FineTuneEngine::onProcessFinished);
    connect(m_process, &QProcess::errorOccurred, this, &FineTuneEngine::onProcessError);
    
    // Start process
    qDebug() << "Command:" << m_llamaCppPath << args.join(" ");
    m_process->start(m_llamaCppPath, args);
    
    if (!m_process->waitForStarted(5000)) {
        emit trainingFailed("Failed to start fine-tuning process");
        return false;
    }
    
    m_isRunning = true;
    emit trainingStarted();
    emit logMessage("Fine-tuning started...");
    
    return true;
}

void FineTuneEngine::stop() {
    if (m_process && m_isRunning) {
        qDebug() << "⏹️  Stopping fine-tuning...";
        m_process->terminate();
        
        if (!m_process->waitForFinished(5000)) {
            qWarning() << "Process didn't terminate, killing...";
            m_process->kill();
        }
        
        m_isRunning = false;
    }
}

void FineTuneEngine::onProcessReadyRead() {
    if (!m_process) return;
    
    // Read all available output
    QString output = QString::fromUtf8(m_process->readAllStandardOutput());
    QString errorOutput = QString::fromUtf8(m_process->readAllStandardError());
    
    // Combine outputs
    QString allOutput = output + errorOutput;
    
    // Process each line
    QStringList lines = allOutput.split('\n', Qt::SkipEmptyParts);
    for (const QString &line : lines) {
        parseProgress(line);
        emit logMessage(line);
    }
}

void FineTuneEngine::parseProgress(const QString &line) {
    // Parse epoch information
    static QRegularExpression epochRegex(R"(epoch\s*(\d+)\s*/\s*(\d+))");
    QRegularExpressionMatch epochMatch = epochRegex.match(line);
    
    if (epochMatch.hasMatch()) {
        m_currentEpoch = epochMatch.captured(1).toInt();
        int totalEpochs = epochMatch.captured(2).toInt();
        
        // Calculate progress
        m_progress = (m_currentEpoch * 100) / totalEpochs;
        
        emit progressUpdated(m_progress, QString("Epoch %1/%2").arg(m_currentEpoch).arg(totalEpochs));
    }
    
    // Parse loss information
    static QRegularExpression lossRegex(R"(loss[:\s]+(\d+\.\d+))");
    QRegularExpressionMatch lossMatch = lossRegex.match(line);
    
    if (lossMatch.hasMatch() && m_currentEpoch > 0) {
        float loss = lossMatch.captured(1).toFloat();
        emit epochCompleted(m_currentEpoch, m_config.epochs, loss);
    }
    
    // General progress indicators
    if (line.contains("train", Qt::CaseInsensitive) || 
        line.contains("progress", Qt::CaseInsensitive)) {
        emit progressUpdated(m_progress, line);
    }
}

void FineTuneEngine::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus) {
    m_isRunning = false;
    
    qDebug() << "Fine-tuning process finished with code:" << exitCode;
    
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        // Verify output file exists
        if (QFile::exists(m_config.outputModelPath)) {
            m_progress = 100;
            emit progressUpdated(100, "Training completed!");
            emit trainingCompleted(m_config.outputModelPath);
            qDebug() << "✅ Fine-tuning completed successfully!";
        } else {
            emit trainingFailed("Output model file not created");
            qCritical() << "❌ Output model file not found:" << m_config.outputModelPath;
        }
    } else {
        QString error = QString("Fine-tuning failed with code %1").arg(exitCode);
        emit trainingFailed(error);
        qCritical() << "❌" << error;
    }
}

void FineTuneEngine::onProcessError(QProcess::ProcessError error) {
    QString errorMsg;
    
    switch (error) {
        case QProcess::FailedToStart:
            errorMsg = "Failed to start fine-tuning process";
            break;
        case QProcess::Crashed:
            errorMsg = "Fine-tuning process crashed";
            break;
        case QProcess::Timedout:
            errorMsg = "Fine-tuning process timed out";
            break;
        default:
            errorMsg = "Unknown process error";
            break;
    }
    
    qCritical() << "❌ Process error:" << errorMsg;
    emit trainingFailed(errorMsg);
    m_isRunning = false;
}

bool FineTuneEngine::validateModel(const QString &modelPath) {
    QFileInfo fileInfo(modelPath);
    
    // Check if file exists
    if (!fileInfo.exists()) {
        qWarning() << "Model file does not exist:" << modelPath;
        return false;
    }
    
    // Check file size (should be at least 1 MB)
    qint64 sizeInMB = fileInfo.size() / (1024 * 1024);
    if (sizeInMB < 1) {
        qWarning() << "Model file too small:" << sizeInMB << "MB";
        return false;
    }
    
    // Check file extension
    if (!modelPath.endsWith(".gguf", Qt::CaseInsensitive)) {
        qWarning() << "Model file should have .gguf extension";
        return false;
    }
    
    qDebug() << "✅ Model appears valid:" << sizeInMB << "MB";
    return true;
}

