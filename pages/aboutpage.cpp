#include "aboutpage.h"

#include "core/appversion.h"
#include "core/updatemanager.h"
#include "theme/thememanager.h"

#include <QDesktopServices>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPixmap>
#include <QUrl>
#include <QVBoxLayout>

AboutPage::AboutPage(QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(nullptr)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_infoCard(nullptr)
    , m_infoCardTitle(nullptr)
    , m_iconLabel(nullptr)
    , m_appNameLabel(nullptr)
    , m_descriptionLabel(nullptr)
    , m_versionCard(nullptr)
    , m_versionCardTitle(nullptr)
    , m_versionLabel(nullptr)
    , m_updateCard(nullptr)
    , m_updateCardTitle(nullptr)
    , m_updateStatusLabel(nullptr)
    , m_checkUpdateButton(nullptr)
    , m_updateManager(nullptr)
{
    setupUi();
    m_updateManager = new UpdateManager(this);
    connect(m_updateManager, &UpdateManager::updateAvailable,
            this, &AboutPage::onUpdateAvailable);
    connect(m_updateManager, &UpdateManager::noUpdateAvailable,
            this, &AboutPage::onNoUpdateAvailable);
    connect(m_updateManager, &UpdateManager::checkFailed,
            this, &AboutPage::onCheckFailed);
}

AboutPage::~AboutPage() {}

void AboutPage::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_titleLabel = new QLabel(tr("关于"), this);
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

    m_infoCard = new QFrame(m_contentWidget);
    m_infoCard->setObjectName("infoCard");
    m_infoCard->setStyleSheet(theme.cardStyleSheet("infoCard"));
    QVBoxLayout *infoLayout = new QVBoxLayout(m_infoCard);
    infoLayout->setContentsMargins(24, 24, 24, 24);
    infoLayout->setSpacing(12);

    m_infoCardTitle = new QLabel(tr("项目信息"), m_infoCard);
    QFont infoTitleFont = m_infoCardTitle->font();
    infoTitleFont.setPointSize(12);
    infoTitleFont.setBold(true);
    m_infoCardTitle->setFont(infoTitleFont);
    m_infoCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                     .arg(theme.textPrimaryColor()));
    infoLayout->addWidget(m_infoCardTitle);

    QHBoxLayout *iconTextLayout = new QHBoxLayout();
    iconTextLayout->setSpacing(16);

    m_iconLabel = new QLabel(m_infoCard);
    QPixmap iconPixmap(":/icons/app_icon.png");
    m_iconLabel->setPixmap(iconPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation));
    m_iconLabel->setFixedSize(64, 64);
    m_iconLabel->setScaledContents(true);
    iconTextLayout->addWidget(m_iconLabel, 0, Qt::AlignTop);

    QVBoxLayout *textLayout = new QVBoxLayout();
    textLayout->setSpacing(8);

    m_appNameLabel = new QLabel(tr("DesktopPet"), m_infoCard);
    QFont appNameFont = m_appNameLabel->font();
    appNameFont.setPointSize(14);
    appNameFont.setBold(true);
    m_appNameLabel->setFont(appNameFont);
    m_appNameLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.buttonPrimaryColor()));
    textLayout->addWidget(m_appNameLabel);

    m_descriptionLabel = new QLabel(tr("一个可爱的桌面宠物应用，让你的桌面更生动有趣。"), m_infoCard);
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(theme.textSecondaryColor()));
    textLayout->addWidget(m_descriptionLabel);

    iconTextLayout->addLayout(textLayout, 1);
    infoLayout->addLayout(iconTextLayout);

    contentLayout->addWidget(m_infoCard);

    m_versionCard = new QFrame(m_contentWidget);
    m_versionCard->setObjectName("versionCard");
    m_versionCard->setStyleSheet(theme.cardStyleSheet("versionCard"));
    QVBoxLayout *versionLayout = new QVBoxLayout(m_versionCard);
    versionLayout->setContentsMargins(24, 24, 24, 24);
    versionLayout->setSpacing(12);

    m_versionCardTitle = new QLabel(tr("版本信息"), m_versionCard);
    QFont versionTitleFont = m_versionCardTitle->font();
    versionTitleFont.setPointSize(12);
    versionTitleFont.setBold(true);
    m_versionCardTitle->setFont(versionTitleFont);
    m_versionCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(theme.textPrimaryColor()));
    versionLayout->addWidget(m_versionCardTitle);

    m_versionLabel = new QLabel(tr("当前版本：") + QString::fromUtf8(APP_VERSION), m_versionCard);
    m_versionLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textSecondaryColor()));
    versionLayout->addWidget(m_versionLabel);

    contentLayout->addWidget(m_versionCard);

    m_updateCard = new QFrame(m_contentWidget);
    m_updateCard->setObjectName("updateCard");
    m_updateCard->setStyleSheet(theme.cardStyleSheet("updateCard"));
    QVBoxLayout *updateLayout = new QVBoxLayout(m_updateCard);
    updateLayout->setContentsMargins(24, 24, 24, 24);
    updateLayout->setSpacing(12);

    m_updateCardTitle = new QLabel(tr("检查更新"), m_updateCard);
    QFont updateTitleFont = m_updateCardTitle->font();
    updateTitleFont.setPointSize(12);
    updateTitleFont.setBold(true);
    m_updateCardTitle->setFont(updateTitleFont);
    m_updateCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(theme.textPrimaryColor()));
    updateLayout->addWidget(m_updateCardTitle);

    m_updateStatusLabel = new QLabel(m_updateCard);
    m_updateStatusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(theme.textSecondaryColor()));
    updateLayout->addWidget(m_updateStatusLabel);

    m_checkUpdateButton = new QPushButton(tr("检查更新"), m_updateCard);
    m_checkUpdateButton->setMinimumHeight(36);
    m_checkUpdateButton->setStyleSheet(theme.primaryButtonStyleSheet());
    connect(m_checkUpdateButton, &QPushButton::clicked, this, &AboutPage::onCheckUpdateClicked);
    updateLayout->addWidget(m_checkUpdateButton);

    contentLayout->addWidget(m_updateCard);
    contentLayout->addStretch();

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);
}

