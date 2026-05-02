#include "apiconfigpage.h"

#include "theme/thememanager.h"

#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QMessageBox>
#include <QVBoxLayout>

ApiConfigPage::ApiConfigPage(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_titleLabel(nullptr)
    , m_statusCard(nullptr)
    , m_editorCard(nullptr)
    , m_profilesCard(nullptr)
    , m_statusCardTitle(nullptr)
    , m_editorCardTitle(nullptr)
    , m_profilesCardTitle(nullptr)
    , m_apiKeyLabel(nullptr)
    , m_baseUrlLabel(nullptr)
    , m_modelLabel(nullptr)
    , m_apiKeyEdit(nullptr)
    , m_baseUrlEdit(nullptr)
    , m_modelEdit(nullptr)
    , m_saveApiConfigButton(nullptr)
    , m_apiProfileList(nullptr)
    , m_addApiProfileButton(nullptr)
    , m_removeApiProfileButton(nullptr)
    , m_currentApiProfileLabel(nullptr)
{
    setupUi();
    connectSignals();
}

ApiConfigPage::~ApiConfigPage() {}

void ApiConfigPage::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_titleLabel = new QLabel(tr("API配置"), this);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(theme.titleLabelStyleSheet());
    m_titleLabel->setMargin(0);
    outerLayout->addWidget(m_titleLabel);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(theme.scrollAreaStyleSheet());

    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet(theme.pageStyleSheet());

    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(28, 20, 28, 28);
    contentLayout->setSpacing(20);

    m_statusCard = new QFrame(m_contentWidget);
    m_statusCard->setObjectName("statusCard");
    m_statusCard->setStyleSheet(theme.cardStyleSheet("statusCard"));
    QVBoxLayout *statusLayout = new QVBoxLayout(m_statusCard);
    statusLayout->setContentsMargins(24, 20, 24, 20);
    statusLayout->setSpacing(10);

    m_statusCardTitle = new QLabel(tr("当前状态"), m_statusCard);
    QFont statusTitleFont = m_statusCardTitle->font();
    statusTitleFont.setPointSize(12);
    statusTitleFont.setBold(true);
    m_statusCardTitle->setFont(statusTitleFont);
    m_statusCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(theme.textPrimaryColor()));
    statusLayout->addWidget(m_statusCardTitle);

    m_currentApiProfileLabel = new QLabel(tr("当前配置：未选择"), m_statusCard);
    m_currentApiProfileLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; font-weight: bold; border: none; background: transparent;")
            .arg(theme.buttonPrimaryColor()));
    statusLayout->addWidget(m_currentApiProfileLabel);

    contentLayout->addWidget(m_statusCard);

    m_editorCard = new QFrame(m_contentWidget);
    m_editorCard->setObjectName("editorCard");
    m_editorCard->setStyleSheet(theme.cardStyleSheet("editorCard"));
    QVBoxLayout *editorLayout = new QVBoxLayout(m_editorCard);
    editorLayout->setContentsMargins(24, 24, 24, 24);
    editorLayout->setSpacing(16);

    m_editorCardTitle = new QLabel(tr("配置编辑"), m_editorCard);
    QFont editorTitleFont = m_editorCardTitle->font();
    editorTitleFont.setPointSize(12);
    editorTitleFont.setBold(true);
    m_editorCardTitle->setFont(editorTitleFont);
    m_editorCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(theme.textPrimaryColor()));
    editorLayout->addWidget(m_editorCardTitle);

    m_apiKeyLabel = new QLabel(tr("API Key:"), m_editorCard);
    m_apiKeyLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(theme.textSecondaryColor()));
    editorLayout->addWidget(m_apiKeyLabel);

    m_apiKeyEdit = new QLineEdit(m_editorCard);
    m_apiKeyEdit->setEchoMode(QLineEdit::Password);
    m_apiKeyEdit->setPlaceholderText(tr("请输入 API Key"));
    m_apiKeyEdit->setMinimumHeight(36);
    m_apiKeyEdit->setStyleSheet(theme.lineEditStyleSheet());
    editorLayout->addWidget(m_apiKeyEdit);

    m_baseUrlLabel = new QLabel(tr("Base URL:"), m_editorCard);
    m_baseUrlLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textSecondaryColor()));
    editorLayout->addWidget(m_baseUrlLabel);

    m_baseUrlEdit = new QLineEdit(m_editorCard);
    m_baseUrlEdit->setPlaceholderText(tr("请输入 Base URL"));
    m_baseUrlEdit->setMinimumHeight(36);
    m_baseUrlEdit->setStyleSheet(theme.lineEditStyleSheet());
    editorLayout->addWidget(m_baseUrlEdit);

    m_modelLabel = new QLabel(tr("Model:"), m_editorCard);
    m_modelLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                  .arg(theme.textSecondaryColor()));
    editorLayout->addWidget(m_modelLabel);

    m_modelEdit = new QLineEdit(m_editorCard);
    m_modelEdit->setPlaceholderText(tr("请输入模型名称"));
    m_modelEdit->setMinimumHeight(36);
    m_modelEdit->setStyleSheet(theme.lineEditStyleSheet());
    editorLayout->addWidget(m_modelEdit);

    m_saveApiConfigButton = new QPushButton(tr("保存配置"), m_editorCard);
    m_saveApiConfigButton->setMinimumHeight(36);
    m_saveApiConfigButton->setStyleSheet(theme.primaryButtonStyleSheet());
    editorLayout->addWidget(m_saveApiConfigButton);

    contentLayout->addWidget(m_editorCard);

    m_profilesCard = new QFrame(m_contentWidget);
    m_profilesCard->setObjectName("profilesCard");
    m_profilesCard->setStyleSheet(theme.cardStyleSheet("profilesCard"));
    QVBoxLayout *profilesLayout = new QVBoxLayout(m_profilesCard);
    profilesLayout->setContentsMargins(24, 24, 24, 24);
    profilesLayout->setSpacing(16);

    m_profilesCardTitle = new QLabel(tr("已有配置"), m_profilesCard);
    QFont profilesTitleFont = m_profilesCardTitle->font();
    profilesTitleFont.setPointSize(12);
    profilesTitleFont.setBold(true);
    m_profilesCardTitle->setFont(profilesTitleFont);
    m_profilesCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                     .arg(theme.textPrimaryColor()));
    profilesLayout->addWidget(m_profilesCardTitle);

    m_apiProfileList = new QListWidget(m_profilesCard);
    m_apiProfileList->setMinimumHeight(160);
    m_apiProfileList->setStyleSheet(theme.listWidgetStyleSheet());
    profilesLayout->addWidget(m_apiProfileList, 1);

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    m_addApiProfileButton = new QPushButton(tr("新增配置"), m_profilesCard);
    m_addApiProfileButton->setMinimumHeight(36);
    m_addApiProfileButton->setStyleSheet(theme.secondaryButtonStyleSheet());

    m_removeApiProfileButton = new QPushButton(tr("删除配置"), m_profilesCard);
    m_removeApiProfileButton->setMinimumHeight(36);
    m_removeApiProfileButton->setStyleSheet(theme.dangerButtonStyleSheet());

    buttonLayout->addWidget(m_addApiProfileButton);
    buttonLayout->addWidget(m_removeApiProfileButton);
    buttonLayout->addStretch();

    profilesLayout->addLayout(buttonLayout);
    contentLayout->addWidget(m_profilesCard, 1);

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);
}

