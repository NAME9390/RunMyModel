#include "model_trainer.h"
#include <QApplication>
#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QDateTime>
#include <QMutexLocker>
#include <QFuture>
#include <QtConcurrent>
#include <QProcess>
#include <QTimer>

ModelTrainer::ModelTrainer(QObject *parent)
    : QObject(parent)
    , m_isTraining(false)
    , m_trainingProgress(0)
    , m_trainingStatus("Ready")
{
    initializeTrainingConfig();
    m_progressTimer = new QTimer(this);
    connect(m_progressTimer, &QTimer::timeout, this, &ModelTrainer::updateTrainingProgress);
}

ModelTrainer::~ModelTrainer()
{
    if (m_isTraining) {
        stopTraining();
    }
}

void ModelTrainer::initializeTrainingConfig()
{
    m_trainingConfig = createDefaultConfig();
}

bool ModelTrainer::startTraining(const QString &baseModel, const QString &trainingData, const QJsonObject &config)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_isTraining) {
        emit trainingError("Training is already in progress");
        return false;
    }
    
    if (!validateBaseModel(baseModel)) {
        emit trainingError(QString("Invalid base model: %1").arg(baseModel));
        return false;
    }
    
    if (!validateTrainingData(trainingData)) {
        emit trainingError(QString("Invalid training data: %1").arg(trainingData));
        return false;
    }
    
    m_baseModel = baseModel;
    m_trainingData = trainingData;
    m_trainingConfig = config;
    m_isTraining = true;
    m_trainingProgress = 0;
    m_trainingStatus = "Starting";
    
    // Start training process asynchronously
    m_trainingFuture = QtConcurrent::run([this]() {
        processTraining();
    });
    
    m_progressTimer->start(1000); // Update every second
    emit trainingStarted();
    
    return true;
}

bool ModelTrainer::stopTraining()
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_isTraining) {
        return false;
    }
    
    m_isTraining = false;
    m_trainingStatus = "Stopping";
    m_progressTimer->stop();
    
    cleanupTraining();
    
    return true;
}

bool ModelTrainer::isTraining() const
{
    QMutexLocker locker(&m_mutex);
    return m_isTraining;
}

void ModelTrainer::setTrainingConfig(const QJsonObject &config)
{
    QMutexLocker locker(&m_mutex);
    m_trainingConfig = config;
}

QJsonObject ModelTrainer::getTrainingConfig() const
{
    QMutexLocker locker(&m_mutex);
    return m_trainingConfig;
}

void ModelTrainer::setTrainingData(const QString &dataPath)
{
    QMutexLocker locker(&m_mutex);
    m_trainingData = dataPath;
}

QString ModelTrainer::getTrainingData() const
{
    QMutexLocker locker(&m_mutex);
    return m_trainingData;
}

int ModelTrainer::getTrainingProgress() const
{
    QMutexLocker locker(&m_mutex);
    return m_trainingProgress;
}

QString ModelTrainer::getTrainingStatus() const
{
    QMutexLocker locker(&m_mutex);
    return m_trainingStatus;
}

QJsonObject ModelTrainer::getTrainingMetrics() const
{
    QMutexLocker locker(&m_mutex);
    return m_trainingMetrics;
}

bool ModelTrainer::exportTrainedModel(const QString &outputPath)
{
    QMutexLocker locker(&m_mutex);
    
    if (m_isTraining) {
        emit trainingError("Cannot export while training is in progress");
        return false;
    }
    
    // Simulate model export
    QString trainedModelPath = generateOutputPath();
    if (QFile::exists(trainedModelPath)) {
        return QFile::copy(trainedModelPath, outputPath);
    }
    
    return false;
}

QStringList ModelTrainer::getAvailableExports() const
{
    QStringList exports;
    QString exportDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/trained_models";
    
    QDir dir(exportDir);
    if (dir.exists()) {
        QFileInfoList files = dir.entryInfoList(QStringList() << "*.gguf", QDir::Files);
        for (const QFileInfo &fileInfo : files) {
            exports.append(fileInfo.absoluteFilePath());
        }
    }
    
    return exports;
}

void ModelTrainer::processTraining()
{
    emit trainingProgress(0);
    
    // Simulate training process
    for (int i = 0; i <= 100; ++i) {
        if (!m_isTraining) {
            break;
        }
        
        {
            QMutexLocker locker(&m_mutex);
            m_trainingProgress = i;
            m_trainingStatus = QString("Training epoch %1/100").arg(i);
        }
        
        emit trainingProgress(i);
        
        // Simulate training time
        QThread::msleep(100);
        
        // Update metrics
        QJsonObject metrics;
        metrics["epoch"] = i;
        metrics["loss"] = 1.0 - (i / 100.0);
        metrics["accuracy"] = i / 100.0;
        metrics["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
        
        updateMetrics(metrics);
    }
    
    // Training finished
    {
        QMutexLocker locker(&m_mutex);
        m_isTraining = false;
        m_trainingStatus = "Completed";
    }
    
    m_progressTimer->stop();
    
    QString outputPath = generateOutputPath();
    emit trainingFinished(true, outputPath);
}

void ModelTrainer::updateTrainingProgress()
{
    // This is called by the timer to update progress
    emit trainingProgress(m_trainingProgress);
}

QJsonObject ModelTrainer::createDefaultConfig() const
{
    QJsonObject config;
    config["epochs"] = 100;
    config["learningRate"] = 0.001;
    config["batchSize"] = 32;
    config["validationSplit"] = 0.2;
    config["optimizer"] = "adam";
    config["lossFunction"] = "crossentropy";
    config["earlyStopping"] = true;
    config["patience"] = 10;
    
    return config;
}

bool ModelTrainer::validateTrainingData(const QString &dataPath) const
{
    return QFile::exists(dataPath) && QFileInfo(dataPath).size() > 0;
}

bool ModelTrainer::validateBaseModel(const QString &modelPath) const
{
    return QFile::exists(modelPath) && QFileInfo(modelPath).size() > 0;
}

QString ModelTrainer::generateOutputPath() const
{
    QString outputDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/trained_models";
    QDir().mkpath(outputDir);
    
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss");
    return outputDir + "/trained_model_" + timestamp + ".gguf";
}

void ModelTrainer::updateMetrics(const QJsonObject &metrics)
{
    QMutexLocker locker(&m_mutex);
    m_trainingMetrics = metrics;
    emit metricsUpdated(metrics);
}

bool ModelTrainer::runTrainingProcess(const QStringList &arguments)
{
    // In a real implementation, this would run the actual training process
    // For now, we simulate it
    return true;
}

QStringList ModelTrainer::getTrainingArguments() const
{
    QStringList args;
    args << "--base-model" << m_baseModel;
    args << "--training-data" << m_trainingData;
    args << "--output" << generateOutputPath();
    
    if (m_trainingConfig.contains("epochs")) {
        args << "--epochs" << QString::number(m_trainingConfig["epochs"].toInt());
    }
    
    if (m_trainingConfig.contains("learningRate")) {
        args << "--learning-rate" << QString::number(m_trainingConfig["learningRate"].toDouble());
    }
    
    return args;
}

void ModelTrainer::cleanupTraining()
{
    m_trainingStatus = "Stopped";
    m_trainingProgress = 0;
}
