#include "websearchconfigpage.h"

#include "services/websearchservice.h"
#include "services/websearchsettingsservice.h"
#include "services/secretstorageservice.h"
#include "theme/thememanager.h"
#include "widgets/softmessagebox.h"

#include <QFont>
#include <QHBoxLayout>
#include <QVBoxLayout>

WebSearchConfigPage::WebSearchConfigPage(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_titleLabel(nullptr)
    , m_statusCard(nullptr)
    , m_statusLabel(nullptr)
    , m_settingsCard(nullptr)
    , m_settingsCardTitle(nullptr)
    , m_enabledCheckBox(nullptr)
    , m_providerLabel(nullptr)
    , m_providerCombo(nullptr)
    , m_apiKeyLabel(nullptr)
    , m_apiKeyEdit(nullptr)
    , m_resultCountLabel(nullptr)
    , m_resultCountSpin(nullptr)
    , m_searchDepthLabel(nullptr)
    , m_searchDepthCombo(nullptr)
    , m_timeoutLabel(nullptr)
    , m_timeoutSpin(nullptr)
    , m_saveButton(nullptr)
    , m_testButton(nullptr)
    , m_reloadButton(nullptr)
    , m_testService(new WebSearchService(this))
    , m_testPending(false)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
    setupUi();
    connectSignals();
    loadConfig();
    updateStatusDisplay();
}

WebSearchConfigPage::~WebSearchConfigPage() {}

void WebSearchConfigPage::setupUi()
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

    // Title
    QHBoxLayout *headerLayout = new QHBoxLayout();
    headerLayout->setSpacing(24);
    m_titleLabel = new QLabel(tr("搜索设置"), m_contentWidget);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());
    m_titleLabel->setMargin(0);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    contentLayout->addLayout(headerLayout);

    // Status card
    m_statusCard = new SoftCardWidget(m_contentWidget);
    m_statusCard->setObjectName("searchStatusCard");
    m_statusCard->setBackgroundOpacity(40);
    QHBoxLayout *statusLayout = new QHBoxLayout(m_statusCard);
    statusLayout->setContentsMargins(24, 20, 24, 20);
    statusLayout->setSpacing(12);

    m_statusLabel = new QLabel(m_statusCard);
    QFont statusTitleFont = m_statusLabel->font();
    statusTitleFont.setPointSize(12);
    statusTitleFont.setBold(true);
    m_statusLabel->setFont(statusTitleFont);
    m_statusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.accent));
    statusLayout->addWidget(m_statusLabel, 1);

    m_reloadButton = new QPushButton(tr("重新加载"), m_statusCard);
    m_reloadButton->setMinimumHeight(32);
    m_reloadButton->setMinimumWidth(80);
    m_reloadButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    connect(m_reloadButton, &QPushButton::clicked,
            this, &WebSearchConfigPage::onReloadClicked);
    statusLayout->addWidget(m_reloadButton, 0, Qt::AlignVCenter);

    contentLayout->addWidget(m_statusCard);

    // Settings card
    m_settingsCard = new SoftCardWidget(m_contentWidget);
    m_settingsCard->setObjectName("searchSettingsCard");
    m_settingsCard->setBackgroundOpacity(40);
    QVBoxLayout *settingsLayout = new QVBoxLayout(m_settingsCard);
    settingsLayout->setContentsMargins(24, 24, 24, 24);
    settingsLayout->setSpacing(16);

    m_settingsCardTitle = new QLabel(tr("搜索配置"), m_settingsCard);
    QFont settingsTitleFont = m_settingsCardTitle->font();
    settingsTitleFont.setPointSize(12);
    settingsTitleFont.setBold(true);
    m_settingsCardTitle->setFont(settingsTitleFont);
    m_settingsCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.subtitleText));
    settingsLayout->addWidget(m_settingsCardTitle);

    // Enabled checkbox
    m_enabledCheckBox = new QCheckBox(tr("启用联网搜索"), m_settingsCard);
    m_enabledCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    settingsLayout->addWidget(m_enabledCheckBox);

    // Provider combo
    QHBoxLayout *providerLayout = new QHBoxLayout();
    m_providerLabel = new QLabel(tr("搜索服务:"), m_settingsCard);
    m_providerLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.textPrimary));
    m_providerCombo = new QComboBox(m_settingsCard);
    m_providerCombo->addItem("Tavily", static_cast<int>(WebSearchProvider::Tavily));
    m_providerCombo->addItem("Brave Search", static_cast<int>(WebSearchProvider::Brave));
    m_providerCombo->addItem("Exa", static_cast<int>(WebSearchProvider::Exa));
    m_providerCombo->setMinimumWidth(200);
    providerLayout->addWidget(m_providerLabel);
    providerLayout->addWidget(m_providerCombo);
    providerLayout->addStretch();
    settingsLayout->addLayout(providerLayout);

    // API Key
    m_apiKeyLabel = new QLabel(tr("API Key:"), m_settingsCard);
    m_apiKeyLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.textSecondary));
    settingsLayout->addWidget(m_apiKeyLabel);

    m_apiKeyEdit = new QLineEdit(m_settingsCard);
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText(tr("输入搜索服务 API Key"));
    m_apiKeyEdit->setMinimumHeight(36);
    settingsLayout->addWidget(m_apiKeyEdit);

    // Result count
    QHBoxLayout *resultCountLayout = new QHBoxLayout();
    m_resultCountLabel = new QLabel(tr("结果数量:"), m_settingsCard);
    m_resultCountLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.textSecondary));
    m_resultCountSpin = new QSpinBox(m_settingsCard);
    m_resultCountSpin->setRange(1, 10);
    m_resultCountSpin->setValue(5);
    m_resultCountSpin->setMinimumWidth(80);
    resultCountLayout->addWidget(m_resultCountLabel);
    resultCountLayout->addWidget(m_resultCountSpin);
    resultCountLayout->addStretch();
    settingsLayout->addLayout(resultCountLayout);

    // Search depth
    QHBoxLayout *depthLayout = new QHBoxLayout();
    m_searchDepthLabel = new QLabel(tr("搜索深度:"), m_settingsCard);
    m_searchDepthLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.textSecondary));
    m_searchDepthCombo = new QComboBox(m_settingsCard);
    m_searchDepthCombo->addItem("basic", "basic");
    m_searchDepthCombo->addItem("advanced", "advanced");
    m_searchDepthCombo->setMinimumWidth(120);
    depthLayout->addWidget(m_searchDepthLabel);
    depthLayout->addWidget(m_searchDepthCombo);
    depthLayout->addStretch();
    settingsLayout->addLayout(depthLayout);

    // Timeout
    QHBoxLayout *timeoutLayout = new QHBoxLayout();
    m_timeoutLabel = new QLabel(tr("超时时间:"), m_settingsCard);
    m_timeoutLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.textSecondary));
    m_timeoutSpin = new QSpinBox(m_settingsCard);
    m_timeoutSpin->setRange(5, 60);
    m_timeoutSpin->setValue(15);
    m_timeoutSpin->setSuffix(tr(" 秒"));
    m_timeoutSpin->setMinimumWidth(100);
    timeoutLayout->addWidget(m_timeoutLabel);
    timeoutLayout->addWidget(m_timeoutSpin);
    timeoutLayout->addStretch();
    settingsLayout->addLayout(timeoutLayout);

    settingsLayout->addSpacing(8);

    // Buttons
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    m_saveButton = new QPushButton(tr("保存设置"), m_settingsCard);
    m_saveButton->setMinimumHeight(32);
    m_saveButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    buttonLayout->addWidget(m_saveButton);

    m_testButton = new QPushButton(tr("测试搜索"), m_settingsCard);
    m_testButton->setMinimumHeight(32);
    m_testButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    buttonLayout->addWidget(m_testButton);

    buttonLayout->addStretch();
    settingsLayout->addLayout(buttonLayout);

    contentLayout->addWidget(m_settingsCard);

    contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);
}

