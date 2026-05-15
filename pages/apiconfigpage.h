#ifndef APICONFIGPAGE_H
#define APICONFIGPAGE_H

#include <QLabel>
#include <QListWidget>
#include <QMap>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

#include "models/apiconfig.h"
#include "widgets/softcardwidget.h"

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

private slots:
    void onAddApiProfile();
    void onEditApiProfile(int row);
    void onRemoveApiProfile(int row);
    void onApiProfileSelectionChanged();

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

    QMap<QString, ApiConfig> m_apiConfigs;
    QString m_currentApiProfile;
};

#endif // APICONFIGPAGE_H
