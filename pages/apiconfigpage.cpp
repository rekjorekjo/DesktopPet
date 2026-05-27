#include "apiconfigpage.h"

#include "core/appsettings.h"
#include "dialogs/apiconfigdialog.h"
#include "services/apiprofileservice.h"
#include "services/chatsettingsservice.h"
#include "services/secretstorageservice.h"
#include "theme/thememanager.h"
#include "widgets/softmessagebox.h"

#include <QFont>
#include <QHBoxLayout>
#include <QPainter>
#include <QSignalBlocker>
#include <QVBoxLayout>

ApiConfigPage::ApiConfigPage(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_titleLabel(nullptr)
    , m_statusCard(nullptr)
    , m_profilesCard(nullptr)
    , m_profilesCardTitle(nullptr)
    , m_currentApiProfileLabel(nullptr)
    , m_apiProfileList(nullptr)
    , m_addApiProfileButton(nullptr)
    , m_emptyLabel(nullptr)
    , m_configDialog(nullptr)
    , m_chatSettingsCard(nullptr)
    , m_chatSettingsTitleLabel(nullptr)
    , m_systemPromptEdit(nullptr)
    , m_resetPromptButton(nullptr)
    , m_savePromptButton(nullptr)
    , m_testConnectionButton(nullptr)
    , m_testChatService(new ChatCompletionService(this))
    , m_testPending(false)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
    setupUi();
    connectSignals();

    // Load persisted profiles
    QString loadError;
    if (!ApiProfileService::instance().load(&loadError)) {
        SoftMessageBox::warning(this,
                                tr("加载配置"),
                                tr("加载 API 配置失败：\n%1").arg(loadError));
    }

    refreshProfileList();
    updateCurrentProfileDisplay();

    // Load chat settings
    ChatSettingsService::instance().load();
    loadChatSettingsToUi();
}

ApiConfigPage::~ApiConfigPage() {}

void ApiConfigPage::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

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

    m_titleLabel = new QLabel(tr("聊天设置"), m_contentWidget);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());
    m_titleLabel->setMargin(0);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();

    contentLayout->addLayout(headerLayout);

    m_statusCard = new SoftCardWidget(m_contentWidget);
    m_statusCard->setObjectName("statusCard");
    m_statusCard->setBackgroundOpacity(40);
    QHBoxLayout *statusOuterLayout = new QHBoxLayout(m_statusCard);
    statusOuterLayout->setContentsMargins(24, 20, 24, 20);
    statusOuterLayout->setSpacing(12);

    QVBoxLayout *statusTextLayout = new QVBoxLayout();
    statusTextLayout->setSpacing(10);

    m_currentApiProfileLabel = new QLabel(tr("当前API配置：未选择"), m_statusCard);
    QFont statusTitleFont = m_currentApiProfileLabel->font();
    statusTitleFont.setPointSize(12);
    statusTitleFont.setBold(true);
    m_currentApiProfileLabel->setFont(statusTitleFont);
    m_currentApiProfileLabel->setStyleSheet(
        QString("color: %1; border: none; background: transparent;")
            .arg(p.accent));
    statusTextLayout->addWidget(m_currentApiProfileLabel);

    QLabel *storageNoteLabel = new QLabel(
        tr("存储后端：%1 — 请妥善保管设备与 API 配置文件。")
            .arg(SecretStorageService::backendName()),
        m_statusCard);
    storageNoteLabel->setStyleSheet(QString(
        "color: %1; border: none; background: transparent; font-size: 12px;"
    ).arg(p.textSecondary));
    statusTextLayout->addWidget(storageNoteLabel);

    statusOuterLayout->addLayout(statusTextLayout, 1);

    m_testConnectionButton = new QPushButton(tr("测试连接"), m_statusCard);
    m_testConnectionButton->setMinimumHeight(32);
    m_testConnectionButton->setMinimumWidth(80);
    m_testConnectionButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    connect(m_testConnectionButton, &QPushButton::clicked,
            this, &ApiConfigPage::onTestConnectionClicked);
    statusOuterLayout->addWidget(m_testConnectionButton, 0, Qt::AlignVCenter);

    contentLayout->addWidget(m_statusCard);

    m_profilesCard = new SoftCardWidget(m_contentWidget);
    m_profilesCard->setObjectName("profilesCard");
    m_profilesCard->setBackgroundOpacity(40);
    QVBoxLayout *profilesLayout = new QVBoxLayout(m_profilesCard);
    profilesLayout->setContentsMargins(24, 24, 24, 24);
    profilesLayout->setSpacing(16);

    QHBoxLayout *profilesHeaderLayout = new QHBoxLayout();
    profilesHeaderLayout->setSpacing(12);

    m_profilesCardTitle = new QLabel(tr("配置库"), m_profilesCard);
    QFont profilesTitleFont = m_profilesCardTitle->font();
    profilesTitleFont.setPointSize(12);
    profilesTitleFont.setBold(true);
    m_profilesCardTitle->setFont(profilesTitleFont);
    m_profilesCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                     .arg(p.subtitleText));
    profilesHeaderLayout->addWidget(m_profilesCardTitle);
    profilesHeaderLayout->addStretch();

    m_addApiProfileButton = new QPushButton(tr("新增配置"), m_profilesCard);
    m_addApiProfileButton->setMinimumHeight(32);
    m_addApiProfileButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    profilesHeaderLayout->addWidget(m_addApiProfileButton);

    profilesLayout->addLayout(profilesHeaderLayout);

    m_apiProfileList = new QListWidget(m_profilesCard);
    m_apiProfileList->setMinimumHeight(160);
    m_apiProfileList->setStyleSheet(theme.listWidgetStyleSheet());
    profilesLayout->addWidget(m_apiProfileList, 1);

    m_emptyLabel = new QLabel(tr("暂无配置，点击右上角「新增配置」添加"), m_profilesCard);
    m_emptyLabel->setAlignment(Qt::AlignCenter);
    m_emptyLabel->setStyleSheet(QString(
        "color: %1; font-size: 13px; background: transparent; border: none; padding: 40px 0;"
    ).arg(p.textSecondary));
    profilesLayout->addWidget(m_emptyLabel);

    contentLayout->addWidget(m_profilesCard, 1);

    setupChatSettingsSection(contentLayout);

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);
}