void WebSearchConfigPage::connectSignals()
{
    connect(m_saveButton, &QPushButton::clicked, this, &WebSearchConfigPage::onSaveClicked);
    connect(m_testButton, &QPushButton::clicked, this, &WebSearchConfigPage::onTestSearchClicked);
    connect(m_providerCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &WebSearchConfigPage::onProviderChanged);
    connect(m_testService, &WebSearchService::searchFinished,
            this, [this](const QString &reqId, const QString &query, const QList<WebSearchResult> &results) {
        onSearchFinished(reqId, query, results.size());
    });
    connect(m_testService, &WebSearchService::searchFailed,
            this, &WebSearchConfigPage::onSearchFailed);
}

void WebSearchConfigPage::loadConfig()
{
    WebSearchConfig config = WebSearchSettingsService::load();

    m_enabledCheckBox->setChecked(config.enabled);

    int providerIndex = m_providerCombo->findData(static_cast<int>(config.provider));
    if (providerIndex >= 0) {
        m_providerCombo->setCurrentIndex(providerIndex);
    }

    m_apiKeyEdit->setText(config.apiKey);
    m_resultCountSpin->setValue(config.resultCount);

    int depthIndex = m_searchDepthCombo->findData(config.searchDepth);
    if (depthIndex >= 0) {
        m_searchDepthCombo->setCurrentIndex(depthIndex);
    }

    m_timeoutSpin->setValue(config.timeoutMs / 1000);

    onProviderChanged(m_providerCombo->currentIndex());
}

void WebSearchConfigPage::updateStatusDisplay()
{
    ThemePalette p = ThemeManager::instance().currentPalette();

    WebSearchConfig config = WebSearchSettingsService::load();

    if (config.enabled) {
        m_statusLabel->setText(tr("状态：已启用"));
        m_statusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.accent));
    } else {
        m_statusLabel->setText(tr("状态：未启用"));
        m_statusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.textSecondary));
    }
}

