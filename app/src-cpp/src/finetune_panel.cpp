#include "finetune_panel.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QSpinBox>
#include <QSlider>
#include <QProgressBar>
#include <QTextEdit>
#include <QGroupBox>
#include <QFileDialog>
#include <QMessageBox>
#include <QCheckBox>
#include <QDebug>
#include <QDir>
#include <QTimer>

FineTunePanel::FineTunePanel(QWidget *parent)
    : QWidget(parent)
    , m_isTraining(false)
{
    setupUI();
    qDebug() << "FineTunePanel created";
}

FineTunePanel::~FineTunePanel() {
    qDebug() << "FineTunePanel destroyed";
}

void FineTunePanel::setupUI() {
    setStyleSheet("background: #1e1e1e; color: white;");
    
    m_mainLayout = new QVBoxLayout(this);
    m_mainLayout->setSpacing(20);
    m_mainLayout->setContentsMargins(20, 20, 20, 20);
    
    // Header
    m_headerLabel = new QLabel("🎓 Fine-Tune Models");
    m_headerLabel->setStyleSheet("font-size: 20px; font-weight: bold; color: #0078d4;");
    m_mainLayout->addWidget(m_headerLabel);
    
    QLabel *subtitle = new QLabel("Create custom models by fine-tuning on your data");
    subtitle->setStyleSheet("font-size: 14px; color: #888888; margin-bottom: 10px;");
    m_mainLayout->addWidget(subtitle);
    
    // File selection group
    m_filesGroup = new QGroupBox("📁 File Selection");
    m_filesGroup->setStyleSheet(
        "QGroupBox {"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "   border: 2px solid #3d3d3d;"
        "   border-radius: 5px;"
        "   margin-top: 10px;"
        "   padding-top: 10px;"
        "}"
        "QGroupBox::title {"
        "   subcontrol-origin: margin;"
        "   left: 10px;"
        "   padding: 0 5px 0 5px;"
        "}"
    );
    
    QGridLayout *filesLayout = new QGridLayout(m_filesGroup);
    
    // Base model selection
    filesLayout->addWidget(new QLabel("Base Model:"), 0, 0);
    m_baseModelEdit = new QLineEdit();
    m_baseModelEdit->setPlaceholderText("Select base model (.gguf)");
    m_baseModelEdit->setStyleSheet(
        "QLineEdit {"
        "   background: #2d2d2d;"
        "   border: 1px solid #3d3d3d;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   color: white;"
        "}"
    );
    filesLayout->addWidget(m_baseModelEdit, 0, 1);
    
    m_baseModelButton = new QPushButton("Browse...");
    m_baseModelButton->setStyleSheet(
        "QPushButton {"
        "   background: #0078d4;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 4px;"
        "   padding: 8px 16px;"
        "}"
        "QPushButton:hover { background: #106ebe; }"
    );
    connect(m_baseModelButton, &QPushButton::clicked, this, &FineTunePanel::onSelectBaseModel);
    filesLayout->addWidget(m_baseModelButton, 0, 2);
    
    // Training data selection
    filesLayout->addWidget(new QLabel("Training Data:"), 1, 0);
    m_trainingDataEdit = new QLineEdit();
    m_trainingDataEdit->setPlaceholderText("Select training data (.jsonl)");
    m_trainingDataEdit->setStyleSheet(m_baseModelEdit->styleSheet());
    filesLayout->addWidget(m_trainingDataEdit, 1, 1);
    
    m_trainingDataButton = new QPushButton("Browse...");
    m_trainingDataButton->setStyleSheet(m_baseModelButton->styleSheet());
    connect(m_trainingDataButton, &QPushButton::clicked, this, &FineTunePanel::onSelectTrainingData);
    filesLayout->addWidget(m_trainingDataButton, 1, 2);
    
    // Output path selection
    filesLayout->addWidget(new QLabel("Output Path:"), 2, 0);
    m_outputPathEdit = new QLineEdit();
    m_outputPathEdit->setPlaceholderText("Select output location");
    m_outputPathEdit->setStyleSheet(m_baseModelEdit->styleSheet());
    filesLayout->addWidget(m_outputPathEdit, 2, 1);
    
    m_outputPathButton = new QPushButton("Browse...");
    m_outputPathButton->setStyleSheet(m_baseModelButton->styleSheet());
    connect(m_outputPathButton, &QPushButton::clicked, this, &FineTunePanel::onSelectOutputPath);
    filesLayout->addWidget(m_outputPathButton, 2, 2);
    
    m_mainLayout->addWidget(m_filesGroup);
    
    // Training parameters group
    m_paramsGroup = new QGroupBox("⚙️ Training Parameters");
    m_paramsGroup->setStyleSheet(m_filesGroup->styleSheet());
    
    QGridLayout *paramsLayout = new QGridLayout(m_paramsGroup);
    
    // Epochs
    paramsLayout->addWidget(new QLabel("Epochs:"), 0, 0);
    m_epochsSpinBox = new QSpinBox();
    m_epochsSpinBox->setRange(1, 100);
    m_epochsSpinBox->setValue(3);
    m_epochsSpinBox->setStyleSheet(
        "QSpinBox {"
        "   background: #2d2d2d;"
        "   border: 1px solid #3d3d3d;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   color: white;"
        "}"
    );
    paramsLayout->addWidget(m_epochsSpinBox, 0, 1);
    
    // Batch size
    paramsLayout->addWidget(new QLabel("Batch Size:"), 1, 0);
    m_batchSizeSpinBox = new QSpinBox();
    m_batchSizeSpinBox->setRange(32, 2048);
    m_batchSizeSpinBox->setValue(512);
    m_batchSizeSpinBox->setStyleSheet(m_epochsSpinBox->styleSheet());
    paramsLayout->addWidget(m_batchSizeSpinBox, 1, 1);
    
    // Learning rate
    paramsLayout->addWidget(new QLabel("Learning Rate:"), 2, 0);
    m_learningRateSlider = new QSlider(Qt::Horizontal);
    m_learningRateSlider->setRange(1, 100);
    m_learningRateSlider->setValue(10); // 0.001
    m_learningRateSlider->setStyleSheet(
        "QSlider::groove:horizontal {"
        "   height: 8px;"
        "   background: #2d2d2d;"
        "   border-radius: 4px;"
        "}"
        "QSlider::handle:horizontal {"
        "   background: #0078d4;"
        "   width: 18px;"
        "   margin: -5px 0;"
        "   border-radius: 9px;"
        "}"
    );
    paramsLayout->addWidget(m_learningRateSlider, 2, 1);
    
    m_learningRateLabel = new QLabel("0.001");
    m_learningRateLabel->setStyleSheet("color: #888888;");
    paramsLayout->addWidget(m_learningRateLabel, 2, 2);
    
    connect(m_learningRateSlider, &QSlider::valueChanged, [this](int value) {
        float rate = value / 10000.0f;
        m_learningRateLabel->setText(QString::number(rate, 'f', 4));
    });
    
    m_mainLayout->addWidget(m_paramsGroup);
    
    // Training control group
    m_controlGroup = new QGroupBox("🚀 Training Control");
    m_controlGroup->setStyleSheet(m_filesGroup->styleSheet());
    
    QVBoxLayout *controlLayout = new QVBoxLayout(m_controlGroup);
    
    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    
    m_startButton = new QPushButton("▶️ Start Training");
    m_startButton->setStyleSheet(
        "QPushButton {"
        "   background: #28a745;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 6px;"
        "   padding: 12px 24px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background: #218838; }"
        "QPushButton:disabled {"
        "   background: #3d3d3d;"
        "   color: #666666;"
        "}"
    );
    connect(m_startButton, &QPushButton::clicked, this, &FineTunePanel::onStartTraining);
    buttonLayout->addWidget(m_startButton);
    
    m_stopButton = new QPushButton("⏹️ Stop Training");
    m_stopButton->setEnabled(false);
    m_stopButton->setStyleSheet(
        "QPushButton {"
        "   background: #dc3545;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 6px;"
        "   padding: 12px 24px;"
        "   font-size: 14px;"
        "   font-weight: bold;"
        "}"
        "QPushButton:hover { background: #c82333; }"
        "QPushButton:disabled {"
        "   background: #3d3d3d;"
        "   color: #666666;"
        "}"
    );
    connect(m_stopButton, &QPushButton::clicked, this, &FineTunePanel::onStopTraining);
    buttonLayout->addWidget(m_stopButton);
    
    buttonLayout->addStretch();
    controlLayout->addLayout(buttonLayout);
    
    // Progress bar
    m_progressBar = new QProgressBar();
    m_progressBar->setStyleSheet(
        "QProgressBar {"
        "   background: #2d2d2d;"
        "   border: 1px solid #3d3d3d;"
        "   border-radius: 4px;"
        "   height: 20px;"
        "   text-align: center;"
        "}"
        "QProgressBar::chunk {"
        "   background: #0078d4;"
        "   border-radius: 4px;"
        "}"
    );
    controlLayout->addWidget(m_progressBar);
    
    // Status label
    m_statusLabel = new QLabel("Ready to start training");
    m_statusLabel->setStyleSheet("color: #888888; font-size: 12px;");
    controlLayout->addWidget(m_statusLabel);
    
    // Log display
    m_logDisplay = new QTextEdit();
    m_logDisplay->setMaximumHeight(120);
    m_logDisplay->setStyleSheet(
        "QTextEdit {"
        "   background: #0d1117;"
        "   color: #ffffff;"
        "   border: 1px solid #30363d;"
        "   border-radius: 4px;"
        "   padding: 8px;"
        "   font-family: 'Consolas', 'Monaco', monospace;"
        "   font-size: 11px;"
        "}"
    );
    m_logDisplay->append("Fine-tuning panel initialized. Select files to begin.");
    controlLayout->addWidget(m_logDisplay);
    
    m_mainLayout->addWidget(m_controlGroup);
    
    m_mainLayout->addStretch();
}

