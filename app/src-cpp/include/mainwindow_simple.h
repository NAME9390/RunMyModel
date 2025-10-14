#pragma once
#include <QMainWindow>
#include <QTextEdit>
#include <QLineEdit>

class LlamaEngine;

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

private:
    QTextEdit *m_chatDisplay;
    QLineEdit *m_messageInput;
    LlamaEngine *m_llamaEngine;
    QString m_currentResponse;
};

