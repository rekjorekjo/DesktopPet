#include "aboutpage.h"

#include "core/appversion.h"

#include <QApplication>
#include "services/updateservice.h"
#include "theme/thememanager.h"
#include "utils/imageutils.h"
#include "widgets/softmessagebox.h"

#include <QDesktopServices>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QPixmap>
#include <QProcess>
#include <QProgressBar>
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
    , m_updateStatusLabel(nullptr)
    , m_latestVersionLabel(nullptr)
    , m_releaseNotesLabel(nullptr)
    , m_checkUpdateButton(nullptr)
    , m_downloadButton(nullptr)
    , m_openReleasePageButton(nullptr)
    , m_downloadProgressBar(nullptr)
    , m_downloadStatusLabel(nullptr)
    , m_updateService(nullptr)
    , m_currentUpdateInfo(nullptr)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
    setupUi();

    m_updateService = new UpdateService(this);
    connect(m_updateService, &UpdateService::checkFinished,
            this, &AboutPage::onCheckFinished);
    connect(m_updateService, &UpdateService::checkFailed,
            this, &AboutPage::onCheckFailed);
    connect(m_updateService, &UpdateService::downloadProgress,
            this, &AboutPage::onDownloadProgress);
    connect(m_updateService, &UpdateService::downloadFinished,
            this, &AboutPage::onDownloadFinished);
    connect(m_updateService, &UpdateService::downloadFailed,
            this, &AboutPage::onDownloadFailed);
}

AboutPage::~AboutPage()
{
    delete m_currentUpdateInfo;
}

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

    // ── Info Card ──
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

    // ── Version Card (merged with update) ──
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

    m_updateStatusLabel = new QLabel(tr("点击下方按钮检查新版本。"), m_versionCard);
    m_updateStatusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(p.textSecondary));
    m_updateStatusLabel->setWordWrap(true);
    m_updateStatusLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    versionLayout->addWidget(m_updateStatusLabel);

    m_latestVersionLabel = new QLabel(m_versionCard);
    m_latestVersionLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                          .arg(p.textSecondary));
    m_latestVersionLabel->setWordWrap(true);
    m_latestVersionLabel->setVisible(false);
    versionLayout->addWidget(m_latestVersionLabel);

    m_releaseNotesLabel = new QLabel(m_versionCard);
    m_releaseNotesLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(p.textSecondary));
    m_releaseNotesLabel->setWordWrap(true);
    m_releaseNotesLabel->setMaximumHeight(120);
    m_releaseNotesLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
    m_releaseNotesLabel->setVisible(false);
    versionLayout->addWidget(m_releaseNotesLabel);

    m_downloadProgressBar = new QProgressBar(m_versionCard);
    m_downloadProgressBar->setMinimum(0);
    m_downloadProgressBar->setMaximum(100);
    m_downloadProgressBar->setVisible(false);
    versionLayout->addWidget(m_downloadProgressBar);

    m_downloadStatusLabel = new QLabel(m_versionCard);
    m_downloadStatusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                           .arg(p.textSecondary));
    m_downloadStatusLabel->setVisible(false);
    versionLayout->addWidget(m_downloadStatusLabel);

    // Buttons — full-width vertical layout
    QVBoxLayout *buttonLayout = new QVBoxLayout();
    buttonLayout->setSpacing(8);

    m_checkUpdateButton = new QPushButton(tr("检查更新"), m_versionCard);
    m_checkUpdateButton->setMinimumHeight(42);
    m_checkUpdateButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_checkUpdateButton->setStyleSheet(theme.softButtonStyleSheet(6, 45));
    connect(m_checkUpdateButton, &QPushButton::clicked, this, &AboutPage::onCheckUpdateClicked);
    buttonLayout->addWidget(m_checkUpdateButton);

    m_downloadButton = new QPushButton(tr("下载并安装"), m_versionCard);
    m_downloadButton->setMinimumHeight(42);
    m_downloadButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_downloadButton->setStyleSheet(theme.softButtonStyleSheet(6, 45));
    m_downloadButton->setVisible(false);
    connect(m_downloadButton, &QPushButton::clicked, this, &AboutPage::onDownloadClicked);
    buttonLayout->addWidget(m_downloadButton);

    m_openReleasePageButton = new QPushButton(tr("打开发布页"), m_versionCard);
    m_openReleasePageButton->setMinimumHeight(42);
    m_openReleasePageButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_openReleasePageButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 45));
    m_openReleasePageButton->setVisible(false);
    connect(m_openReleasePageButton, &QPushButton::clicked, this, &AboutPage::onOpenReleasePageClicked);
    buttonLayout->addWidget(m_openReleasePageButton);

    versionLayout->addLayout(buttonLayout);

    contentLayout->addWidget(m_versionCard);
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
    m_updateStatusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(p.textSecondary));
    m_latestVersionLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                          .arg(p.textSecondary));
    m_releaseNotesLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(p.textSecondary));
    m_downloadStatusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                           .arg(p.textSecondary));
    m_checkUpdateButton->setStyleSheet(theme.softButtonStyleSheet(6, 45));
    m_downloadButton->setStyleSheet(theme.softButtonStyleSheet(6, 45));
    m_openReleasePageButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 45));
}