void ApiConfigPage::connectSignals()
{
    connect(m_addApiProfileButton, &QPushButton::clicked, this, &ApiConfigPage::onAddApiProfile);
    connect(m_removeApiProfileButton, &QPushButton::clicked, this, &ApiConfigPage::onRemoveApiProfile);
    connect(m_apiProfileList, &QListWidget::currentRowChanged, this, &ApiConfigPage::onApiProfileSelectionChanged);
    connect(m_saveApiConfigButton, &QPushButton::clicked, this, &ApiConfigPage::onSaveApiConfigClicked);
}

void ApiConfigPage::refreshTheme()
{
    applyTheme();
}

void ApiConfigPage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();

    m_titleLabel->setStyleSheet(theme.titleLabelStyleSheet());
    m_scrollArea->setStyleSheet(theme.scrollAreaStyleSheet());
    m_contentWidget->setStyleSheet(theme.pageStyleSheet());

    m_statusCard->setStyleSheet(theme.cardStyleSheet("statusCard"));
    m_editorCard->setStyleSheet(theme.cardStyleSheet("editorCard"));
    m_profilesCard->setStyleSheet(theme.cardStyleSheet("profilesCard"));

    m_statusCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(theme.textPrimaryColor()));
    m_editorCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(theme.textPrimaryColor()));
    m_profilesCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(theme.textPrimaryColor()));

    m_apiKeyLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(theme.textSecondaryColor()));
    m_baseUrlLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textSecondaryColor()));
    m_modelLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                  .arg(theme.textSecondaryColor()));

    m_currentApiProfileLabel->setStyleSheet(
        QString("color: %1; font-size: 14px; font-weight: bold; border: none; background: transparent;")
            .arg(theme.buttonPrimaryColor()));

    m_apiKeyEdit->setStyleSheet(theme.lineEditStyleSheet());
    m_baseUrlEdit->setStyleSheet(theme.lineEditStyleSheet());
    m_modelEdit->setStyleSheet(theme.lineEditStyleSheet());
    m_saveApiConfigButton->setStyleSheet(theme.primaryButtonStyleSheet());
    m_addApiProfileButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_removeApiProfileButton->setStyleSheet(theme.dangerButtonStyleSheet());
    m_apiProfileList->setStyleSheet(theme.listWidgetStyleSheet());
}

