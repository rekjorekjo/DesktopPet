#ifndef PETCHATWIDGET_H
#define PETCHATWIDGET_H

#include "services/chatcompletionservice.h"

#include <QList>
#include <QPlainTextEdit>
#include <QStackedWidget>
#include <QWidget>

class EmptyStateWidget;

class PetChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PetChatWidget(QWidget *parent = nullptr);
    ~PetChatWidget();

    void setPetName(const QString &name);
    void setPetDisplayName(const QString &name);
    void setApiConfigName(const QString &name);
    void setApiInfo(const QString &apiConfigName, const QString &model);

    void appendUserMessage(const QString &content);
    void appendAiMessage(const QString &content);
    void appendSystemMessage(const QString &content);

    void clearMessages();

    void focusInput();

public slots:
    void applyTheme();

signals:
    void messageSubmitted(const QString &message);

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUi();
    void submitMessage();
    void showEmptyState();
    void hideEmptyState();
    void setWaitingState(bool waiting);
    QString currentSystemPrompt() const;

private slots:
    void onRequestFinished(const QString &requestId, const QString &content);
    void onRequestFailed(const QString &requestId, const QString &errorMessage);

private:
    QStackedWidget *m_messageStack;
    EmptyStateWidget *m_emptyState;
    QPlainTextEdit *m_messageDisplay;
    QPlainTextEdit *m_inputEdit;

    QString m_petName;
    QString m_petDisplayName;
    QString m_apiConfigName;
    QString m_model;
    bool m_hasMessages;

    ChatCompletionService *m_chatService;
    QList<ChatCompletionService::Message> m_messages;
    QString m_pendingRequestId;
    bool m_requestPending;
};

#endif // PETCHATWIDGET_H
