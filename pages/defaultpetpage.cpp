#include "defaultpetpage.h"

#include "theme/thememanager.h"

#include <QFont>
#include <QFrame>
#include <QVBoxLayout>

DefaultPetPage::DefaultPetPage(QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(nullptr)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_petCard(nullptr)
    , m_petCardTitle(nullptr)
    , m_petResourceList(nullptr)
{
    setupUi();
}

DefaultPetPage::~DefaultPetPage() {}

void DefaultPetPage::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_titleLabel = new QLabel(tr("默认宠物"), this);
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

    m_petCard = new QFrame(m_contentWidget);
    m_petCard->setObjectName("petCard");
    m_petCard->setStyleSheet(theme.cardStyleSheet("petCard"));
    QVBoxLayout *petLayout = new QVBoxLayout(m_petCard);
    petLayout->setContentsMargins(24, 24, 24, 24);
    petLayout->setSpacing(16);

    m_petCardTitle = new QLabel(tr("宠物资源列表"), m_petCard);
    QFont petTitleFont = m_petCardTitle->font();
    petTitleFont.setPointSize(12);
    petTitleFont.setBold(true);
    m_petCardTitle->setFont(petTitleFont);
    m_petCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textPrimaryColor()));
    petLayout->addWidget(m_petCardTitle);

    m_petResourceList = new QListWidget(m_petCard);
    m_petResourceList->setMinimumHeight(300);
    m_petResourceList->setStyleSheet(theme.listWidgetStyleSheet());
    petLayout->addWidget(m_petResourceList, 1);

    contentLayout->addWidget(m_petCard, 1);

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);
}

void DefaultPetPage::refreshTheme()
{
    applyTheme();
}

void DefaultPetPage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();

    m_titleLabel->setStyleSheet(theme.titleLabelStyleSheet());
    m_scrollArea->setStyleSheet(theme.scrollAreaStyleSheet());
    m_contentWidget->setStyleSheet(theme.pageStyleSheet());

    m_petCard->setStyleSheet(theme.cardStyleSheet("petCard"));
    m_petCardTitle->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textPrimaryColor()));
    m_petResourceList->setStyleSheet(theme.listWidgetStyleSheet());
}
