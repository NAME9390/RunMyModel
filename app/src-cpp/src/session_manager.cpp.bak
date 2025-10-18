#include "session_manager.h"
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
#include <QTimer>

SessionManager::SessionManager(QObject *parent)
    : QObject(parent)
    , m_autoSaveEnabled(false)
{
    initializeSessionsDirectory();
    m_autoSaveTimer = new QTimer(this);
    connect(m_autoSaveTimer, &QTimer::timeout, this, &SessionManager::autoSave);
    setAutoSaveInterval(5); // 5 minutes default
}

SessionManager::~SessionManager()
{
    if (m_autoSaveEnabled) {
        autoSave();
    }
}

void SessionManager::initializeSessionsDirectory()
{
    m_sessionsDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/sessions";
    QDir().mkpath(m_sessionsDirectory);
}

bool SessionManager::saveSession(const QString &sessionName, const QJsonObject &sessionData)
{
    QMutexLocker locker(&m_mutex);
    
    QString filePath = getSessionFilePath(sessionName);
    QFile file(filePath);
    
    if (!file.open(QIODevice::WriteOnly)) {
        return false;
    }
    
    QJsonDocument doc(sessionData);
    file.write(doc.toJson());
    file.close();
    
    emit sessionSaved(sessionName);
    return true;
}

bool SessionManager::loadSession(const QString &sessionName)
{
    QMutexLocker locker(&m_mutex);
    
    QString filePath = getSessionFilePath(sessionName);
    QFile file(filePath);
    
    if (!file.open(QIODevice::ReadOnly)) {
        return false;
    }
    
    QByteArray data = file.readAll();
    file.close();
    
    QJsonParseError error;
    QJsonDocument doc = QJsonDocument::fromJson(data, &error);
    
    if (error.error != QJsonParseError::NoError) {
        return false;
    }
    
    m_currentSession = doc.object();
    emit sessionLoaded(sessionName);
    return true;
}

bool SessionManager::deleteSession(const QString &sessionName)
{
    QMutexLocker locker(&m_mutex);
    
    QString filePath = getSessionFilePath(sessionName);
    bool success = QFile::remove(filePath);
    
    if (success) {
        emit sessionDeleted(sessionName);
    }
    
    return success;
}

QStringList SessionManager::getAvailableSessions() const
{
    QDir dir(m_sessionsDirectory);
    QStringList sessions;
    
    QFileInfoList files = dir.entryInfoList(QStringList() << "*.json", QDir::Files);
    for (const QFileInfo &fileInfo : files) {
        sessions.append(fileInfo.baseName());
    }
    
    return sessions;
}

QJsonObject SessionManager::getCurrentSession() const
{
    QMutexLocker locker(&m_mutex);
    return m_currentSession;
}

void SessionManager::setCurrentSession(const QJsonObject &session)
{
    QMutexLocker locker(&m_mutex);
    m_currentSession = session;
}

void SessionManager::clearCurrentSession()
{
    QMutexLocker locker(&m_mutex);
    m_currentSession = createDefaultSession();
}

void SessionManager::enableAutoSave(bool enabled)
{
    m_autoSaveEnabled = enabled;
    if (enabled) {
        m_autoSaveTimer->start();
    } else {
        m_autoSaveTimer->stop();
    }
}

void SessionManager::setAutoSaveInterval(int minutes)
{
    m_autoSaveTimer->setInterval(minutes * 60 * 1000);
}

void SessionManager::autoSave()
{
    if (m_autoSaveEnabled && !m_currentSession.isEmpty()) {
        QString sessionName = m_currentSession["name"].toString();
        if (sessionName.isEmpty()) {
            sessionName = QString("autosave_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        }
        
        saveSession(sessionName, m_currentSession);
        emit autoSaveTriggered();
    }
}

QString SessionManager::getSessionFilePath(const QString &sessionName) const
{
    return m_sessionsDirectory + "/" + sessionName + ".json";
}

QJsonObject SessionManager::createDefaultSession() const
{
    QJsonObject session;
    session["name"] = "";
    session["created"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    session["lastModified"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    session["messages"] = QJsonArray();
    session["settings"] = QJsonObject();
    session["model"] = QJsonObject();
    
    return session;
}