void AboutPage::onCheckUpdateClicked()
{
    m_checkUpdateButton->setEnabled(false);
    m_updateStatusLabel->setVisible(true);
    m_updateStatusLabel->setText(tr("正在检查更新..."));

    // Hide update-specific UI
    m_latestVersionLabel->setVisible(false);
    m_releaseNotesLabel->setVisible(false);
    m_downloadButton->setVisible(false);
    m_openReleasePageButton->setVisible(false);
    m_downloadProgressBar->setVisible(false);
    m_downloadStatusLabel->setVisible(false);

    m_updateService->checkForUpdates();
}

void AboutPage::onCheckFinished(const UpdateInfo &info)
{
    if (info.updateAvailable) {
        showUpdateAvailable(info);
    } else {
        showNoUpdate();
    }
}

void AboutPage::onCheckFailed(const QString &errorMessage)
{
    resetCheckButton();
    m_updateStatusLabel->setVisible(true);
    m_updateStatusLabel->setText(errorMessage);
}

void AboutPage::showUpdateAvailable(const UpdateInfo &info)
{
    resetCheckButton();

    // Save for later use
    delete m_currentUpdateInfo;
    m_currentUpdateInfo = new UpdateInfo(info);

    m_updateStatusLabel->setVisible(true);
    m_updateStatusLabel->setText(tr("发现新版本：%1").arg(info.latestVersion));

    m_latestVersionLabel->setVisible(true);
    if (!info.releaseName.isEmpty()) {
        m_latestVersionLabel->setText(tr("版本名称：%1").arg(info.releaseName));
    } else {
        m_latestVersionLabel->setText(tr("版本：%1").arg(info.latestVersion));
    }

    // Show release notes (truncated)
    if (!info.releaseNotes.isEmpty()) {
        m_releaseNotesLabel->setVisible(true);
        QString notes = info.releaseNotes;
        if (notes.length() > 500) {
            notes = notes.left(500) + "...";
        }
        m_releaseNotesLabel->setText(notes);
    }

    if (!info.assetName.isEmpty()) {
        m_downloadButton->setVisible(true);
        m_downloadButton->setEnabled(true);
    }

    m_openReleasePageButton->setVisible(true);
}

void AboutPage::showNoUpdate()
{
    resetCheckButton();
    m_updateStatusLabel->setVisible(true);
    m_updateStatusLabel->setText(tr("当前已是最新版本。"));
}

void AboutPage::resetCheckButton()
{
    m_checkUpdateButton->setEnabled(true);
}

void AboutPage::onDownloadClicked()
{
    if (!m_currentUpdateInfo || m_currentUpdateInfo->downloadUrl.isEmpty()) {
        return;
    }

    m_downloadButton->setEnabled(false);
    m_checkUpdateButton->setEnabled(false);
    m_downloadProgressBar->setVisible(true);
    m_downloadProgressBar->setValue(0);
    m_downloadStatusLabel->setVisible(true);
    m_downloadStatusLabel->setText(tr("正在下载..."));

    m_updateService->downloadUpdate(*m_currentUpdateInfo);
}

void AboutPage::onDownloadProgress(qint64 received, qint64 total)
{
    if (total > 0) {
        int percent = static_cast<int>((received * 100) / total);
        m_downloadProgressBar->setValue(percent);
        m_downloadStatusLabel->setText(tr("正在下载... %1%").arg(percent));
    } else {
        m_downloadStatusLabel->setText(tr("正在下载..."));
    }
}

void AboutPage::onDownloadFinished(const QString &filePath)
{
    m_downloadProgressBar->setValue(100);
    m_downloadStatusLabel->setText(tr("下载完成，准备启动安装程序..."));

    // Launch installer
    QProcess::startDetached(filePath, QStringList());

    // Quit application
    QApplication::quit();
}

void AboutPage::onDownloadFailed(const QString &message)
{
    m_downloadButton->setEnabled(true);
    m_checkUpdateButton->setEnabled(true);
    m_downloadProgressBar->setVisible(false);
    m_downloadStatusLabel->setVisible(true);
    m_downloadStatusLabel->setText(message);
}

void AboutPage::onOpenReleasePageClicked()
{
    if (m_currentUpdateInfo && !m_currentUpdateInfo->htmlUrl.isEmpty()) {
        QDesktopServices::openUrl(QUrl(m_currentUpdateInfo->htmlUrl));
    }
}
