#ifndef LOGPAGE_H
#define LOGPAGE_H

#include <QLabel>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

#include "services/chatlogservice.h"
#include "widgets/softcardwidget.h"

class LogPage : public QWidget
{
    Q_OBJECT

public:
    explicit LogPage(QWidget *parent = nullptr);
    ~LogPage();

public slots:
    void refreshTheme();

private slots:
    void onLoadLogClicked();
    void onSearchTextChanged(const QString &text);

private:
    void setupUi();
    void connectSignals();
    void applyTheme();
    void displayEntries(const QList<ChatLogEntry> &entries);
    void showPlaceholder(const QString &message);

    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QLabel *m_titleLabel;

    SoftCardWidget *m_logCard;

    QPushButton *m_loadLogButton;
    QLineEdit *m_filePathEdit;
    QLineEdit *m_searchEdit;

    QPlainTextEdit *m_logDisplay;

    QString m_currentFilePath;
    QList<ChatLogEntry> m_currentEntries;
};

#endif // LOGPAGE_H
