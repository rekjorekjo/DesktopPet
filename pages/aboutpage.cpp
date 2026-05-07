#include "aboutpage.h"

#include "core/appversion.h"
#include "theme/thememanager.h"
#include "utils/imageutils.h"
#include "utils/updatemanager.h"

#include <QDebug>
#include <QDesktopServices>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPixmap>
#include <QSizePolicy>
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
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
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

    m_titleLabel = new QLabel(tr("关于"), m_contentWidget);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());
    m_titleLabel->setMargin(0);
    headerLayout->addWidget(m_titleLabel);
    headerLayout->addStretch();
    contentLayout->addLayout(headerLayout);

    m_infoCard = new SoftCardWidget(m_contentWidget);
    m_infoCard->setObjectName("infoCard");
    QVBoxLayout *infoLayout = new QVBoxLayout(m_infoCard);
    infoLayout->setContentsMargins(24, 24, 24, 24);
    infoLayout->setSpacing(12);

    m_infoCardTitle = new QLabel(tr("项目信息"), m_infoCard);
    QFont infoTitleFont = m_infoCardTitle->font();
    infoTitleFont.setPointSize(12);
    infoTitleFont.setBold(true);
    m_infoCardTitle->setFont(infoTitleFont);
    m_infoCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                     .arg(p.subtitleText));
    infoLayout->addWidget(m_infoCardTitle);

    QHBoxLayout *iconTextLayout = new QHBoxLayout();
    iconTextLayout->setSpacing(16);

    m_iconLabel = new QLabel(m_infoCard);
    QPixmap iconPixmap(":/icons/app_icon.png");
    if (iconPixmap.isNull()) {
        qWarning() << "AboutPage: Failed to load app icon from :/icons/app_icon.png";
        iconPixmap = QPixmap(64, 64);
        iconPixmap.fill(Qt::transparent);
    }
    QPixmap roundedIcon = ImageUtils::roundedPixmap(iconPixmap, 64, 12);
    m_iconLabel->setPixmap(roundedIcon.isNull() ? iconPixmap.scaled(64, 64, Qt::KeepAspectRatio, Qt::SmoothTransformation) : roundedIcon);
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
                                    .arg(p.accent));
    textLayout->addWidget(m_appNameLabel);

    m_descriptionLabel = new QLabel(tr("一个可爱的桌面宠物应用，让你的桌面更生动有趣。"), m_infoCard);
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(p.textSecondary));
    textLayout->addWidget(m_descriptionLabel);

    iconTextLayout->addLayout(textLayout, 1);
    infoLayout->addLayout(iconTextLayout);

    contentLayout->addWidget(m_infoCard);

    m_versionCard = new SoftCardWidget(m_contentWidget);
    m_versionCard->setObjectName("versionCard");
    QVBoxLayout *versionLayout = new QVBoxLayout(m_versionCard);
    versionLayout->setContentsMargins(24, 24, 24, 24);
    versionLayout->setSpacing(12);

    m_versionCardTitle = new QLabel(tr("版本信息"), m_versionCard);
    QFont versionTitleFont = m_versionCardTitle->font();
    versionTitleFont.setPointSize(12);
    versionTitleFont.setBold(true);
    m_versionCardTitle->setFont(versionTitleFont);
    m_versionCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(p.subtitleText));
    versionLayout->addWidget(m_versionCardTitle);

    m_versionLabel = new QLabel(tr("当前版本：") + QString::fromUtf8(APP_VERSION), m_versionCard);
    m_versionLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(p.textSecondary));
    versionLayout->addWidget(m_versionLabel);

    contentLayout->addWidget(m_versionCard);

    m_updateCard = new SoftCardWidget(m_contentWidget);
    m_updateCard->setObjectName("updateCard");
    QVBoxLayout *updateLayout = new QVBoxLayout(m_updateCard);
    updateLayout->setContentsMargins(24, 24, 24, 24);
    updateLayout->setSpacing(12);

    m_updateCardTitle = new QLabel(tr("检查更新"), m_updateCard);
    QFont updateTitleFont = m_updateCardTitle->font();
    updateTitleFont.setPointSize(12);
    updateTitleFont.setBold(true);
    m_updateCardTitle->setFont(updateTitleFont);
    m_updateCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(p.subtitleText));
    updateLayout->addWidget(m_updateCardTitle);

    m_updateStatusLabel = new QLabel(m_updateCard);
    m_updateStatusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(p.textSecondary));
    m_updateStatusLabel->setWordWrap(true);
    m_updateStatusLabel->setMaximumHeight(48);
    m_updateStatusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_updateStatusLabel->setVisible(false);
    updateLayout->addWidget(m_updateStatusLabel);

    m_checkUpdateButton = new QPushButton(tr("检查更新"), m_updateCard);
    m_checkUpdateButton->setMinimumHeight(36);
    m_checkUpdateButton->setStyleSheet(theme.softButtonStyleSheet(6, 45));
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
    ThemePalette p = theme.currentPalette();

    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());
    m_scrollArea->setStyleSheet(theme.softScrollAreaStyleSheet());
    m_contentWidget->setStyleSheet(theme.softPageStyleSheet());

    m_infoCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                     .arg(p.subtitleText));
    m_appNameLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(p.accent));
    m_descriptionLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(p.textSecondary));

    m_versionCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(p.subtitleText));
    m_versionLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(p.textSecondary));

    m_updateCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(p.subtitleText));
    m_updateStatusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(p.textSecondary));
    m_checkUpdateButton->setStyleSheet(theme.softButtonStyleSheet(6, 45));
}

void AboutPage::onCheckUpdateClicked()
{
    m_checkUpdateButton->setEnabled(false);
    m_updateStatusLabel->setVisible(true);
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
    QString displayText = elidedStatusText(errorMessage);
    m_updateStatusLabel->setText(displayText);
    if (errorMessage.length() > 80) {
        m_updateStatusLabel->setToolTip(errorMessage);
    } else {
        m_updateStatusLabel->setToolTip(QString());
    }
}

QString AboutPage::elidedStatusText(const QString &text) const
{
    if (text.length() > 80) {
        return text.left(80) + "...";
    }
    return text;
}