void ApiConfigPage::connectSignals()
{
    connect(m_addApiProfileButton, &QPushButton::clicked, this, &ApiConfigPage::onAddApiProfile);
    connect(m_apiProfileList, &QListWidget::currentRowChanged, this, &ApiConfigPage::onApiProfileSelectionChanged);
    connect(m_testChatService, &ChatCompletionService::connectionTestFinished,
            this, &ApiConfigPage::onConnectionTestFinished);
}

void ApiConfigPage::refreshTheme()
{
    applyTheme();
    refreshProfileList();
}

void ApiConfigPage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());
    m_scrollArea->setStyleSheet(theme.softScrollAreaStyleSheet());
    m_contentWidget->setStyleSheet(theme.softPageStyleSheet());

    m_currentApiProfileLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                                .arg(p.accent));
    m_profilesCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(p.subtitleText));

    m_addApiProfileButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_apiProfileList->setStyleSheet(theme.listWidgetStyleSheet());

    m_emptyLabel->setStyleSheet(QString(
        "color: %1; font-size: 13px; background: transparent; border: none; padding: 40px 0;"
    ).arg(p.textSecondary));

    // Chat settings section
    if (m_chatSettingsTitleLabel) {
        m_chatSettingsTitleLabel->setStyleSheet(QString(
            "color: %1; border: none; background: transparent;"
        ).arg(p.subtitleText));
    }
    if (m_systemPromptEdit) {
        QString scrollStyle = theme.scrollBarStyleSheet(false);
        m_systemPromptEdit->setStyleSheet(QString(
            "QPlainTextEdit {"
            "  background-color: %1;"
            "  color: %2;"
            "  border: 1px solid %3;"
            "  border-radius: 8px;"
            "  padding: 10px;"
            "  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif;"
            "  selection-background-color: %4;"
            "}"
            "QPlainTextEdit:hover {"
            "  border-color: %5;"
            "}"
            "QPlainTextEdit:focus {"
            "  border-color: %6;"
            "}"
        ).arg(p.inputBackground,
              p.inputText,
              p.inputBorder,
              p.selectionBackground,
              p.inputHoverBorder,
              p.inputFocusBorder) + scrollStyle);
    }
    if (m_resetPromptButton) {
        m_resetPromptButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    }
    if (m_savePromptButton) {
        m_savePromptButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    }
    if (m_testConnectionButton) {
        m_testConnectionButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    }
}

