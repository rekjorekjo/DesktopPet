#include "petchatwidget.h"

#include "theme/thememanager.h"
#include "widgets/emptystatewidget.h"

#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

PetChatWidget::PetChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_messageStack(nullptr)
    , m_emptyState(nullptr)
    , m_messageDisplay(nullptr)
    , m_inputEdit(nullptr)
    , m_hasMessages(false)
{
    setWindowFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_ShowWithoutActivating, false);

    setMinimumWidth(260);
    setMinimumHeight(180);

    setupUi();
    applyTheme();
}

PetChatWidget::~PetChatWidget() {}

void PetChatWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(8);

    m_messageStack = new QStackedWidget(this);

    m_emptyState = new EmptyStateWidget(m_messageStack);
    m_emptyState->setTitle(tr("对话"));
    m_messageStack->addWidget(m_emptyState);

    m_messageDisplay = new QPlainTextEdit(m_messageStack);
    m_messageDisplay->setReadOnly(true);
    m_messageDisplay->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_messageDisplay->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_messageDisplay->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_messageStack->addWidget(m_messageDisplay);

    m_messageStack->setCurrentWidget(m_emptyState);
    mainLayout->addWidget(m_messageStack, 1);

    m_inputEdit = new QPlainTextEdit(this);
    m_inputEdit->setMinimumHeight(48);
    m_inputEdit->setMaximumHeight(80);
    m_inputEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_inputEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_inputEdit->setPlaceholderText(tr("输入消息，按 Enter 发送，Shift+Enter 换行..."));
    m_inputEdit->installEventFilter(this);

    mainLayout->addWidget(m_inputEdit);

    showEmptyState();
}

void PetChatWidget::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    QString inputStyle = QString(
        "QPlainTextEdit { background: %1; color: %2; border: 1px solid %3; "
        "border-radius: 8px; padding: 10px; font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; }"
        "QPlainTextEdit:focus { border: 1px solid %4; }"
    ).arg(p.inputBackground, p.textPrimary, p.border, p.accent);

    QString messageStyle = QString(
        "QPlainTextEdit { background: %1; color: %2; border: none; "
        "border-radius: 8px; padding: 10px; font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; }"
    ).arg(p.inputBackground, p.textPrimary);

    m_inputEdit->setStyleSheet(inputStyle);
    m_messageDisplay->setStyleSheet(messageStyle);

    if (m_emptyState) {
        m_emptyState->applyTheme();
    }
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
            submitMessage();
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}

void PetChatWidget::showEmptyState()
{
    if (!m_emptyState) {
        return;
    }

    QString apiName = m_apiConfigName.isEmpty() ? tr("未选择 API") : m_apiConfigName;
    QString modelName = m_model.isEmpty() ? tr("未选择模型") : m_model;

    m_emptyState->setTitle(tr("和宠物对话"));
    m_emptyState->setDescription(
        QString("%1: %2\n%3: %4")
            .arg(tr("当前 API"), apiName, tr("当前模型"), modelName));
    m_emptyState->setContent(tr("输入消息开始对话..."));

    m_messageStack->setCurrentWidget(m_emptyState);
}

void PetChatWidget::hideEmptyState()
{
    if (!m_hasMessages) {
        m_hasMessages = true;
        m_messageDisplay->clear();
        m_messageStack->setCurrentWidget(m_messageDisplay);
    }
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
}

void PetChatWidget::setApiConfigName(const QString &name)
{
    m_apiConfigName = name;
    if (!m_hasMessages) {
        showEmptyState();
    }
}

void PetChatWidget::setApiInfo(const QString &apiConfigName, const QString &model)
{
    m_apiConfigName = apiConfigName;
    m_model = model;
    if (!m_hasMessages) {
        showEmptyState();
    }
}

void PetChatWidget::appendUserMessage(const QString &content)
{
    hideEmptyState();

    QString currentText = m_messageDisplay->toPlainText();
    QString newMessage = tr("用户：\n%1\n").arg(content);
    m_messageDisplay->setPlainText(currentText + newMessage);

    QTextCursor cursor = m_messageDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_messageDisplay->setTextCursor(cursor);
}

void PetChatWidget::appendAiMessage(const QString &content)
{
    QString currentText = m_messageDisplay->toPlainText();
    QString newMessage = tr("\n宠物：\n%1\n").arg(content);
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
    m_hasMessages = false;
    m_messageDisplay->clear();
    showEmptyState();
}

void PetChatWidget::submitMessage()
{
    QString content = m_inputEdit->toPlainText().trimmed();
    if (content.isEmpty()) {
        return;
    }

    appendUserMessage(content);
    m_inputEdit->clear();

    appendSystemMessage(tr("对话回复将在后续版本接入。"));

    emit messageSubmitted(content);
}
