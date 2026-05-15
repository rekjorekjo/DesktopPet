#include "apiconfigpage.h"

#include "core/appsettings.h"
#include "dialogs/apiconfigdialog.h"
#include "theme/thememanager.h"
#include "widgets/softmessagebox.h"

#include <QFont>
#include <QHBoxLayout>
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
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
    setupUi();
    connectSignals();
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

    m_titleLabel = new QLabel(tr("API配置"), m_contentWidget);
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
    QVBoxLayout *statusLayout = new QVBoxLayout(m_statusCard);
    statusLayout->setContentsMargins(24, 20, 24, 20);
    statusLayout->setSpacing(10);

    m_currentApiProfileLabel = new QLabel(tr("当前配置：未选择"), m_statusCard);
    QFont statusTitleFont = m_currentApiProfileLabel->font();
    statusTitleFont.setPointSize(12);
    statusTitleFont.setBold(true);
    m_currentApiProfileLabel->setFont(statusTitleFont);
    m_currentApiProfileLabel->setStyleSheet(
        QString("color: %1; border: none; background: transparent;")
            .arg(p.accent));
    statusLayout->addWidget(m_currentApiProfileLabel);

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

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);
}

void ApiConfigPage::connectSignals()
{
    connect(m_addApiProfileButton, &QPushButton::clicked, this, &ApiConfigPage::onAddApiProfile);
    connect(m_apiProfileList, &QListWidget::currentRowChanged, this, &ApiConfigPage::onApiProfileSelectionChanged);
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
}

void ApiConfigPage::updateEmptyState()
{
    bool empty = m_apiConfigs.isEmpty();
    m_emptyLabel->setVisible(empty);
    m_apiProfileList->setVisible(!empty);
}

void ApiConfigPage::updateCurrentProfileDisplay()
{
    if (m_currentApiProfile.isEmpty() || !m_apiConfigs.contains(m_currentApiProfile)) {
        m_currentApiProfile.clear();
        m_currentApiProfileLabel->setText(tr("当前配置：未选择"));
        AppSettings::setCurrentApiConfigName("");
    } else {
        m_currentApiProfileLabel->setText(tr("当前配置：%1").arg(m_currentApiProfile));
        AppSettings::setCurrentApiConfigName(m_currentApiProfile);
    }
}

void ApiConfigPage::refreshProfileList()
{
    m_apiProfileList->clear();

    for (auto it = m_apiConfigs.constBegin(); it != m_apiConfigs.constEnd(); ++it) {
        const QString &name = it.key();

        QListWidgetItem *item = new QListWidgetItem(m_apiProfileList);
        item->setText(name);
        item->setSizeHint(QSize(0, 64));
        m_apiProfileList->setItemWidget(item, createProfileRowWidget(name));
    }

    updateEmptyState();
}

