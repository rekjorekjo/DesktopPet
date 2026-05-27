#include "petchatwidget.h"

#include "models/websearchconfig.h"
#include "services/apiprofileservice.h"
#include "services/chatsettingsservice.h"
#include "services/websearchsettingsservice.h"
#include "theme/thememanager.h"
#include "widgets/emptystatewidget.h"

#include <QFrame>
#include <QHBoxLayout>
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
    , m_cancelButton(nullptr)
    , m_hasMessages(false)
    , m_chatService(new ChatCompletionService(this))
    , m_requestPending(false)
    , m_webSearchService(new WebSearchService(this))
    , m_waitingForSearch(false)
{
    connect(m_chatService, &ChatCompletionService::requestFinished,
            this, &PetChatWidget::onRequestFinished);
    connect(m_chatService, &ChatCompletionService::requestFailed,
            this, &PetChatWidget::onRequestFailed);
    connect(m_chatService, &ChatCompletionService::requestCanceled,
            this, &PetChatWidget::onRequestCanceled);

    connect(m_webSearchService, &WebSearchService::searchFinished,
            this, &PetChatWidget::onWebSearchFinished);
    connect(m_webSearchService, &WebSearchService::searchFailed,
            this, &PetChatWidget::onWebSearchFailed);
    connect(m_webSearchService, &WebSearchService::searchCanceled,
            this, &PetChatWidget::onWebSearchCanceled);

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

    // Bottom input area: horizontal layout with input + cancel button
    QWidget *inputPanel = new QWidget(this);
    inputPanel->setObjectName("chatInputPanel");

    QHBoxLayout *inputLayout = new QHBoxLayout(inputPanel);
    inputLayout->setContentsMargins(0, 0, 0, 0);
    inputLayout->setSpacing(6);

    m_inputEdit = new QPlainTextEdit(inputPanel);
    m_inputEdit->setFrameShape(QFrame::NoFrame);
    m_inputEdit->setMinimumHeight(48);
    m_inputEdit->setMaximumHeight(80);
    m_inputEdit->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_inputEdit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_inputEdit->setPlaceholderText(tr("输入消息，按 Enter 发送，Shift+Enter 换行..."));
    m_inputEdit->installEventFilter(this);
    inputLayout->addWidget(m_inputEdit, 1);

    m_cancelButton = new QPushButton(tr("取消"), inputPanel);
    m_cancelButton->setObjectName("chatCancelButton");
    m_cancelButton->setMinimumHeight(48);
    m_cancelButton->setMaximumHeight(80);
    m_cancelButton->setFixedWidth(56);
    m_cancelButton->setVisible(false);
    connect(m_cancelButton, &QPushButton::clicked, this, &PetChatWidget::cancelCurrentRequest);
    inputLayout->addWidget(m_cancelButton);

    mainLayout->addWidget(inputPanel);

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

    // Cancel button style
    m_cancelButton->setStyleSheet(QString(
        "QPushButton {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 8px;"
        "  padding: 6px 12px;"
        "  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif;"
        "  font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "  background-color: %4;"
        "}"
        "QPushButton:pressed {"
        "  background-color: %5;"
        "}"
    ).arg(p.inputBackground,
          p.dangerText,
          p.inputBorder,
          p.dangerHover,
          p.accentPressed));

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

void PetChatWidget::setPetDisplayName(const QString &name)
{
    m_petDisplayName = name.trimmed();
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
    QString newMessage = tr("我：\n%1\n").arg(content);
    m_messageDisplay->setPlainText(currentText + newMessage);

    QTextCursor cursor = m_messageDisplay->textCursor();
    cursor.movePosition(QTextCursor::End);
    m_messageDisplay->setTextCursor(cursor);
}

void PetChatWidget::appendAiMessage(const QString &content)
{
    QString currentText = m_messageDisplay->toPlainText();
    const QString speakerName = m_petDisplayName.isEmpty() ? tr("宠物") : m_petDisplayName;
    QString newMessage = tr("\n%1：\n%2\n").arg(speakerName, content);
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
    m_chatService->cancelAllRequests();

    m_pendingSearchRequestId.clear();
    m_pendingSearchQuery.clear();
    m_pendingUserMessage.clear();
    m_waitingForSearch = false;
    m_webSearchService->cancelAllRequests();

    setWaitingState(false);
    showEmptyState();
}

QString PetChatWidget::currentSystemPrompt() const
{
    QString prompt = ChatSettingsService::instance().systemPrompt().trimmed();
    if (prompt.isEmpty()) {
        prompt = ChatSettingsService::defaultSystemPrompt();
    }
    return prompt;
}

void PetChatWidget::submitMessage()
{
    QString content = m_inputEdit->toPlainText().trimmed();
    if (content.isEmpty()) {
        return;
    }

    if (m_requestPending || m_waitingForSearch) {
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
    emit messageSubmitted(content);

    startChatRequestWithOptionalSearch(content);
}

void PetChatWidget::setWaitingState(bool waiting)
{
    m_inputEdit->setEnabled(!waiting);
    if (waiting) {
        m_inputEdit->setPlaceholderText(tr("正在等待回复，可点击取消..."));
        m_cancelButton->setVisible(true);
        m_cancelButton->setEnabled(true);
    } else {
        m_inputEdit->setPlaceholderText(tr("输入消息，按 Enter 发送，Shift+Enter 换行..."));
        m_cancelButton->setVisible(false);
    }
}

void PetChatWidget::cancelCurrentRequest()
{
    if (m_waitingForSearch && !m_pendingSearchRequestId.isEmpty()) {
        m_webSearchService->cancelRequest(m_pendingSearchRequestId);
        return;
    }

    if (m_requestPending && !m_pendingRequestId.isEmpty()) {
        m_chatService->cancelRequest(m_pendingRequestId);
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

void PetChatWidget::onRequestCanceled(const QString &requestId)
{
    if (requestId != m_pendingRequestId) return;

    m_pendingRequestId.clear();
    m_requestPending = false;
    setWaitingState(false);

    appendSystemMessage(tr("已取消当前回复。"));
}

bool PetChatWidget::isForcedWebSearchQuery(const QString &text) const
{
    QString trimmed = text.trimmed();
    return trimmed.startsWith("/search ", Qt::CaseInsensitive)
        || trimmed.startsWith("#search ", Qt::CaseInsensitive)
        || trimmed.startsWith("搜一下")
        || trimmed.startsWith("查一下");
}

bool PetChatWidget::shouldUseWebSearch(const QString &message) const
{
    // Keyword-based trigger
    static const QStringList keywords = {
        QStringLiteral("最近"), QStringLiteral("最新"), QStringLiteral("今天"),
        QStringLiteral("昨天"), QStringLiteral("新闻"), QStringLiteral("天气"),
        QStringLiteral("股价"), QStringLiteral("汇率"), QStringLiteral("赛事"),
        QStringLiteral("比分"), QStringLiteral("上映"), QStringLiteral("发布"),
        QStringLiteral("热搜"), QStringLiteral("头条"), QStringLiteral("实时"),
        QStringLiteral("现在"), QStringLiteral("目前"), QStringLiteral("今年"),
        QStringLiteral("本月"), QStringLiteral("这周"), QStringLiteral("上周"),
    };

    for (const QString &kw : keywords) {
        if (message.contains(kw)) {
            return true;
        }
    }

    return false;
}

QString PetChatWidget::normalizeSearchQuery(const QString &message) const
{
    QString trimmed = message.trimmed();

    // Strip forced-search prefixes
    static const QStringList prefixTriggers = {
        "/search", "#search", "搜一下", "查一下"
    };
    for (const QString &prefix : prefixTriggers) {
        if (trimmed.startsWith(prefix, Qt::CaseInsensitive)) {
            trimmed = trimmed.mid(prefix.size()).trimmed();
            break;
        }
    }

    // If nothing left after stripping, use original
    if (trimmed.isEmpty()) {
        return message.trimmed();
    }

    return trimmed;
}

QString PetChatWidget::buildWebSearchContext(const QList<WebSearchResult> &results) const
{
    if (results.isEmpty()) {
        return QString();
    }

    QString ctx = QStringLiteral("[联网搜索结果]\n");
    for (int i = 0; i < results.size(); ++i) {
        const WebSearchResult &r = results.at(i);
        ctx += QString::number(i + 1) + ". ";
        if (!r.title.isEmpty()) {
            ctx += r.title + "\n";
        }
        if (!r.url.isEmpty()) {
            ctx += "   链接: " + r.url + "\n";
        }
        if (!r.snippet.isEmpty()) {
            ctx += "   摘要: " + r.snippet + "\n";
        }
        if (!r.publishedAt.isEmpty()) {
            ctx += "   时间: " + r.publishedAt + "\n";
        }
    }
    ctx += QStringLiteral("[搜索结果结束]\n\n请根据以上搜索结果回答用户的问题。如果搜索结果中没有相关信息，请如实说明。");
    return ctx;
}

void PetChatWidget::startChatRequestWithOptionalSearch(const QString &userMessage)
{
    WebSearchConfig config = WebSearchSettingsService::load();

    if (isForcedWebSearchQuery(userMessage)) {
        if (!config.enabled || config.apiKey.trimmed().isEmpty()) {
            m_requestPending = false;
            m_waitingForSearch = false;
            setWaitingState(false);
            appendSystemMessage(tr("请先在联网搜索设置中启用联网搜索并配置搜索 API Key。"));
            return;
        }

        QString query = normalizeSearchQuery(userMessage);
        m_pendingSearchQuery = query;
        m_pendingUserMessage = userMessage;
        m_waitingForSearch = true;
        setWaitingState(true);
        m_pendingSearchRequestId = m_webSearchService->search(query, config);
        return;
    }

    if (config.enabled && !config.apiKey.trimmed().isEmpty() && shouldUseWebSearch(userMessage)) {
        QString query = normalizeSearchQuery(userMessage);
        m_pendingSearchQuery = query;
        m_pendingUserMessage = userMessage;
        m_waitingForSearch = true;
        setWaitingState(true);
        m_pendingSearchRequestId = m_webSearchService->search(query, config);
        return;
    }

    // No search needed, send directly
    sendChatRequestWithWebContext(userMessage, QString());
}

void PetChatWidget::sendChatRequestWithWebContext(const QString &userMessage, const QString &webContext)
{
    // Get current system prompt
    const QString systemPrompt = currentSystemPrompt();

    // Ensure system message is first in context
    if (m_messages.isEmpty()) {
        ChatCompletionService::Message sysMsg;
        sysMsg.role = "system";
        sysMsg.content = systemPrompt;
        m_messages.append(sysMsg);
    } else if (m_messages.first().role == "system") {
        m_messages[0].content = systemPrompt;
    }

    // Build messages for this request
    QList<ChatCompletionService::Message> requestMessages = m_messages;

    // If we have web context, inject as temporary system message (NOT persisted in m_messages)
    if (!webContext.isEmpty()) {
        ChatCompletionService::Message webMsg;
        webMsg.role = "system";
        webMsg.content = webContext;
        requestMessages.append(webMsg);
    }

    // Add user message to persisted context
    ChatCompletionService::Message userMsg;
    userMsg.role = "user";
    userMsg.content = userMessage;
    m_messages.append(userMsg);

    // Add user message to request messages (always needed)
    requestMessages.append(userMsg);

    // Trim context: keep system message + last 12 normal messages
    if (m_messages.size() > 13) {
        QList<ChatCompletionService::Message> trimmed;
        trimmed.append(m_messages.first()); // system message
        int start = m_messages.size() - 12;
        for (int i = start; i < m_messages.size(); ++i) {
            trimmed.append(m_messages.at(i));
        }
        m_messages = trimmed;
    }

    // Get current API config
    ApiConfig config;
    ApiProfileService::instance().currentProfile(&config);

    m_requestPending = true;
    setWaitingState(true);
    m_pendingRequestId = m_chatService->sendChatCompletion(config, requestMessages);
}

void PetChatWidget::onWebSearchFinished(const QString &requestId, const QString &query, const QList<WebSearchResult> &results)
{
    Q_UNUSED(query);
    if (requestId != m_pendingSearchRequestId) return;

    m_pendingSearchRequestId.clear();
    m_waitingForSearch = false;

    // Show search status briefly in chat
    appendSystemMessage(tr("联网搜索完成，找到 %1 条结果，正在生成回答...").arg(results.size()));

    QString webContext = buildWebSearchContext(results);
    sendChatRequestWithWebContext(m_pendingUserMessage, webContext);
    m_pendingUserMessage.clear();
}

void PetChatWidget::onWebSearchFailed(const QString &requestId, const QString &query, const QString &message)
{
    Q_UNUSED(query);
    if (requestId != m_pendingSearchRequestId) return;

    m_pendingSearchRequestId.clear();
    m_pendingSearchQuery.clear();
    m_pendingUserMessage.clear();
    m_waitingForSearch = false;
    m_requestPending = false;
    setWaitingState(false);

    appendSystemMessage(tr("联网搜索失败：%1").arg(message));
}

void PetChatWidget::onWebSearchCanceled(const QString &requestId)
{
    if (requestId != m_pendingSearchRequestId) return;

    m_pendingSearchRequestId.clear();
    m_waitingForSearch = false;
    m_pendingUserMessage.clear();
    setWaitingState(false);

    appendSystemMessage(tr("已取消联网搜索。"));
}