void ApiConfigPage::clearApiEditor()
{
    m_apiKeyEdit->clear();
    m_baseUrlEdit->clear();
    m_modelEdit->clear();
}

void ApiConfigPage::loadApiConfigToEditor(const QString &profileName)
{
    if (!m_apiConfigs.contains(profileName)) {
        clearApiEditor();
        m_currentApiProfileLabel->setText(tr("当前配置：未选择"));
        m_currentApiProfile.clear();
        return;
    }

    const ApiConfig &config = m_apiConfigs[profileName];
    m_apiKeyEdit->setText(config.apiKey);
    m_baseUrlEdit->setText(config.baseUrl);
    m_modelEdit->setText(config.model);

    m_currentApiProfile = profileName;
    m_currentApiProfileLabel->setText(tr("当前配置：%1").arg(profileName));
}

void ApiConfigPage::saveCurrentApiConfig()
{
    if (m_currentApiProfile.isEmpty()) {
        QMessageBox::warning(this, tr("提示"), tr("请先新增或选择一个配置。"));
        return;
    }

    ApiConfig config;
    config.apiKey = m_apiKeyEdit->text();
    config.baseUrl = m_baseUrlEdit->text();
    config.model = m_modelEdit->text();

    m_apiConfigs[m_currentApiProfile] = config;
}

void ApiConfigPage::onAddApiProfile()
{
    bool ok = false;
    QString profileName = QInputDialog::getText(this,
                                                tr("新增配置"),
                                                tr("请输入配置名称："),
                                                QLineEdit::Normal,
                                                "",
                                                &ok);

    if (!ok || profileName.trimmed().isEmpty()) {
        return;
    }

    profileName = profileName.trimmed();

    if (m_apiConfigs.contains(profileName)) {
        QMessageBox::warning(this, tr("提示"), tr("配置名称已存在。"));
        return;
    }

    m_apiConfigs[profileName] = ApiConfig{};
    m_apiProfileList->addItem(profileName);
    m_apiProfileList->setCurrentRow(m_apiProfileList->count() - 1);
}

void ApiConfigPage::onRemoveApiProfile()
{
    QListWidgetItem *currentItem = m_apiProfileList->currentItem();
    if (!currentItem) {
        QMessageBox::information(this, tr("提示"), tr("请先选择一个配置。"));
        return;
    }

    QString profileName = currentItem->text();
    delete m_apiProfileList->takeItem(m_apiProfileList->row(currentItem));
    m_apiConfigs.remove(profileName);

    if (m_apiProfileList->count() > 0) {
        m_apiProfileList->setCurrentRow(0);
    } else {
        clearApiEditor();
        m_currentApiProfile.clear();
        m_currentApiProfileLabel->setText(tr("当前配置：未选择"));
    }
}

void ApiConfigPage::onApiProfileSelectionChanged()
{
    QListWidgetItem *currentItem = m_apiProfileList->currentItem();
    if (!currentItem) {
        clearApiEditor();
        m_currentApiProfile.clear();
        m_currentApiProfileLabel->setText(tr("当前配置：未选择"));
        return;
    }

    loadApiConfigToEditor(currentItem->text());
}

void ApiConfigPage::onSaveApiConfigClicked()
{
    saveCurrentApiConfig();
    if (!m_currentApiProfile.isEmpty()) {
        QMessageBox::information(this, tr("提示"), tr("配置已保存：%1").arg(m_currentApiProfile));
    }
}
