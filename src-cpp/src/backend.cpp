#include "backend.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDir>
#include <QCoreApplication>
#include <QTextStream>
#include <QDebug>

// Legacy backend - kept for UI compatibility
// Real functionality now in Python backend

Backend::Backend(QObject *parent)
    : QObject(parent)
    , m_networkManager(new QNetworkAccessManager(this))
{
    qDebug() << "Legacy Backend initialized (most features now in Python backend)";
}

Backend::~Backend() = default;

QJsonArray Backend::getAllHuggingFaceModels()
{
    // Load models from llms.txt (same as before, but simplified)
    QJsonArray models;
    
    QStringList possiblePaths;
    possiblePaths << QCoreApplication::applicationDirPath() + "/llms.txt"
                  << QDir::currentPath() + "/llms.txt";
    
    QString pickedPath;
    for (const QString &path : possiblePaths) {
        if (QFile::exists(path)) {
            pickedPath = path;
            break;
        }
    }
    
    if (pickedPath.isEmpty()) {
        qWarning() << "❌ No llms.txt found";
        return models;
    }
    
    QFile file(pickedPath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return models;
    }
    
    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine();
        QStringList parts = line.split(',');
        if (parts.size() >= 4) {
            QJsonObject model;
            model["name"] = parts[0].trimmed();
            model["size"] = parts[1].trimmed();
            model["task_type"] = parts[2].trimmed();
            model["rating"] = (parts.size() >= 5) ? parts[3].trimmed() : "N/A";
            model["url"] = (parts.size() >= 5) ? parts[4].trimmed() : parts[3].trimmed();
            model["downloaded"] = false; // TODO: Check actual status
            models.append(model);
        }
    }
    file.close();
    
    return models;
}

QString Backend::downloadHuggingFaceModel(const QString &modelName)
{
    // Placeholder - Python backend handles downloads now
    return QString("Download functionality moved to Python backend. Model: %1").arg(modelName);
}

QString Backend::removeHuggingFaceModel(const QString &modelName)
{
    // Placeholder
    return QString("Remove functionality moved to Python backend. Model: %1").arg(modelName);
}

bool Backend::cancelModelDownload(const QString &modelName)
{
    Q_UNUSED(modelName)
    return false;
}

QJsonObject Backend::getModelDownloadProgress(const QString &modelName)
{
    QJsonObject progress;
    progress["model_name"] = modelName;
    progress["progress"] = 0.0;
    progress["status"] = "not_started";
    return progress;
}

QJsonObject Backend::chatWithHuggingFace(const QJsonObject &request)
{
    QJsonObject response;
    QString modelName = request["model"].toString();

    response["content"] = QString(
        "⚠️ Chat functionality is now handled by Python backend.\n\n"
        "The C++ backend is legacy. Real inference uses:\n"
        "- Python FastAPI backend (localhost:5000)\n"
        "- llama-cpp-python\n"
        "- BackendClient for communication"
    );

    response["error"] = "Use Python backend for inference";

    return response;
}
