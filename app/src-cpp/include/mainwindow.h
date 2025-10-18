#pragma once

#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>
#include <QComboBox>
#include <QSlider>
#include <QSpinBox>
#include <QTabWidget>
#include <QListWidget>
#include <QProgressBar>
#include <QTime>
#include "llama_engine.h"
#include "finetune_panel.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void sendMessage();
    void onTokenReceived(const QString &token);
    void onResponseComplete();
    void onError(const QString &error);
    void onStopGeneration();
    void onClearChat();
    void onSaveChat();
    void onLoadModel();
    void onUnloadModel();
    void onTemperatureChanged(int value);
    void onMaxTokensChanged(int value);

private:
    void setupUI();
    void createChatTab();
    void createSettingsTab();
    void createModelsTab();
    void createFineTuneTab();
    void appendMessage(const QString &message, const QString &sender);
    void updateStats();
    void loadAvailableModels();
    void onModelFineTuned(const QString &modelPath);

    LlamaEngine *m_llamaEngine;
    FineTunePanel *m_fineTunePanel;
    
    // UI elements - Chat Tab
    QTabWidget *m_tabWidget;
    QTextEdit *m_chatDisplay;
    QLineEdit *m_messageInput;
    QPushButton *m_sendButton;
    QPushButton *m_stopButton;
    QPushButton *m_clearButton;
    QPushButton *m_saveButton;
    QLabel *m_statusLabel;
    QLabel *m_statsLabel;
    
    // UI elements - Settings Tab
    QSlider *m_temperatureSlider;
    QSpinBox *m_maxTokensSpinBox;
    QLabel *m_temperatureLabel;
    
    // UI elements - Models Tab
    QListWidget *m_modelsList;
    QPushButton *m_loadModelButton;
    QPushButton *m_unloadModelButton;
    QLabel *m_currentModelLabel;
    QProgressBar *m_modelLoadProgress;
    
    // State
    QString m_currentResponse;
    bool m_isGenerating;
    int m_tokenCount;
    QTime m_generationStartTime;
    
    // Settings
    float m_temperature;
    int m_maxTokens;
    QString m_currentModelPath;
};