void AboutPage::refreshTheme()
{
    applyTheme();
}

void AboutPage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();

    m_titleLabel->setStyleSheet(theme.titleLabelStyleSheet());
    m_scrollArea->setStyleSheet(theme.scrollAreaStyleSheet());
    m_contentWidget->setStyleSheet(theme.pageStyleSheet());

    m_infoCard->setStyleSheet(theme.cardStyleSheet("infoCard"));
    m_infoCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                     .arg(theme.textPrimaryColor()));
    m_appNameLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.buttonPrimaryColor()));
    m_descriptionLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(theme.textSecondaryColor()));

    m_versionCard->setStyleSheet(theme.cardStyleSheet("versionCard"));
    m_versionCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(theme.textPrimaryColor()));
    m_versionLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textSecondaryColor()));

    m_updateCard->setStyleSheet(theme.cardStyleSheet("updateCard"));
    m_updateCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(theme.textPrimaryColor()));
    m_updateStatusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(theme.textSecondaryColor()));
    m_checkUpdateButton->setStyleSheet(theme.primaryButtonStyleSheet());
}

void AboutPage::onCheckUpdateClicked()
{
    m_checkUpdateButton->setEnabled(false);
    m_updateStatusLabel->setText(tr("正在检查更新..."));
    m_updateManager->checkForUpdates();
}

void AboutPage::onUpdateAvailable(const QString &latestVersion, const QString &releaseUrl)
{
    m_checkUpdateButton->setEnabled(true);
    m_updateStatusLabel->setText(tr("发现新版本：%1").arg(latestVersion));

    QMessageBox::StandardButton reply = QMessageBox::question(
        this,
        tr("发现新版本"),
        tr("发现新版本 %1，是否打开下载页面？").arg(latestVersion),
        QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        QDesktopServices::openUrl(QUrl(releaseUrl));
    }
}

void AboutPage::onNoUpdateAvailable(const QString &latestVersion)
{
    m_checkUpdateButton->setEnabled(true);
    m_updateStatusLabel->setText(tr("已是最新版本：%1").arg(latestVersion));
}

void AboutPage::onCheckFailed(const QString &errorMessage)
{
    m_checkUpdateButton->setEnabled(true);
    m_updateStatusLabel->setText(tr("检查更新失败：%1").arg(errorMessage));
}