void FineTunePanel::onSelectBaseModel() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select Base Model",
        "models/",
        "GGUF Models (*.gguf);;All Files (*)"
    );
    
    if (!fileName.isEmpty()) {
        m_baseModelEdit->setText(fileName);
        m_logDisplay->append(QString("Base model selected: %1").arg(QFileInfo(fileName).fileName()));
    }
}

void FineTunePanel::onSelectTrainingData() {
    QString fileName = QFileDialog::getOpenFileName(
        this,
        "Select Training Data",
        ".",
        "JSONL Files (*.jsonl);;JSON Files (*.json);;All Files (*)"
    );
    
    if (!fileName.isEmpty()) {
        m_trainingDataEdit->setText(fileName);
        m_logDisplay->append(QString("Training data selected: %1").arg(QFileInfo(fileName).fileName()));
    }
}

void FineTunePanel::onSelectOutputPath() {
    QString fileName = QFileDialog::getSaveFileName(
        this,
        "Select Output Location",
        "models/fine_tuned_model.gguf",
        "GGUF Models (*.gguf);;All Files (*)"
    );
    
    if (!fileName.isEmpty()) {
        m_outputPathEdit->setText(fileName);
        m_logDisplay->append(QString("Output path set: %1").arg(QFileInfo(fileName).fileName()));
    }
}

