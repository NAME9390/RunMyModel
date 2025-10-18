#ifndef FINETUNE_PANEL_H
#define FINETUNE_PANEL_H

#include <QWidget>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTextEdit>
#include <QProgressBar>
#include <QLineEdit>
#include <QSpinBox>
#include <QSlider>
#include <QCheckBox>
#include <QGroupBox>
#include <QFileDialog>
#include <QListWidget>

/**
 * @brief Fine-tuning panel for training custom models
 * 
 * This is a basic implementation that provides the interface expected by MainWindow.
 * It will be enhanced with the full Training Wizard functionality in future iterations.
 */
class FineTunePanel : public QWidget {
    Q_OBJECT

public:
    explicit FineTunePanel(QWidget *parent = nullptr);
    ~FineTunePanel();

signals:
    void modelFineTuned(const QString &modelPath);

private slots:
    void onSelectTrainingData();
    void onSelectBaseModel();
    void onSelectOutputPath();
    void onStartTraining();
    void onStopTraining();

private:
    void setupUI();
    void updateTrainingStatus(const QString &status);
    
    // UI components
    QVBoxLayout *m_mainLayout;
    QLabel *m_headerLabel;
    
    // File selection
    QGroupBox *m_filesGroup;
    QLineEdit *m_baseModelEdit;
    QPushButton *m_baseModelButton;
    QLineEdit *m_trainingDataEdit;
    QPushButton *m_trainingDataButton;
    QLineEdit *m_outputPathEdit;
    QPushButton *m_outputPathButton;
    
    // Training parameters
    QGroupBox *m_paramsGroup;
    QSpinBox *m_epochsSpinBox;
    QSpinBox *m_batchSizeSpinBox;
    QSlider *m_learningRateSlider;
    QLabel *m_learningRateLabel;
    
    // Training control
    QGroupBox *m_controlGroup;
    QPushButton *m_startButton;
    QPushButton *m_stopButton;
    QProgressBar *m_progressBar;
    QLabel *m_statusLabel;
    QTextEdit *m_logDisplay;
    
    // State
    bool m_isTraining;
    QString m_currentModelPath;
};

#endif // FINETUNE_PANEL_H
