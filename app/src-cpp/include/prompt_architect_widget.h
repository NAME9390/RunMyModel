#ifndef PROMPT_ARCHITECT_WIDGET_H
#define PROMPT_ARCHITECT_WIDGET_H

#include <QWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QListWidget>
#include <QTextEdit>
#include <QPushButton>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QGroupBox>
#include <QJsonArray>
#include <QJsonObject>
#include <QMap>

class BackendClient;
class PromptBlockWidget;

/**
 * PromptArchitectWidget - Visual prompt builder UI
 * 
 * Open WebUI-inspired interface for creating prompts from blocks
 */
class PromptArchitectWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PromptArchitectWidget(BackendClient *backendClient, QWidget *parent = nullptr);
    ~PromptArchitectWidget();

signals:
    void promptBuilt(const QString &prompt);
    void templateSelected(const QString &templateId);

private slots:
    void onTemplateSelected(int index);
    void onAddBlockClicked();
    void onRemoveBlockClicked();
    void onBuildPromptClicked();
    void onSavePromptClicked();
    void onLoadPromptClicked();
    void onBlockContentChanged();
    void onVariableChanged();
    void onTemplatesLoaded(const QJsonArray &templates);

private:
    void setupUI();
    void applyModernStyling();
    void loadTemplates();
    void createNewBlock(const QString &blockType);
    void updatePreview();

    BackendClient *m_backendClient;
    QString m_sessionId;

    // UI Elements
    QComboBox *m_templateCombo;
    QPushButton *m_loadTemplateBtn;
    
    // Block management
    QVBoxLayout *m_blocksLayout;
    QWidget *m_blocksContainer;
    QList<PromptBlockWidget*> m_blockWidgets;
    
    QComboBox *m_blockTypeCombo;
    QPushButton *m_addBlockBtn;
    
    // Variables
    QVBoxLayout *m_variablesLayout;
    QWidget *m_variablesContainer;
    QMap<QString, QLineEdit*> m_variableInputs;
    
    // Preview
    QTextEdit *m_previewText;
    
    // Actions
    QPushButton *m_buildBtn;
    QPushButton *m_saveBtn;
    QPushButton *m_loadBtn;
    QPushButton *m_useInChatBtn;
    
    QJsonArray m_availableTemplates;
};

/**
 * Individual prompt block widget
 */
class PromptBlockWidget : public QGroupBox
{
    Q_OBJECT

public:
    explicit PromptBlockWidget(const QString &blockType, const QString &blockId, QWidget *parent = nullptr);
    
    QString blockId() const { return m_blockId; }
    QString blockType() const { return m_blockType; }
    QString content() const;
    
    void setContent(const QString &content);

signals:
    void contentChanged(const QString &blockId);
    void removeRequested(const QString &blockId);

private:
    void setupUI();
    
    QString m_blockId;
    QString m_blockType;
    QTextEdit *m_contentEdit;
    QPushButton *m_removeBtn;
};

#endif // PROMPT_ARCHITECT_WIDGET_H

