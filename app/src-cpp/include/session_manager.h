#ifndef SESSION_MANAGER_H
#define SESSION_MANAGER_H

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
#include <QTimer>

/**
 * @brief Session Manager for conversation persistence
 */
class SessionManager : public QObject
{
    Q_OBJECT

public:
    explicit SessionManager(QObject *parent = nullptr);
    ~SessionManager();

    // Session Management
    bool saveSession(const QString &sessionName, const QJsonObject &sessionData);
    bool loadSession(const QString &sessionName);
    bool deleteSession(const QString &sessionName);
    QStringList getAvailableSessions() const;
    
    // Session Data
    QJsonObject getCurrentSession() const;
    void setCurrentSession(const QJsonObject &session);
    void clearCurrentSession();
    
    // Auto-save
    void enableAutoSave(bool enabled);
    void setAutoSaveInterval(int minutes);
    void autoSave();

signals:
    void sessionSaved(const QString &sessionName);
    void sessionLoaded(const QString &sessionName);
    void sessionDeleted(const QString &sessionName);
    void autoSaveTriggered();

private:
    QJsonObject m_currentSession;
    QString m_sessionsDirectory;
    mutable QMutex m_mutex;
    QTimer *m_autoSaveTimer;
    bool m_autoSaveEnabled;
    
    void initializeSessionsDirectory();
    QString getSessionFilePath(const QString &sessionName) const;
    QJsonObject createDefaultSession() const;
};

#endif // SESSION_MANAGER_H
