#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QComboBox>
#include <QProgressBar>
#include <QTabWidget>
#include <QStackedWidget>
#include <QToolButton>
#include <QFrame>
#include <QScrollArea>
#include <QGridLayout>
#include <QMap>
#include <QDockWidget>
#include <QCheckBox>
#include "backend.h"
#include "modelcard.h"
#include "downloadpanel.h"
#include "gpu_detector.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onSendMessage();
    void onModelSelected(const QString &modelName);
    void onRefreshModels();
    void onDownloadModelRequested(const QString &modelName);
    void onRemoveModel();
    void onModelSearchChanged(const QString &text);
    void onPageChanged(const QString &page);
    void onBrowseModelsClicked();
    void onNewChatClicked();
    void onAboutClicked();
    void showModelDetails(QListWidgetItem *item);
    void onModelDownloadProgress(const QString &modelName, double progress);
    void onModelDownloadComplete(const QString &modelName);
    void onAddCustomModel();

private:
    // Backend
    Backend *m_backend;
    
    // Main layout
    QWidget *m_centralWidget;
    QHBoxLayout *m_mainLayout;
    
    // Sidebar
    QFrame *m_sidebar;
    QVBoxLayout *m_sidebarLayout;
    QLabel *m_brandingLabel;
    QPushButton *m_newChatBtn;
    QPushButton *m_chatPageBtn;
    QPushButton *m_downloadPageBtn;
    QPushButton *m_installedPageBtn;
    QPushButton *m_customPageBtn;
    QPushButton *m_aboutBtn;
    
    // Main content area
    QStackedWidget *m_contentStack;
    
    // Chat page
    QWidget *m_chatPage;
    QTextEdit *m_chatDisplay;
    QLineEdit *m_chatInput;
    QPushButton *m_sendButton;
    QComboBox *m_modelSelector;
    QPushButton *m_modelSelectorBtn;
    QLabel *m_statusLabel;
    
    // Browse/Download models page
    QWidget *m_downloadPage;
    QLineEdit *m_downloadSearch;
    QComboBox *m_taskTypeFilter;
    QComboBox *m_sortByCombo;
    QScrollArea *m_modelsScrollArea;
    QWidget *m_modelsContainer;
    QGridLayout *m_modelsGrid;
    QLabel *m_downloadCountLabel;
    QMap<QString, ModelCard*> m_modelCards;
    
    // Installed models page
    QWidget *m_installedPage;
    QLineEdit *m_installedSearch;
    QListWidget *m_installedModelsList;
    QTextEdit *m_installedModelDetails;
    QPushButton *m_removeModelBtn;
    QPushButton *m_useModelBtn;
    QLabel *m_installedCountLabel;
    
    // Custom models page
    QWidget *m_customPage;
    QLineEdit *m_customModelPath;
    QPushButton *m_browseCustomBtn;
    QLineEdit *m_customModelName;
    QComboBox *m_customModelFormat;
    QTextEdit *m_customModelConfig;
    QPushButton *m_addCustomModelBtn;
    QListWidget *m_customModelsList;
    
    // Download panel
    QDockWidget *m_downloadDock;
    DownloadPanel *m_downloadPanel;
    
    // GPU detection
    GPUDetector *m_gpuDetector;
    qint64 m_detectedVRAM;
    QCheckBox *m_gpuFilterCheckbox;
    QLabel *m_gpuInfoLabel;
    
    // Helper methods
    void setupUI();
    void setupSidebar();
    void setupChatPage();
    void setupDownloadPage();
    void setupInstalledPage();
    void setupCustomPage();
    void setupDownloadDock();
    void applyModernStyling();
    void loadAvailableModels();
    void loadInstalledModels();
    void updateModelSelector();
    void appendChatMessage(const QString &role, const QString &message);
    void switchToPage(int index);
    QPushButton* createSidebarButton(const QString &icon, const QString &text);
    
    // Current state
    QString m_currentModel;
    QList<QJsonObject> m_availableModels;
    QList<QJsonObject> m_installedModels;
};

#endif // MAINWINDOW_H
