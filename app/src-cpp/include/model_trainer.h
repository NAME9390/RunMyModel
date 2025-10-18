#ifndef MODEL_TRAINER_H
#define MODEL_TRAINER_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QDateTime>
#include <QMutex>
#include <QFuture>
#include <QtConcurrent>
#include <QProcess>
#include <QTimer>

/**
 * @brief Model Trainer for fine-tuning capabilities
 */
class ModelTrainer : public QObject
{
    Q_OBJECT

public:
    explicit ModelTrainer(QObject *parent = nullptr);
    ~ModelTrainer();

    // Training Management
    bool startTraining(const QString &baseModel, const QString &trainingData, const QJsonObject &config);
    bool stopTraining();
    bool isTraining() const;
    
    // Training Configuration
    void setTrainingConfig(const QJsonObject &config);
    QJsonObject getTrainingConfig() const;
    void setTrainingData(const QString &dataPath);
    QString getTrainingData() const;
    
    // Training Progress
    int getTrainingProgress() const;
    QString getTrainingStatus() const;
    QJsonObject getTrainingMetrics() const;
    
    // Model Export
    bool exportTrainedModel(const QString &outputPath);
    QStringList getAvailableExports() const;

signals:
    void trainingStarted();
    void trainingProgress(int percentage);
    void trainingFinished(bool success, const QString &outputPath);
    void trainingError(const QString &error);
    void metricsUpdated(const QJsonObject &metrics);

private slots:
    void processTraining();
    void updateTrainingProgress();

private:
    bool m_isTraining;
    QString m_baseModel;
    QString m_trainingData;
    QJsonObject m_trainingConfig;
    QJsonObject m_trainingMetrics;
    int m_trainingProgress;
    QString m_trainingStatus;
    mutable QMutex m_mutex;
    QTimer *m_progressTimer;
    QFuture<void> m_trainingFuture;
    
    void initializeTrainingConfig();
    QJsonObject createDefaultConfig() const;
    bool validateTrainingData(const QString &dataPath) const;
    bool validateBaseModel(const QString &modelPath) const;
    QString generateOutputPath() const;
    void updateMetrics(const QJsonObject &metrics);
    bool runTrainingProcess(const QStringList &arguments);
    QStringList getTrainingArguments() const;
    void cleanupTraining();
};

#endif // MODEL_TRAINER_H