void WebSearchConfigPage::onSaveClicked()
{
    WebSearchConfig config;
    config.enabled = m_enabledCheckBox->isChecked();
    config.provider = static_cast<WebSearchProvider>(m_providerCombo->currentData().toInt());
    config.apiKey = m_apiKeyEdit->text().trimmed();
    config.resultCount = m_resultCountSpin->value();
    config.searchDepth = m_searchDepthCombo->currentData().toString();
    config.timeoutMs = m_timeoutSpin->value() * 1000;

    QString error;
    if (!WebSearchSettingsService::save(config, &error)) {
        SoftMessageBox::showWarning(this, tr("搜索设置"), tr("保存失败：%1").arg(error));
        return;
    }

    updateStatusDisplay();
    SoftMessageBox::showInformation(this, tr("搜索设置"), tr("搜索设置已保存。"));
}

void WebSearchConfigPage::onReloadClicked()
{
    QString error;
    WebSearchConfig config = WebSearchSettingsService::load(&error);

    if (!error.isEmpty()) {
        SoftMessageBox::showWarning(this, tr("搜索设置"),
                                    tr("重新加载搜索设置失败：%1").arg(error));
        return;
    }

    m_enabledCheckBox->setChecked(config.enabled);

    int providerIndex = m_providerCombo->findData(static_cast<int>(config.provider));
    if (providerIndex >= 0) {
        m_providerCombo->setCurrentIndex(providerIndex);
    }

    m_apiKeyEdit->setText(config.apiKey);
    m_resultCountSpin->setValue(config.resultCount);

    int depthIndex = m_searchDepthCombo->findData(config.searchDepth);
    if (depthIndex >= 0) {
        m_searchDepthCombo->setCurrentIndex(depthIndex);
    }

    m_timeoutSpin->setValue(config.timeoutMs / 1000);

    onProviderChanged(m_providerCombo->currentIndex());
    updateStatusDisplay();

    SoftMessageBox::showInformation(this, tr("搜索设置"),
                                    tr("搜索设置已重新加载。"));
}

void WebSearchConfigPage::onTestSearchClicked()
{
    if (m_testPending) return;

    if (m_apiKeyEdit->text().trimmed().isEmpty()) {
        SoftMessageBox::showWarning(this, tr("搜索设置"), tr("请先填写搜索 API Key。"));
        return;
    }

    WebSearchConfig config;
    config.enabled = true;
    config.provider = static_cast<WebSearchProvider>(m_providerCombo->currentData().toInt());
    config.apiKey = m_apiKeyEdit->text().trimmed();
    config.resultCount = m_resultCountSpin->value();
    config.searchDepth = m_searchDepthCombo->currentData().toString();
    config.timeoutMs = m_timeoutSpin->value() * 1000;

    m_testPending = true;
    m_testButton->setEnabled(false);
    m_testButton->setText(tr("测试中..."));

    m_testRequestId = m_testService->search("DesktopPet Qt", config);
}

void WebSearchConfigPage::onProviderChanged(int index)
{
    Q_UNUSED(index);
    WebSearchProvider provider = static_cast<WebSearchProvider>(m_providerCombo->currentData().toInt());

    // Brave doesn't use searchDepth
    bool depthVisible = (provider != WebSearchProvider::Brave);
    m_searchDepthLabel->setVisible(depthVisible);
    m_searchDepthCombo->setVisible(depthVisible);
}

void WebSearchConfigPage::onSearchFinished(const QString &requestId, const QString &query, int resultCount)
{
    Q_UNUSED(query);
    if (requestId != m_testRequestId) return;

    m_testPending = false;
    m_testRequestId.clear();
    m_testButton->setEnabled(true);
    m_testButton->setText(tr("测试搜索"));

    SoftMessageBox::showInformation(this, tr("测试搜索"),
                                    tr("搜索测试成功，返回 %1 条结果。").arg(resultCount));
}

void WebSearchConfigPage::onSearchFailed(const QString &requestId, const QString &query, const QString &message)
{
    Q_UNUSED(query);
    if (requestId != m_testRequestId) return;

    m_testPending = false;
    m_testRequestId.clear();
    m_testButton->setEnabled(true);
    m_testButton->setText(tr("测试搜索"));

    SoftMessageBox::showWarning(this, tr("测试搜索"),
                                tr("搜索测试失败：%1").arg(SecretStorageService::redactSecrets(message)));
}

void WebSearchConfigPage::refreshTheme()
{
    applyTheme();
}

void WebSearchConfigPage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());
    m_scrollArea->setStyleSheet(theme.softScrollAreaStyleSheet());
    m_contentWidget->setStyleSheet(theme.softPageStyleSheet());

    m_statusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.accent));

    m_settingsCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.subtitleText));
    m_providerLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.textPrimary));
    m_apiKeyLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.textSecondary));
    m_resultCountLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.textSecondary));
    m_searchDepthLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.textSecondary));
    m_timeoutLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;").arg(p.textSecondary));

    m_enabledCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_saveButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_testButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    m_reloadButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));

    updateStatusDisplay();
}
