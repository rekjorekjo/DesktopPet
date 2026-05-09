#ifndef PETCHATWIDGET_H
#define PETCHATWIDGET_H

#include <QLabel>
#include <QPlainTextEdit>
#include <QPushButton>
#include <QWidget>

class PetChatWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PetChatWidget(QWidget *parent = nullptr);
    ~PetChatWidget();

    void setPetName(const QString &name);
    void setApiConfigName(const QString &name);

    void appendUserMessage(const QString &content);
    void appendAiMessage(const QString &content);
    void appendSystemMessage(const QString &content);

    void clearMessages();

    void focusInput();

public slots:
    void applyTheme();

signals:
    void messageSubmitted(const QString &message);
    void closeRequested();

protected:
    void paintEvent(QPaintEvent *event) override;
    bool eventFilter(QObject *watched, QEvent *event) override;

private:
    void setupUi();
    void connectSignals();
    void onSendClicked();

    QLabel *m_petNameLabel;
    QLabel *m_apiConfigLabel;
    QPushButton *m_closeButton;

    QPlainTextEdit *m_messageDisplay;
    QPlainTextEdit *m_inputEdit;
    QPushButton *m_sendButton;

    QString m_petName;
    QString m_apiConfigName;
};

#endif // PETCHATWIDGET_H