QWidget *ApiConfigPage::createProfileRowWidget(const QString &profileName)
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    const ApiConfig &config = m_apiConfigs[profileName];
    bool isCurrent = (profileName == m_currentApiProfile);

    QWidget *rowWidget = new QWidget();
    rowWidget->setObjectName("profileRowWidget");
    QVBoxLayout *rowLayout = new QVBoxLayout(rowWidget);
    rowLayout->setContentsMargins(12, 6, 8, 6);
    rowLayout->setSpacing(4);

    // Top row: name + current marker + buttons
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(8);

    QLabel *nameLabel = new QLabel(profileName, rowWidget);
    nameLabel->setStyleSheet(QString("color: %1; background: transparent; border: none; font-size: 13px; font-weight: bold;")
                                 .arg(p.textPrimary));
    topLayout->addWidget(nameLabel, 1);

    if (isCurrent) {
        QLabel *currentLabel = new QLabel(tr("当前"), rowWidget);
        currentLabel->setStyleSheet(QString(
            "color: %1; background: %2; border: none; font-size: 11px; font-weight: bold;"
            "padding: 2px 8px; border-radius: 4px;"
        ).arg(p.buttonPrimaryText, p.accent));
        topLayout->addWidget(currentLabel);
    }

    QPushButton *editBtn = new QPushButton(tr("编辑"), rowWidget);
    editBtn->setStyleSheet(theme.softSecondaryButtonStyleSheet(4, 20));
    editBtn->setFixedSize(52, 28);
    connect(editBtn, &QPushButton::clicked, this, [this, rowWidget]() {
        for (int i = 0; i < m_apiProfileList->count(); ++i) {
            if (m_apiProfileList->itemWidget(m_apiProfileList->item(i)) == rowWidget) {
                onEditApiProfile(i);
                return;
            }
        }
    });
    topLayout->addWidget(editBtn);

    QPushButton *delBtn = new QPushButton(tr("删除"), rowWidget);
    delBtn->setStyleSheet(theme.dangerButtonStyleSheet());
    delBtn->setFixedSize(52, 28);
    connect(delBtn, &QPushButton::clicked, this, [this, rowWidget]() {
        for (int i = 0; i < m_apiProfileList->count(); ++i) {
            if (m_apiProfileList->itemWidget(m_apiProfileList->item(i)) == rowWidget) {
                onRemoveApiProfile(i);
                return;
            }
        }
    });
    topLayout->addWidget(delBtn);

    rowLayout->addLayout(topLayout);

    // Bottom row: baseUrl + model
    QHBoxLayout *bottomLayout = new QHBoxLayout();
    bottomLayout->setSpacing(12);

    QString urlText = config.baseUrl.isEmpty() ? tr("(未设置 Base URL)") : config.baseUrl;
    QLabel *urlLabel = new QLabel(urlText, rowWidget);
    urlLabel->setStyleSheet(QString("color: %1; background: transparent; border: none; font-size: 11px;")
                                .arg(p.textSecondary));
    urlLabel->setMaximumWidth(260);
    urlLabel->setTextInteractionFlags(Qt::TextSelectableByMouse);
    bottomLayout->addWidget(urlLabel, 1);

    QString modelText = config.model.isEmpty() ? tr("(未设置 Model)") : config.model;
    QLabel *modelLabel = new QLabel(modelText, rowWidget);
    modelLabel->setStyleSheet(QString("color: %1; background: transparent; border: none; font-size: 11px;")
                                  .arg(p.textSecondary));
    bottomLayout->addWidget(modelLabel, 1);

    rowLayout->addLayout(bottomLayout);

    return rowWidget;
}

void ApiConfigPage::onAddApiProfile()
{
    QString profileName;
    ApiConfig config;

    bool ok = ApiConfigDialog::getNewProfile(this,
                                             m_apiConfigs.keys(),
                                             &profileName,
                                             &config);
    if (!ok) {
        return;
    }

    m_apiConfigs[profileName] = config;
    m_currentApiProfile = profileName;

    refreshProfileList();
    updateCurrentProfileDisplay();

    for (int i = 0; i < m_apiProfileList->count(); ++i) {
        if (m_apiProfileList->item(i)->text() == profileName) {
            m_apiProfileList->setCurrentRow(i);
            break;
        }
    }
}

void ApiConfigPage::onEditApiProfile(int row)
{
    QListWidgetItem *item = m_apiProfileList->item(row);
    if (!item) return;

    QString profileName = item->text();
    if (!m_apiConfigs.contains(profileName)) return;

    ApiConfig config;
    bool ok = ApiConfigDialog::editProfile(this, profileName, m_apiConfigs.value(profileName), &config);
    if (!ok) return;

    m_apiConfigs[profileName] = config;
    refreshProfileList();
}

void ApiConfigPage::onRemoveApiProfile(int row)
{
    QListWidgetItem *item = m_apiProfileList->item(row);
    if (!item) return;

    QString profileName = item->text();

    bool yes = SoftMessageBox::question(this,
                                        tr("确认删除"),
                                        tr("确定要删除配置「%1」吗？").arg(profileName))
               == SoftMessageBox::Yes;
    if (!yes) return;

    m_apiConfigs.remove(profileName);

    if (m_currentApiProfile == profileName) {
        if (!m_apiConfigs.isEmpty()) {
            m_currentApiProfile = m_apiConfigs.constBegin().key();
        } else {
            m_currentApiProfile.clear();
        }
    }

    refreshProfileList();
    updateCurrentProfileDisplay();

    if (!m_currentApiProfile.isEmpty()) {
        for (int i = 0; i < m_apiProfileList->count(); ++i) {
            if (m_apiProfileList->item(i)->text() == m_currentApiProfile) {
                m_apiProfileList->setCurrentRow(i);
                break;
            }
        }
    }
}

void ApiConfigPage::onApiProfileSelectionChanged()
{
    QListWidgetItem *currentItem = m_apiProfileList->currentItem();
    if (!currentItem) {
        return;
    }

    QString profileName = currentItem->text();
    if (!m_apiConfigs.contains(profileName)) {
        return;
    }

    m_currentApiProfile = profileName;
    updateCurrentProfileDisplay();
    refreshProfileList();
}
