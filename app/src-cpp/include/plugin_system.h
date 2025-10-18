#ifndef PLUGIN_SYSTEM_H
#define PLUGIN_SYSTEM_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QMap>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonDocument>
#include <QDir>
#include <QStandardPaths>
#include <QDebug>
#include <QMutex>
#include <QPluginLoader>
#include <QDir>
#include <QFileInfo>

/**
 * @brief Plugin System for extensible architecture
 */
class PluginSystem : public QObject
{
    Q_OBJECT

public:
    explicit PluginSystem(QObject *parent = nullptr);
    ~PluginSystem();

    // Plugin Management
    bool loadPlugin(const QString &pluginPath);
    bool unloadPlugin(const QString &pluginName);
    QStringList getLoadedPlugins() const;
    QStringList getAvailablePlugins() const;
    
    // Plugin Information
    QJsonObject getPluginInfo(const QString &pluginName) const;
    QStringList getPluginCapabilities(const QString &pluginName) const;
    
    // Plugin Directory
    void setPluginDirectory(const QString &directory);
    QString getPluginDirectory() const;
    void refreshPluginList();

signals:
    void pluginLoaded(const QString &pluginName);
    void pluginUnloaded(const QString &pluginName);
    void pluginListUpdated();
    void errorOccurred(const QString &error);

private:
    QMap<QString, QPluginLoader*> m_loadedPlugins;
    QString m_pluginDirectory;
    mutable QMutex m_mutex;
    
    void initializePluginDirectory();
    QStringList scanForPlugins() const;
    bool validatePlugin(const QString &pluginPath) const;
    QJsonObject extractPluginInfo(const QString &pluginPath) const;
};

#endif // PLUGIN_SYSTEM_H
