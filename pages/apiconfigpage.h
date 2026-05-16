#ifndef APICONFIGPAGE_H
#define APICONFIGPAGE_H

#include <QLabel>
#include <QListWidget>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

#include <QPointer>

#include "models/apiconfig.h"
#include "widgets/softcardwidget.h"

class ApiConfigDialog;
class QVBoxLayout;

class ApiConfigPage : public QWidget
{
    Q_OBJECT

public:
    explicit ApiConfigPage(QWidget *parent = nullptr);
    ~ApiConfigPage();

public slots:
    void refreshTheme();

private:
    void setupUi();
    void connectSignals();
    void applyTheme();
    void updateEmptyState();

    void refreshProfileList();
    QWidget *createProfileRowWidget(const QString &profileName);
    void updateCurrentProfileDisplay();
    QIcon tintedIcon(const QString &path, const QColor &color) const;

    void setupChatSettingsSection(QVBoxLayout *parentLayout);
    void loadChatSettingsToUi();
    void saveChatSettingsFromUi();
    void resetSystemPromptToDefault();

private slots:
    void onAddApiProfile();
    void onEditApiProfile(int row);
    void onRemoveApiProfile(int row);
    void onApiProfileSelectionChanged();
    void onDialogSubmitted(const QString &profileName, const ApiConfig &config);

private:
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QLabel *m_titleLabel;
    SoftCardWidget *m_statusCard;
    SoftCardWidget *m_profilesCard;
    QLabel *m_profilesCardTitle;
    QLabel *m_currentApiProfileLabel;

    QListWidget *m_apiProfileList;
    QPushButton *m_addApiProfileButton;
    QLabel *m_emptyLabel;

    QPointer<ApiConfigDialog> m_configDialog;
    QString m_editingProfileName;

    SoftCardWidget *m_chatSettingsCard;
    QLabel *m_chatSettingsTitleLabel;
    QPlainTextEdit *m_systemPromptEdit;
    QPushButton *m_resetPromptButton;
    QPushButton *m_savePromptButton;
};

#endif // APICONFIGPAGE_H
