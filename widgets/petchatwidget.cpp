#include "petchatwidget.h"

#include "theme/thememanager.h"

#include <QHBoxLayout>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QScrollArea>
#include <QVBoxLayout>

PetChatWidget::PetChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_petNameLabel(nullptr)
    , m_apiConfigLabel(nullptr)
    , m_closeButton(nullptr)
    , m_messageDisplay(nullptr)
    , m_inputEdit(nullptr)
    , m_sendButton(nullptr)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, false);

    setMinimumWidth(260);
    setMinimumHeight(180);

    setupUi();
    connectSignals();
    applyTheme();
}

PetChatWidget::~PetChatWidget() {}

void PetChatWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 10, 12, 12);
    mainLayout->setSpacing(8);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(8);

    m_petNameLabel = new QLabel(this);
    m_petNameLabel->setText(tr("宠物"));
    QFont petNameFont = m_petNameLabel->font();
    petNameFont.setBold(true);
    petNameFont.setPointSize(10);
    m_petNameLabel->setFont(petNameFont);

    m_apiConfigLabel = new QLabel(this);
    m_apiConfigLabel->setText(tr("未选择 API"));
    m_apiConfigLabel->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

    m_closeButton = new QPushButton(this);
    m_closeButton->setText("×");
    m_closeButton->setFixedSize(24, 24);
    m_closeButton->setCursor(Qt::PointingHandCursor);

    headerLayout->addWidget(m_petNameLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(m_apiConfigLabel);
    headerLayout->addWidget(m_closeButton);

    mainLayout->addLayout(headerLayout);

    m_messageDisplay = new QPlainTextEdit(this);
    m_messageDisplay->setReadOnly(true);
    m_messageDisplay->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_messageDisplay->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_messageDisplay->setPlainText(tr("点击下方输入内容，AI Chat 将在后续版本接入。"));
    m_messageDisplay->setMinimumHeight(80);

    mainLayout->addWidget(m_messageDisplay, 1);

    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(8);

    m_inputEdit = new QPlainTextEdit(this);
    m_inputEdit->setMaximumHeight(60);
    m_inputEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_inputEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_inputEdit->setPlaceholderText(tr("输入消息..."));
    m_inputEdit->installEventFilter(this);

    m_sendButton = new QPushButton(QStringLiteral("➤"), this);
    m_sendButton->setToolTip(tr("发送"));
    m_sendButton->setAccessibleName(tr("发送"));
    m_sendButton->setFixedSize(44, 44);
    m_sendButton->setCursor(Qt::PointingHandCursor);
    QFont sendFont = m_sendButton->font();
    sendFont.setPointSize(16);
    sendFont.setBold(true);
    m_sendButton->setFont(sendFont);

    inputLayout->addWidget(m_inputEdit, 1);
    inputLayout->addWidget(m_sendButton);

    mainLayout->addLayout(inputLayout);
}

void PetChatWidget::connectSignals()
{
    connect(m_closeButton, &QPushButton::clicked, this, &PetChatWidget::closeRequested);
    connect(m_sendButton, &QPushButton::clicked, this, &PetChatWidget::onSendClicked);
}

void PetChatWidget::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    QString cardStyle = QString(
        "QPlainTextEdit { background: %1; color: %2; border: 1px solid %3; "
        "border-radius: 6px; padding: 8px; font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; }"
        "QPlainTextEdit:focus { border: 1px solid %4; }"
    ).arg(p.inputBackground, p.textPrimary, p.border, p.accent);

    m_petNameLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(p.textPrimary));

    m_apiConfigLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(p.textSecondary));

    m_closeButton->setStyleSheet(QString(
        "QPushButton { background: transparent; color: %1; border: none; border-radius: 12px; }"
        "QPushButton:hover { background: %2; color: %3; }"
    ).arg(p.textSecondary, p.accent, p.checkboxCheckColor));

    m_messageDisplay->setStyleSheet(cardStyle);

    m_inputEdit->setStyleSheet(cardStyle);

    m_sendButton->setStyleSheet(theme.softButtonStyleSheet(8, 44));
}

void PetChatWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    constexpr int borderRadius = 12;

    QPainterPath path;
    path.addRoundedRect(rect(), borderRadius, borderRadius);

    QColor bgColor(p.cardBackground);
    bgColor.setAlpha(245);
    painter.fillPath(path, bgColor);

    QColor borderColor(p.border);
    borderColor.setAlpha(150);
    QPen borderPen(borderColor, 1);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
}

bool PetChatWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_inputEdit && event->type() == QEvent::KeyPress) {
        auto *keyEvent = static_cast<QKeyEvent *>(event);
        if (keyEvent->key() == Qt::Key_Return || keyEvent->key() == Qt::Key_Enter) {
            if (keyEvent->modifiers() & Qt::ShiftModifier) {
                return false;
            }
            onSendClicked();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void PetChatWidget::focusInput()
{
    if (m_inputEdit) {
        m_inputEdit->setFocus();
    }
}

void PetChatWidget::setPetName(const QString &name)
{
    m_petName = name;
    m_petNameLabel->setText(name.isEmpty() ? tr("宠物") : name);
}

void PetChatWidget::setApiConfigName(const QString &name)
{
    m_apiConfigName = name;
    m_apiConfigLabel->setText(name.isEmpty() ? tr("未选择 API") : name);
}

void PetChatWidget::appendUserMessage(const QString &content)
{
    QString currentText = m_messageDisplay->toPlainText();
    if (currentText.contains(tr("点击下方输入内容，AI Chat 将在后续版本接入。"))) {
        currentText.clear();
    }

    QString newMessage = tr("用户：\n%1\n").arg(content);
    m_messageDisplay->setPlainText(currentText + newMessage);

    QTextCursor cursor = m_messageDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_messageDisplay->setTextCursor(cursor);
}

void PetChatWidget::appendAiMessage(const QString &content)
{
    QString currentText = m_messageDisplay->toPlainText();
    QString newMessage = tr("\nAI：\n%1\n").arg(content);
    m_messageDisplay->setPlainText(currentText + newMessage);

    QTextCursor cursor = m_messageDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_messageDisplay->setTextCursor(cursor);
}

void PetChatWidget::appendSystemMessage(const QString &content)
{
    QString currentText = m_messageDisplay->toPlainText();
    QString newMessage = tr("\n系统：\n%1\n").arg(content);
    m_messageDisplay->setPlainText(currentText + newMessage);

    QTextCursor cursor = m_messageDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_messageDisplay->setTextCursor(cursor);
}

void PetChatWidget::clearMessages()
{
    m_messageDisplay->setPlainText(tr("点击下方输入内容，AI Chat 将在后续版本接入。"));
}

void PetChatWidget::onSendClicked()
{
    QString content = m_inputEdit->toPlainText().trimmed();
    if (content.isEmpty()) {
        return;
    }

    appendUserMessage(content);
    m_inputEdit->clear();

    appendSystemMessage(tr("AI Chat 接口将在后续版本接入。"));

    emit messageSubmitted(content);
}