void ApiConfigPage::updateEmptyState()
{
    bool empty = ApiProfileService::instance().isEmpty();
    m_emptyLabel->setVisible(empty);
    m_apiProfileList->setVisible(!empty);
}

void ApiConfigPage::updateCurrentProfileDisplay()
{
    ApiProfileService &svc = ApiProfileService::instance();
    QString current = svc.currentProfileName();
    if (current.isEmpty()) {
        m_currentApiProfileLabel->setText(tr("当前API配置：未选择"));
    } else {
        m_currentApiProfileLabel->setText(tr("当前API配置：%1").arg(current));
    }

    if (m_testConnectionButton) {
        m_testConnectionButton->setEnabled(!current.isEmpty() && !m_testPending);
    }
}

QIcon ApiConfigPage::tintedIcon(const QString &path, const QColor &color) const
{
    const int sz = 20;
    QIcon base(path);
    QPixmap src = base.pixmap(QSize(sz, sz));
    if (src.isNull()) return QIcon();

    QPixmap tinted(sz, sz);
    tinted.fill(Qt::transparent);

    QPainter painter(&tinted);
    painter.drawPixmap(0, 0, src);
    painter.setCompositionMode(QPainter::CompositionMode_SourceIn);
    painter.fillRect(tinted.rect(), color);
    painter.end();

    return QIcon(tinted);
}

void ApiConfigPage::refreshProfileList()
{
    ApiProfileService &svc = ApiProfileService::instance();

    QSignalBlocker blocker(m_apiProfileList);
    m_apiProfileList->clear();

    const QStringList names = svc.profileNames();
    QString currentName = svc.currentProfileName();

    int targetRow = -1;
    int row = 0;
    for (const QString &name : names) {
        QListWidgetItem *item = new QListWidgetItem(m_apiProfileList);
        item->setText(QString());
        item->setData(Qt::UserRole, name);
        item->setSizeHint(QSize(0, 56));
        m_apiProfileList->setItemWidget(item, createProfileRowWidget(name));

        if (name == currentName) {
            targetRow = row;
        }
        ++row;
    }

    if (targetRow >= 0) {
        m_apiProfileList->setCurrentRow(targetRow);
    }

    updateEmptyState();
}