void FineTunePanel::onStartTraining() {
    // Basic validation
    if (m_baseModelEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Base Model", "Please select a base model.");
        return;
    }
    
    if (m_trainingDataEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Training Data", "Please select training data.");
        return;
    }
    
    if (m_outputPathEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Missing Output Path", "Please specify an output path.");
        return;
    }
    
    m_isTraining = true;
    m_startButton->setEnabled(false);
    m_stopButton->setEnabled(true);
    
    updateTrainingStatus("Starting training...");
    m_logDisplay->append("=====================================");
    m_logDisplay->append("Training started");
    m_logDisplay->append(QString("Base model: %1").arg(m_baseModelEdit->text()));
    m_logDisplay->append(QString("Training data: %1").arg(m_trainingDataEdit->text()));
    m_logDisplay->append(QString("Output: %1").arg(m_outputPathEdit->text()));
    m_logDisplay->append(QString("Epochs: %1").arg(m_epochsSpinBox->value()));
    m_logDisplay->append(QString("Batch size: %1").arg(m_batchSizeSpinBox->value()));
    m_logDisplay->append("=====================================");
    
    // Simulate training progress (this will be replaced with actual training integration)
    QTimer *progressTimer = new QTimer(this);
    connect(progressTimer, &QTimer::timeout, [this, progressTimer]() {
        static int progress = 0;
        progress += 5;
        m_progressBar->setValue(progress);
        
        if (progress >= 100) {
            progressTimer->stop();
            progressTimer->deleteLater();
            
            // Training completed
            m_isTraining = false;
            m_startButton->setEnabled(true);
            m_stopButton->setEnabled(false);
            
            updateTrainingStatus("Training completed successfully!");
            m_logDisplay->append("Training completed!");
            m_logDisplay->append("Model saved to: " + m_outputPathEdit->text());
            
            // Store the path for emission
            m_currentModelPath = m_outputPathEdit->text();
            
            // Emit signal that training is complete
            emit modelFineTuned(m_currentModelPath);
            
            // Show success message
            QMessageBox::information(this, "Training Complete", 
                                   "Fine-tuning completed successfully!\n\nModel saved to:\n" + m_currentModelPath);
        } else {
            updateTrainingStatus(QString("Training in progress... %1%").arg(progress));
        }
    });
    
    progressTimer->start(1000); // Update every second for demo
}

void FineTunePanel::onStopTraining() {
    if (m_isTraining) {
        m_isTraining = false;
        m_startButton->setEnabled(true);
        m_stopButton->setEnabled(false);
        
        updateTrainingStatus("Training stopped by user");
        m_logDisplay->append("Training stopped by user.");
        m_progressBar->setValue(0);
    }
}

void FineTunePanel::updateTrainingStatus(const QString &status) {
    m_statusLabel->setText(status);
}
