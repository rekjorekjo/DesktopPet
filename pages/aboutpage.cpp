#include "aboutpage.h"

#include "theme/thememanager.h"

#include <QFont>
#include <QFrame>
#include <QVBoxLayout>

AboutPage::AboutPage(QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(nullptr)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_infoCard(nullptr)
    , m_infoCardTitle(nullptr)
    , m_appNameLabel(nullptr)
    , m_techStackLabel(nullptr)
    , m_descriptionLabel(nullptr)
    , m_versionCard(nullptr)
    , m_versionCardTitle(nullptr)
    , m_versionLabel(nullptr)
    , m_statusLabel(nullptr)
    , m_updateCard(nullptr)
    , m_updateCardTitle(nullptr)
    , m_updateNoteLabel(nullptr)
    , m_checkUpdateButton(nullptr)
{
    setupUi();
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

    m_appNameLabel = new QLabel(tr("DesktopPet"), m_infoCard);
    QFont appNameFont = m_appNameLabel->font();
    appNameFont.setPointSize(14);
    appNameFont.setBold(true);
    m_appNameLabel->setFont(appNameFont);
    m_appNameLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.buttonPrimaryColor()));
    infoLayout->addWidget(m_appNameLabel);

    m_descriptionLabel = new QLabel(tr("一个可爱的桌面宠物应用，让你的桌面更加生动有趣。"), m_infoCard);
    m_descriptionLabel->setWordWrap(true);
    m_descriptionLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(theme.textSecondaryColor()));
    infoLayout->addWidget(m_descriptionLabel);

    m_techStackLabel = new QLabel(tr("技术栈：Qt 6 / C++ / CMake"), m_infoCard);
    m_techStackLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                      .arg(theme.textSecondaryColor()));
    infoLayout->addWidget(m_techStackLabel);

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

    m_versionLabel = new QLabel(tr("当前版本：v0.12.0(beta)"), m_versionCard);
    m_versionLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textSecondaryColor()));
    versionLayout->addWidget(m_versionLabel);

    m_statusLabel = new QLabel(tr("开发状态：实验版"), m_versionCard);
    m_statusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(theme.buttonPrimaryColor()));
    versionLayout->addWidget(m_statusLabel);

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

    m_updateNoteLabel = new QLabel(tr("当前阶段更新功能为占位实现，后续版本将支持 GitHub Release 或自定义更新源。"), m_updateCard);
    m_updateNoteLabel->setWordWrap(true);
    m_updateNoteLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(theme.textSecondaryColor()));
    updateLayout->addWidget(m_updateNoteLabel);

    m_checkUpdateButton = new QPushButton(tr("检查更新"), m_updateCard);
    m_checkUpdateButton->setMinimumHeight(36);
    m_checkUpdateButton->setStyleSheet(theme.primaryButtonStyleSheet());
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
    m_techStackLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                      .arg(theme.textSecondaryColor()));

    m_versionCard->setStyleSheet(theme.cardStyleSheet("versionCard"));
    m_versionCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(theme.textPrimaryColor()));
    m_versionLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textSecondaryColor()));
    m_statusLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(theme.buttonPrimaryColor()));

    m_updateCard->setStyleSheet(theme.cardStyleSheet("updateCard"));
    m_updateCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(theme.textPrimaryColor()));
    m_updateNoteLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(theme.textSecondaryColor()));
    m_checkUpdateButton->setStyleSheet(theme.primaryButtonStyleSheet());
}