QWidget *ApiConfigPage::createProfileRowWidget(const QString &profileName)
{
    ThemePalette p = ThemeManager::instance().currentPalette();

    ApiConfig config;
    ApiProfileService::instance().profile(profileName, &config);

    QWidget *rowWidget = new QWidget();
    rowWidget->setObjectName("profileRowWidget");

    QHBoxLayout *layout = new QHBoxLayout(rowWidget);
    layout->setContentsMargins(12, 0, 8, 0);
    layout->setSpacing(12);

    // Name
    QLabel *nameLabel = new QLabel(profileName, rowWidget);
    nameLabel->setMinimumWidth(120);
    nameLabel->setStyleSheet(QString(
        "color: %1; background: transparent; border: none; font-size: 14px; font-weight: bold;"
    ).arg(p.textPrimary));
    layout->addWidget(nameLabel);

    // Model
    QString modelText = config.model.isEmpty() ? QString() : config.model;
    QLabel *modelLabel = new QLabel(modelText, rowWidget);
    modelLabel->setStyleSheet(QString(
        "color: %1; background: transparent; border: none; font-size: 12px;"
    ).arg(p.textSecondary));
    layout->addWidget(modelLabel, 1);

    // Icon buttons
    auto makeIconButton = [&](const QIcon &icon, const QString &tooltip,
                              const QString &hoverBg) -> QPushButton * {
        QPushButton *btn = new QPushButton(rowWidget);
        btn->setToolTip(tooltip);
        btn->setFixedSize(36, 36);
        btn->setIconSize(QSize(20, 20));
        btn->setCursor(Qt::PointingHandCursor);
        btn->setIcon(icon);
        btn->setStyleSheet(QString(
            "QPushButton {"
            "  background: transparent;"
            "  border: none;"
            "  border-radius: 6px;"
            "  padding: 0px;"
            "}"
            "QPushButton:hover {"
            "  background-color: %1;"
            "}"
            "QPushButton:pressed {"
            "  background-color: %2;"
            "}"
        ).arg(hoverBg, p.accentPressed));
        return btn;
    };

    QString hoverBg = p.listHoverBg.isEmpty() ? p.secondaryHover : p.listHoverBg;
    QString dangerHover = p.dangerHover.isEmpty() ? hoverBg : p.dangerHover;

    QIcon editIcon = tintedIcon(":/icons/edit.svg", QColor(p.iconPrimary));
    QIcon trashIcon = tintedIcon(":/icons/trash.svg", QColor(p.dangerText));

    QPushButton *editBtn = makeIconButton(editIcon, tr("编辑"), hoverBg);
    connect(editBtn, &QPushButton::clicked, this, [this, rowWidget]() {
        for (int i = 0; i < m_apiProfileList->count(); ++i) {
            if (m_apiProfileList->itemWidget(m_apiProfileList->item(i)) == rowWidget) {
                onEditApiProfile(i);
                return;
            }
        }
    });
    layout->addWidget(editBtn);

    QPushButton *delBtn = makeIconButton(trashIcon, tr("删除"), dangerHover);
    connect(delBtn, &QPushButton::clicked, this, [this, rowWidget]() {
        for (int i = 0; i < m_apiProfileList->count(); ++i) {
            if (m_apiProfileList->itemWidget(m_apiProfileList->item(i)) == rowWidget) {
                onRemoveApiProfile(i);
                return;
            }
        }
    });
    layout->addWidget(delBtn);

    return rowWidget;
}

