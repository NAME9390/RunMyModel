#include "plugin_system.h"
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
#include <QMutexLocker>
#include <QPluginLoader>

PluginSystem::PluginSystem(QObject *parent)
    : QObject(parent)
{
    initializePluginDirectory();
    refreshPluginList();
}

PluginSystem::~PluginSystem()
{
    // Unload all plugins
    for (auto it = m_loadedPlugins.begin(); it != m_loadedPlugins.end(); ++it) {
        delete it.value();
    }
    m_loadedPlugins.clear();
}

void PluginSystem::initializePluginDirectory()
{
    m_pluginDirectory = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation) + "/plugins";
    QDir().mkpath(m_pluginDirectory);
}

bool PluginSystem::loadPlugin(const QString &pluginPath)
{
    QMutexLocker locker(&m_mutex);
    
    if (!QFile::exists(pluginPath)) {
        emit errorOccurred(QString("Plugin file does not exist: %1").arg(pluginPath));
        return false;
    }
    
    if (!validatePlugin(pluginPath)) {
        emit errorOccurred(QString("Invalid plugin file: %1").arg(pluginPath));
        return false;
    }
    
    QPluginLoader *loader = new QPluginLoader(pluginPath);
    if (!loader->load()) {
        emit errorOccurred(QString("Failed to load plugin: %1").arg(loader->errorString()));
        delete loader;
        return false;
    }
    
    QString pluginName = QFileInfo(pluginPath).baseName();
    m_loadedPlugins[pluginName] = loader;
    
    emit pluginLoaded(pluginName);
    return true;
}

bool PluginSystem::unloadPlugin(const QString &pluginName)
{
    QMutexLocker locker(&m_mutex);
    
    if (!m_loadedPlugins.contains(pluginName)) {
        return false;
    }
    
    QPluginLoader *loader = m_loadedPlugins[pluginName];
    loader->unload();
    delete loader;
    m_loadedPlugins.remove(pluginName);
    
    emit pluginUnloaded(pluginName);
    return true;
}

QStringList PluginSystem::getLoadedPlugins() const
{
    QMutexLocker locker(&m_mutex);
    return m_loadedPlugins.keys();
}

QStringList PluginSystem::getAvailablePlugins() const
{
    return scanForPlugins();
}

QJsonObject PluginSystem::getPluginInfo(const QString &pluginName) const
{
    QMutexLocker locker(&m_mutex);
    
    if (m_loadedPlugins.contains(pluginName)) {
        QPluginLoader *loader = m_loadedPlugins[pluginName];
        return extractPluginInfo(loader->fileName());
    }
    
    return QJsonObject();
}

QStringList PluginSystem::getPluginCapabilities(const QString &pluginName) const
{
    QStringList capabilities;
    
    QJsonObject info = getPluginInfo(pluginName);
    if (info.contains("capabilities")) {
        QJsonArray caps = info["capabilities"].toArray();
        for (const QJsonValue &cap : caps) {
            capabilities.append(cap.toString());
        }
    }
    
    return capabilities;
}

void PluginSystem::setPluginDirectory(const QString &directory)
{
    m_pluginDirectory = directory;
    QDir().mkpath(directory);
    refreshPluginList();
}

QString PluginSystem::getPluginDirectory() const
{
    return m_pluginDirectory;
}

void PluginSystem::refreshPluginList()
{
    emit pluginListUpdated();
}

QStringList PluginSystem::scanForPlugins() const
{
    QStringList plugins;
    QDir dir(m_pluginDirectory);
    
    if (!dir.exists()) {
        return plugins;
    }
    
    QFileInfoList files = dir.entryInfoList(QStringList() << "*.so" << "*.dll" << "*.dylib", QDir::Files);
    for (const QFileInfo &fileInfo : files) {
        if (validatePlugin(fileInfo.absoluteFilePath())) {
            plugins.append(fileInfo.absoluteFilePath());
        }
    }
    
    return plugins;
}

bool PluginSystem::validatePlugin(const QString &pluginPath) const
{
    QFileInfo fileInfo(pluginPath);
    return fileInfo.exists() && fileInfo.isFile() && fileInfo.size() > 0;
}

QJsonObject PluginSystem::extractPluginInfo(const QString &pluginPath) const
{
    QJsonObject info;
    
    QFileInfo fileInfo(pluginPath);
    info["name"] = fileInfo.baseName();
    info["path"] = pluginPath;
    info["size"] = static_cast<qint64>(fileInfo.size());
    info["lastModified"] = fileInfo.lastModified().toString(Qt::ISODate);
    
    return info;
}
