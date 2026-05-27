#include "logpage.h"

#include "core/appsettings.h"
#include "services/chatlogservice.h"
#include "theme/thememanager.h"
#include "widgets/softmessagebox.h"

#include <QFileDialog>
#include <QFont>
#include <QHBoxLayout>
#include <QLabel>
#include <QVBoxLayout>

LogPage::LogPage(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_titleLabel(nullptr)
    , m_logCard(nullptr)
    , m_loadLogButton(nullptr)
    , m_filePathEdit(nullptr)
    , m_searchLabel(nullptr)
    , m_searchEdit(nullptr)
    , m_logDisplay(nullptr)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
    setupUi();
    connectSignals();
}

LogPage::~LogPage() {}

void LogPage::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(theme.softScrollAreaStyleSheet());

    m_contentWidget = new QWidget();
    m_contentWidget->setObjectName("softPageSurface");
    m_contentWidget->setStyleSheet(theme.softPageStyleSheet());

    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(24, 10, 24, 20);
    contentLayout->setSpacing(12);

    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(24);

    m_titleLabel = new QLabel(tr("对话日志"), m_contentWidget);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());
    headerLayout->addWidget(m_titleLabel);

    headerLayout->addStretch();
    contentLayout->addLayout(headerLayout);

    m_logCard = new SoftCardWidget(m_contentWidget);
    m_logCard->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QVBoxLayout *cardLayout = new QVBoxLayout(m_logCard);
    cardLayout->setContentsMargins(20, 16, 20, 20);
    cardLayout->setSpacing(12);

    QHBoxLayout *toolbarLayout = new QHBoxLayout();
    toolbarLayout->setSpacing(12);

    m_loadLogButton = new QPushButton(tr("加载日志"), m_logCard);
    m_loadLogButton->setFixedHeight(32);
    m_loadLogButton->setStyleSheet(theme.softButtonStyleSheet());
    toolbarLayout->addWidget(m_loadLogButton);

    m_filePathEdit = new QLineEdit(m_logCard);
    m_filePathEdit->setReadOnly(true);
    m_filePathEdit->setPlaceholderText(tr("未选择日志文件"));
    m_filePathEdit->setStyleSheet(theme.lineEditStyleSheet());
    toolbarLayout->addWidget(m_filePathEdit, 1);

    cardLayout->addLayout(toolbarLayout);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->setSpacing(12);

    m_searchLabel = new QLabel(tr("搜索"), m_logCard);
    m_searchLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                     .arg(theme.currentPalette().textPrimary));
    searchLayout->addWidget(m_searchLabel);

    m_searchEdit = new QLineEdit(m_logCard);
    m_searchEdit->setPlaceholderText(tr("搜索对话内容 / 宠物 / API配置"));
    m_searchEdit->setStyleSheet(theme.lineEditStyleSheet());
    searchLayout->addWidget(m_searchEdit, 1);

    cardLayout->addLayout(searchLayout);

    m_logDisplay = new QPlainTextEdit(m_logCard);
    m_logDisplay->setReadOnly(true);
    m_logDisplay->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_logDisplay->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_logDisplay->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_logDisplay->setStyleSheet(
        QString("QPlainTextEdit { background: %1; color: %2; border: 1px solid %3; "
                "border-radius: 8px; padding: 12px; font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; }"
                "QPlainTextEdit:focus { border: 1px solid %4; }")
            .arg(theme.currentPalette().cardBackground,
                 theme.currentPalette().textPrimary,
                 theme.currentPalette().border,
                 theme.currentPalette().accent));
    m_logDisplay->setMinimumHeight(400);
    m_logDisplay->clear();
    cardLayout->addWidget(m_logDisplay, 1);

    contentLayout->addWidget(m_logCard, 1);

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea);
}

void LogPage::connectSignals()
{
    connect(m_loadLogButton, &QPushButton::clicked, this, &LogPage::onLoadLogClicked);
    connect(m_searchEdit, &QLineEdit::textChanged, this, &LogPage::onSearchTextChanged);
}

void LogPage::refreshTheme()
{
    applyTheme();
}

void LogPage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    m_scrollArea->setStyleSheet(theme.softScrollAreaStyleSheet());
    m_contentWidget->setStyleSheet(theme.softPageStyleSheet());
    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());

    m_loadLogButton->setStyleSheet(theme.softButtonStyleSheet());
    m_filePathEdit->setStyleSheet(theme.lineEditStyleSheet());
    m_searchEdit->setStyleSheet(theme.lineEditStyleSheet());
    m_searchLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                     .arg(p.textPrimary));

    m_logDisplay->setStyleSheet(
        QString("QPlainTextEdit { background: %1; color: %2; border: 1px solid %3; "
                "border-radius: 8px; padding: 12px; font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif; }"
                "QPlainTextEdit:focus { border: 1px solid %4; }")
            .arg(p.cardBackground, p.textPrimary, p.border, p.accent));
}

void LogPage::onLoadLogClicked()
{
    QString currentPetId = AppSettings::currentPetId();
    QString petLogDir = ChatLogService::petLogDirectory(currentPetId);

    QString filePath = QFileDialog::getOpenFileName(
        this,
        tr("选择日志文件"),
        petLogDir,
        tr("Chat Logs (*.jsonl);;All Files (*.*)"));

    if (filePath.isEmpty()) {
        return;
    }

    m_currentFilePath = filePath;
    m_filePathEdit->setText(filePath);
    m_searchEdit->clear();

    m_currentEntries = ChatLogService::loadLogFile(filePath);

    if (m_currentEntries.isEmpty()) {
        m_logDisplay->clear();
        return;
    }

    displayEntries(m_currentEntries);
}

void LogPage::onSearchTextChanged(const QString &text)
{
    if (m_currentEntries.isEmpty()) {
        return;
    }

    QList<ChatLogEntry> filtered = ChatLogService::filterEntries(m_currentEntries, text);

    if (filtered.isEmpty()) {
        m_logDisplay->clear();
    } else {
        displayEntries(filtered);
    }
}

void LogPage::displayEntries(const QList<ChatLogEntry> &entries)
{
    QString displayText = ChatLogService::formatEntriesForDisplay(entries);
    m_logDisplay->setPlainText(displayText);
}