void ApiConfigPage::onAddApiProfile()
{
    if (m_configDialog) {
        m_configDialog->raise();
        m_configDialog->activateWindow();
        return;
    }

    auto *dialog = new ApiConfigDialog(this);
    dialog->setTitle(tr("新增配置"));
    dialog->setNameEditable(true);
    dialog->setExistingNames(ApiProfileService::instance().profileNames());
    dialog->setValidateProfileName(true);

    m_editingProfileName.clear();
    m_configDialog = dialog;

    connect(dialog, &ApiConfigDialog::submitted,
            this, &ApiConfigPage::onDialogSubmitted);
    connect(dialog, &QObject::destroyed, this, [this]() {
        m_configDialog = nullptr;
    });

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void ApiConfigPage::onEditApiProfile(int row)
{
    QListWidgetItem *item = m_apiProfileList->item(row);
    if (!item) return;

    QString profileName = item->data(Qt::UserRole).toString();

    ApiConfig config;
    if (!ApiProfileService::instance().profile(profileName, &config)) return;

    if (m_configDialog) {
        m_configDialog->raise();
        m_configDialog->activateWindow();
        return;
    }

    auto *dialog = new ApiConfigDialog(this);
    dialog->setTitle(tr("编辑配置 - %1").arg(profileName));
    dialog->setProfileName(profileName);
    dialog->setNameEditable(true);
    dialog->setApiConfig(config);

    QStringList existingNames = ApiProfileService::instance().profileNames();
    existingNames.removeAll(profileName);
    dialog->setExistingNames(existingNames);
    dialog->setValidateProfileName(true);

    m_editingProfileName = profileName;
    m_configDialog = dialog;

    connect(dialog, &ApiConfigDialog::submitted,
            this, &ApiConfigPage::onDialogSubmitted);
    connect(dialog, &QObject::destroyed, this, [this]() {
        m_configDialog = nullptr;
    });

    dialog->show();
    dialog->raise();
    dialog->activateWindow();
}

void ApiConfigPage::onDialogSubmitted(const QString &profileName, const ApiConfig &config)
{
    if (!m_configDialog) return;

    ApiProfileService &svc = ApiProfileService::instance();
    bool isEdit = !m_editingProfileName.isEmpty();
    QString error;

    bool ok;
    if (isEdit) {
        ok = svc.updateProfile(m_editingProfileName, profileName, config, &error);
    } else {
        ok = svc.addProfile(profileName, config, &error);
        if (ok) {
            QString setError;
            if (!svc.setCurrentProfileName(profileName, &setError)) {
                SoftMessageBox::warning(this, tr("新增配置"),
                                        tr("配置已保存，但设置当前API配置失败：\n%1").arg(setError));
            }
        }
    }

    if (!ok) {
        SoftMessageBox::warning(this,
                                isEdit ? tr("编辑配置") : tr("新增配置"),
                                error);
        return;
    }

    m_configDialog->accept();
    m_configDialog = nullptr;
    m_editingProfileName.clear();

    refreshProfileList();
    updateCurrentProfileDisplay();
}

void ApiConfigPage::onRemoveApiProfile(int row)
{
    QListWidgetItem *item = m_apiProfileList->item(row);
    if (!item) return;

    QString profileName = item->data(Qt::UserRole).toString();

    bool yes = SoftMessageBox::question(this,
                                        tr("确认删除"),
                                        tr("确定要删除配置「%1」吗？").arg(profileName))
               == SoftMessageBox::Yes;
    if (!yes) return;

    QString error;
    if (!ApiProfileService::instance().removeProfile(profileName, &error)) {
        SoftMessageBox::warning(this, tr("删除配置"), error);
        return;
    }

    refreshProfileList();
    updateCurrentProfileDisplay();
}

void ApiConfigPage::onApiProfileSelectionChanged()
{
    QListWidgetItem *currentItem = m_apiProfileList->currentItem();
    if (!currentItem) {
        return;
    }

    QString profileName = currentItem->data(Qt::UserRole).toString();

    QString error;
    if (!ApiProfileService::instance().setCurrentProfileName(profileName, &error)) {
        return;
    }

    updateCurrentProfileDisplay();
    refreshProfileList();
}

// ── Chat settings section ──────────────────────────────────────────

void ApiConfigPage::setupChatSettingsSection(QVBoxLayout *parentLayout)
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    m_chatSettingsCard = new SoftCardWidget(m_contentWidget);
    m_chatSettingsCard->setObjectName("chatSettingsCard");
    m_chatSettingsCard->setBackgroundOpacity(40);
    QVBoxLayout *cardLayout = new QVBoxLayout(m_chatSettingsCard);
    cardLayout->setContentsMargins(24, 24, 24, 24);
    cardLayout->setSpacing(12);

    // Title
    m_chatSettingsTitleLabel = new QLabel(tr("对话设定"), m_chatSettingsCard);
    QFont titleFont = m_chatSettingsTitleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    m_chatSettingsTitleLabel->setFont(titleFont);
    m_chatSettingsTitleLabel->setStyleSheet(QString(
        "color: %1; border: none; background: transparent;"
    ).arg(p.subtitleText));
    cardLayout->addWidget(m_chatSettingsTitleLabel);

    // System prompt editor
    m_systemPromptEdit = new QPlainTextEdit(m_chatSettingsCard);
    m_systemPromptEdit->setMinimumHeight(140);
    m_systemPromptEdit->setMaximumHeight(240);
    m_systemPromptEdit->setPlaceholderText(tr("输入系统提示词..."));
    QString scrollStyle = theme.scrollBarStyleSheet(false);
    m_systemPromptEdit->setStyleSheet(QString(
        "QPlainTextEdit {"
        "  background-color: %1;"
        "  color: %2;"
        "  border: 1px solid %3;"
        "  border-radius: 8px;"
        "  padding: 10px;"
        "  font-family: 'Segoe UI', 'Microsoft YaHei', sans-serif;"
        "  selection-background-color: %4;"
        "}"
        "QPlainTextEdit:hover {"
        "  border-color: %5;"
        "}"
        "QPlainTextEdit:focus {"
        "  border-color: %6;"
        "}"
    ).arg(p.inputBackground,
          p.inputText,
          p.inputBorder,
          p.selectionBackground,
          p.inputHoverBorder,
          p.inputFocusBorder) + scrollStyle);
    cardLayout->addWidget(m_systemPromptEdit);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    m_resetPromptButton = new QPushButton(tr("恢复默认"), m_chatSettingsCard);
    m_resetPromptButton->setMinimumHeight(32);
    m_resetPromptButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    connect(m_resetPromptButton, &QPushButton::clicked,
            this, &ApiConfigPage::resetSystemPromptToDefault);
    buttonLayout->addWidget(m_resetPromptButton);

    buttonLayout->addStretch();

    m_savePromptButton = new QPushButton(tr("保存设定"), m_chatSettingsCard);
    m_savePromptButton->setMinimumHeight(32);
    m_savePromptButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    connect(m_savePromptButton, &QPushButton::clicked,
            this, &ApiConfigPage::saveChatSettingsFromUi);
    buttonLayout->addWidget(m_savePromptButton);

    cardLayout->addLayout(buttonLayout);

    parentLayout->addWidget(m_chatSettingsCard);
}

