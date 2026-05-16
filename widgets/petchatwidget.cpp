#include "petchatwidget.h"

#include "services/apiprofileservice.h"
#include "theme/thememanager.h"
#include "widgets/emptystatewidget.h"

#include <QFrame>
#include <QKeyEvent>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

static const QString kSystemPrompt =
    QStringLiteral("你是 DesktopPet 的桌面宠物助手。请用简洁、自然、友好的中文回复用户。");

PetChatWidget::PetChatWidget(QWidget *parent)
    : QWidget(parent)
    , m_messageStack(nullptr)
    , m_emptyState(nullptr)
    , m_messageDisplay(nullptr)
    , m_inputEdit(nullptr)
    , m_hasMessages(false)
    , m_chatService(new ChatCompletionService(this))
    , m_requestPending(false)
{
    connect(m_chatService, &ChatCompletionService::requestFinished,
            this, &PetChatWidget::onRequestFinished);
    connect(m_chatService, &ChatCompletionService::requestFailed,
            this, &PetChatWidget::onRequestFailed);

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
    m_messageStack->setAttribute(Qt::WA_StyledBackground, true);
    m_messageStack->setStyleSheet("background: transparent; border: none;");

    m_emptyState = new EmptyStateWidget(m_messageStack);
    m_emptyState->setTitle(tr("对话"));
    m_messageStack->addWidget(m_emptyState);

    m_messageDisplay = new QPlainTextEdit(m_messageStack);
    m_messageDisplay->setReadOnly(true);
    m_messageDisplay->setFrameShape(QFrame::NoFrame);
    m_messageDisplay->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_messageDisplay->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_messageDisplay->setTextInteractionFlags(Qt::TextSelectableByMouse);
    m_messageStack->addWidget(m_messageDisplay);

    m_messageStack->setCurrentWidget(m_emptyState);
    mainLayout->addWidget(m_messageStack, 1);

    m_inputEdit = new QPlainTextEdit(this);
    m_inputEdit->setFrameShape(QFrame::NoFrame);
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

    QString scrollStyle = theme.scrollBarStyleSheet(false);

    QString inputStyle = QString(
        "QPlainTextEdit {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 8px;"
        "  padding: 10px;"
        "  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif;"
        "  placeholder-text-color: %4;"
        "  selection-background-color: %5;"
        "}"
        "QPlainTextEdit:hover {"
        "  border-color: %6;"
        "}"
        "QPlainTextEdit:focus {"
        "  border-color: %7;"
        "}"
    ).arg(p.inputBackground,
          p.inputText,
          p.inputBorder,
          p.placeholderText,
          p.selectionBackground,
          p.inputHoverBorder,
          p.inputFocusBorder);

    m_inputEdit->setStyleSheet(inputStyle + scrollStyle);

    QString messageStyle = QString(
        "QPlainTextEdit {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: none;"
        "  border-radius: 8px;"
        "  padding: 10px;"
        "  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif;"
        "  selection-background-color: %3;"
        "}"
    ).arg(p.inputBackground, p.inputText, p.selectionBackground);

    m_messageDisplay->setStyleSheet(messageStyle + scrollStyle);

    if (m_emptyState) {
        m_emptyState->applyTheme();
    }

    update();
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
    painter.fillPath(path, bgColor);

    QColor borderColor(p.border);
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
    hideEmptyState();

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
    m_messages.clear();
    m_pendingRequestId.clear();
    m_requestPending = false;
    setWaitingState(false);
    showEmptyState();
}

void PetChatWidget::submitMessage()
{
    QString content = m_inputEdit->toPlainText().trimmed();
    if (content.isEmpty()) {
        return;
    }

    if (m_requestPending) {
        appendSystemMessage(tr("请等待当前回复完成..."));
        return;
    }

    // Get current API config
    ApiConfig config;
    ApiProfileService &svc = ApiProfileService::instance();
    if (!svc.currentProfile(&config)) {
        appendSystemMessage(tr("请先在设置中配置 API。"));
        return;
    }

    if (config.apiFormat != ApiFormat::OpenAICompatible) {
        appendSystemMessage(tr("当前仅支持 OpenAI 兼容格式的 API。"));
        return;
    }

    if (config.apiKey.trimmed().isEmpty()) {
        appendSystemMessage(tr("API 配置不完整：缺少 API_KEY。"));
        return;
    }
    if (config.baseUrl.trimmed().isEmpty()) {
        appendSystemMessage(tr("API 配置不完整：缺少 BASE_URL。"));
        return;
    }
    if (config.model.trimmed().isEmpty()) {
        appendSystemMessage(tr("API 配置不完整：缺少 MODEL。"));
        return;
    }

    appendUserMessage(content);
    m_inputEdit->clear();

    // Add user message to context
    ChatCompletionService::Message userMsg;
    userMsg.role = "user";
    userMsg.content = content;
    m_messages.append(userMsg);

    // Trim context to keep last 12 messages + system prompt
    while (m_messages.size() > 12) {
        m_messages.removeFirst();
    }

    // Build full message list with system prompt
    QList<ChatCompletionService::Message> requestMessages;
    ChatCompletionService::Message sysMsg;
    sysMsg.role = "system";
    sysMsg.content = kSystemPrompt;
    requestMessages.append(sysMsg);
    requestMessages.append(m_messages);

    m_requestPending = true;
    setWaitingState(true);
    m_pendingRequestId = m_chatService->sendChatCompletion(config, requestMessages);

    emit messageSubmitted(content);
}

void PetChatWidget::setWaitingState(bool waiting)
{
    m_inputEdit->setEnabled(!waiting);
    if (waiting) {
        m_inputEdit->setPlaceholderText(tr("正在等待回复..."));
    } else {
        m_inputEdit->setPlaceholderText(tr("输入消息，按 Enter 发送，Shift+Enter 换行..."));
    }
}

void PetChatWidget::onRequestFinished(const QString &requestId, const QString &content)
{
    if (requestId != m_pendingRequestId) return;

    m_pendingRequestId.clear();
    m_requestPending = false;
    setWaitingState(false);

    // Add assistant message to context
    ChatCompletionService::Message assistantMsg;
    assistantMsg.role = "assistant";
    assistantMsg.content = content;
    m_messages.append(assistantMsg);

    appendAiMessage(content);
}

void PetChatWidget::onRequestFailed(const QString &requestId, const QString &errorMessage)
{
    if (requestId != m_pendingRequestId) return;

    m_pendingRequestId.clear();
    m_requestPending = false;
    setWaitingState(false);

    appendSystemMessage(tr("请求失败：%1").arg(errorMessage));
}
