#ifndef WEBSEARCHCONFIGPAGE_H
#define WEBSEARCHCONFIGPAGE_H

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QSpinBox>
#include <QWidget>

#include "widgets/softcardwidget.h"

class QPushButton;
class WebSearchService;

class WebSearchConfigPage : public QWidget
{
    Q_OBJECT

public:
    explicit WebSearchConfigPage(QWidget *parent = nullptr);
    ~WebSearchConfigPage();

public slots:
    void refreshTheme();

private slots:
    void onSaveClicked();
    void onReloadClicked();
    void onTestSearchClicked();
    void onProviderChanged(int index);
    void onSearchFinished(const QString &requestId, const QString &query, int resultCount);
    void onSearchFailed(const QString &requestId, const QString &query, const QString &message);

private:
    void setupUi();
    void connectSignals();
    void applyTheme();
    void loadConfig();
    void updateStatusDisplay();

    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QLabel *m_titleLabel;

    SoftCardWidget *m_statusCard;
    QLabel *m_statusLabel;

    SoftCardWidget *m_settingsCard;
    QLabel *m_settingsCardTitle;
    QCheckBox *m_enabledCheckBox;
    QLabel *m_providerLabel;
    QComboBox *m_providerCombo;
    QLabel *m_apiKeyLabel;
    QLineEdit *m_apiKeyEdit;
    QLabel *m_resultCountLabel;
    QSpinBox *m_resultCountSpin;
    QLabel *m_searchDepthLabel;
    QComboBox *m_searchDepthCombo;
    QLabel *m_timeoutLabel;
    QSpinBox *m_timeoutSpin;

    QPushButton *m_saveButton;
    QPushButton *m_testButton;
    QPushButton *m_reloadButton;

    WebSearchService *m_testService;
    QString m_testRequestId;
    bool m_testPending;
};

#endif // WEBSEARCHCONFIGPAGE_H