void ApiConfigPage::loadChatSettingsToUi()
{
    if (m_systemPromptEdit) {
        m_systemPromptEdit->setPlainText(ChatSettingsService::instance().systemPrompt());
    }
}

void ApiConfigPage::saveChatSettingsFromUi()
{
    QString prompt = m_systemPromptEdit->toPlainText().trimmed();
    if (prompt.isEmpty()) {
        prompt = ChatSettingsService::defaultSystemPrompt();
        m_systemPromptEdit->setPlainText(prompt);
        SoftMessageBox::information(this,
                                    tr("保存设定"),
                                    tr("系统提示词为空，已恢复默认。"));
    }

    ChatSettingsService::instance().setSystemPrompt(prompt);

    QString error;
    if (!ChatSettingsService::instance().save(&error)) {
        SoftMessageBox::warning(this,
                                tr("保存设定"),
                                tr("保存失败：\n%1").arg(error));
        return;
    }

    SoftMessageBox::information(this,
                                tr("保存设定"),
                                tr("对话设定已保存。"));
}

void ApiConfigPage::resetSystemPromptToDefault()
{
    if (m_systemPromptEdit) {
        m_systemPromptEdit->setPlainText(ChatSettingsService::defaultSystemPrompt());
    }
}

// ── Test connection ──────────────────────────────────────────────

void ApiConfigPage::onTestConnectionClicked()
{
    if (m_testPending) return;

    ApiConfig config;
    ApiProfileService &svc = ApiProfileService::instance();
    if (!svc.currentProfile(&config)) {
        SoftMessageBox::warning(this,
                                tr("测试连接"),
                                tr("请先选择一个 API 配置。"));
        return;
    }

    if (config.apiFormat != ApiFormat::OpenAICompatible) {
        SoftMessageBox::warning(this,
                                tr("测试连接"),
                                tr("当前仅支持 OpenAI 兼容格式的 API。"));
        return;
    }

    if (config.apiKey.trimmed().isEmpty()) {
        SoftMessageBox::warning(this,
                                tr("测试连接"),
                                tr("API 配置不完整：缺少 API_KEY。"));
        return;
    }
    if (config.baseUrl.trimmed().isEmpty()) {
        SoftMessageBox::warning(this,
                                tr("测试连接"),
                                tr("API 配置不完整：缺少 BASE_URL。"));
        return;
    }
    if (config.model.trimmed().isEmpty()) {
        SoftMessageBox::warning(this,
                                tr("测试连接"),
                                tr("API 配置不完整：缺少 MODEL。"));
        return;
    }

    m_testPending = true;
    m_testConnectionButton->setEnabled(false);
    m_testConnectionButton->setText(tr("测试中..."));

    m_testRequestId = m_testChatService->testConnection(config);
}

void ApiConfigPage::onConnectionTestFinished(const QString &requestId, bool ok, const QString &message)
{
    if (requestId != m_testRequestId) return;

    m_testPending = false;
    m_testRequestId.clear();
    m_testConnectionButton->setEnabled(true);
    m_testConnectionButton->setText(tr("测试连接"));

    if (ok) {
        SoftMessageBox::information(this,
                                    tr("测试连接"),
                                    tr("连接成功！\n%1").arg(message));
    } else {
        SoftMessageBox::warning(this,
                                tr("测试连接"),
                                tr("连接失败：\n%1").arg(message));
    }
}
