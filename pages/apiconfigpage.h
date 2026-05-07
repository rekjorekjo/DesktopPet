#ifndef APICONFIGPAGE_H
#define APICONFIGPAGE_H

#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QMap>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

#include "widgets/softcardwidget.h"

struct ApiConfig
{
    QString apiKey;
    QString baseUrl;
    QString model;
};

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

    void loadApiConfigToEditor(const QString &profileName);
    void clearApiEditor();
    void saveCurrentApiConfig();

private slots:
    void onAddApiProfile();
    void onRemoveApiProfile();
    void onApiProfileSelectionChanged();
    void onSaveApiConfigClicked();

private:
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QLabel *m_titleLabel;
    SoftCardWidget *m_statusCard;
    SoftCardWidget *m_editorCard;
    SoftCardWidget *m_profilesCard;
    QLabel *m_editorCardTitle;
    QLabel *m_profilesCardTitle;
    QLabel *m_apiKeyLabel;
    QLabel *m_baseUrlLabel;
    QLabel *m_modelLabel;
    QLineEdit *m_apiKeyEdit;
    QLineEdit *m_baseUrlEdit;
    QLineEdit *m_modelEdit;
    QPushButton *m_saveApiConfigButton;

    QListWidget *m_apiProfileList;
    QPushButton *m_addApiProfileButton;
    QPushButton *m_removeApiProfileButton;
    QLabel *m_currentApiProfileLabel;

    QMap<QString, ApiConfig> m_apiConfigs;
    QString m_currentApiProfile;
};

#endif // APICONFIGPAGE_H
