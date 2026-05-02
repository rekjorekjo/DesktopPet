#include "defaultpetpage.h"

#include "theme/thememanager.h"

#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QVBoxLayout>

DefaultPetPage::DefaultPetPage(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_titleLabel(nullptr)
    , m_infoCard(nullptr)
    , m_petNameLabel(nullptr)
    , m_canvasSizeLabel(nullptr)
    , m_actionCountLabel(nullptr)
    , m_petDirLabel(nullptr)
    , m_libraryTitleLabel(nullptr)
    , m_actionLibraryList(nullptr)
    , m_configTitleLabel(nullptr)
    , m_categoryTabs(nullptr)
    , m_dailyActionList(nullptr)
    , m_randomActionList(nullptr)
    , m_scheduledActionList(nullptr)
    , m_emotionActionList(nullptr)
{
    setupUi();
    loadMockData();
}

DefaultPetPage::~DefaultPetPage() {}

void DefaultPetPage::setupUi()
{
    ThemeManager &theme = ThemeManager::instance();

    QVBoxLayout *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(0, 0, 0, 0);
    outerLayout->setSpacing(0);

    m_scrollArea = new QScrollArea(this);
    m_scrollArea->setWidgetResizable(true);
    m_scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scrollArea->setStyleSheet(theme.scrollAreaStyleSheet());

    m_contentWidget = new QWidget();
    m_contentWidget->setStyleSheet(theme.pageStyleSheet());

    QVBoxLayout *contentLayout = new QVBoxLayout(m_contentWidget);
    contentLayout->setContentsMargins(28, 20, 28, 28);
    contentLayout->setSpacing(20);

    m_titleLabel = new QLabel(tr("默认宠物"), m_contentWidget);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(theme.titleLabelStyleSheet());
    m_titleLabel->setMargin(0);
    contentLayout->addWidget(m_titleLabel);

    m_infoCard = new QFrame(m_contentWidget);
    m_infoCard->setObjectName("infoCard");
    m_infoCard->setStyleSheet(theme.cardStyleSheet("infoCard"));
    QGridLayout *infoLayout = new QGridLayout(m_infoCard);
    infoLayout->setContentsMargins(24, 20, 24, 20);
    infoLayout->setSpacing(16);

    QLabel *nameTitleLabel = new QLabel(tr("宠物名称"), m_infoCard);
    nameTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                     .arg(theme.textSecondaryColor()));
    m_petNameLabel = new QLabel(tr("小白猫"), m_infoCard);
    m_petNameLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textPrimaryColor()));
    QFont nameFont = m_petNameLabel->font();
    nameFont.setBold(true);
    m_petNameLabel->setFont(nameFont);

    QLabel *sizeTitleLabel = new QLabel(tr("画布尺寸"), m_infoCard);
    sizeTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                      .arg(theme.textSecondaryColor()));
    m_canvasSizeLabel = new QLabel(tr("400 x 400"), m_infoCard);
    m_canvasSizeLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(theme.textPrimaryColor()));

    QLabel *countTitleLabel = new QLabel(tr("动作数量"), m_infoCard);
    countTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(theme.textSecondaryColor()));
    m_actionCountLabel = new QLabel(tr("10 个"), m_infoCard);
    m_actionCountLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(theme.textPrimaryColor()));

    QLabel *dirTitleLabel = new QLabel(tr("宠物目录"), m_infoCard);
    dirTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                     .arg(theme.textSecondaryColor()));
    m_petDirLabel = new QLabel(tr("/resources/pets/cat"), m_infoCard);
    m_petDirLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(theme.textPrimaryColor()));

    infoLayout->addWidget(nameTitleLabel, 0, 0);
    infoLayout->addWidget(m_petNameLabel, 0, 1);
    infoLayout->addWidget(sizeTitleLabel, 0, 2);
    infoLayout->addWidget(m_canvasSizeLabel, 0, 3);
    infoLayout->addWidget(countTitleLabel, 1, 0);
    infoLayout->addWidget(m_actionCountLabel, 1, 1);
    infoLayout->addWidget(dirTitleLabel, 1, 2);
    infoLayout->addWidget(m_petDirLabel, 1, 3);
    infoLayout->setColumnStretch(1, 1);
    infoLayout->setColumnStretch(3, 2);

    contentLayout->addWidget(m_infoCard);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, m_contentWidget);
    splitter->setStyleSheet(theme.splitterStyleSheet());
    splitter->setHandleWidth(1);

    QWidget *leftPanel = new QWidget(splitter);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(12);

    m_libraryTitleLabel = new QLabel(tr("动作库"), leftPanel);
    QFont libTitleFont = m_libraryTitleLabel->font();
    libTitleFont.setPointSize(12);
    libTitleFont.setBold(true);
    m_libraryTitleLabel->setFont(libTitleFont);
    m_libraryTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(theme.textPrimaryColor()));
    leftLayout->addWidget(m_libraryTitleLabel);

    m_actionLibraryList = new QListWidget(leftPanel);
    m_actionLibraryList->setStyleSheet(theme.listWidgetStyleSheet());
    leftLayout->addWidget(m_actionLibraryList, 1);

    splitter->addWidget(leftPanel);

    QWidget *rightPanel = new QWidget(splitter);
    QVBoxLayout *rightLayout = new QVBoxLayout(rightPanel);
    rightLayout->setContentsMargins(0, 0, 0, 0);
    rightLayout->setSpacing(12);

    m_configTitleLabel = new QLabel(tr("动作分类配置"), rightPanel);
    QFont configTitleFont = m_configTitleLabel->font();
    configTitleFont.setPointSize(12);
    configTitleFont.setBold(true);
    m_configTitleLabel->setFont(configTitleFont);
    m_configTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(theme.textPrimaryColor()));
    rightLayout->addWidget(m_configTitleLabel);

    m_categoryTabs = new QTabWidget(rightPanel);
    m_categoryTabs->setStyleSheet(theme.tabWidgetStyleSheet());

    m_dailyActionList = new QListWidget(m_categoryTabs);
    m_dailyActionList->setStyleSheet(theme.listWidgetStyleSheet());
    m_categoryTabs->addTab(m_dailyActionList, tr("日常动作"));

    m_randomActionList = new QListWidget(m_categoryTabs);
    m_randomActionList->setStyleSheet(theme.listWidgetStyleSheet());
    m_categoryTabs->addTab(m_randomActionList, tr("随机动作"));

    m_scheduledActionList = new QListWidget(m_categoryTabs);
    m_scheduledActionList->setStyleSheet(theme.listWidgetStyleSheet());
    m_categoryTabs->addTab(m_scheduledActionList, tr("定时动作"));

    m_emotionActionList = new QListWidget(m_categoryTabs);
    m_emotionActionList->setStyleSheet(theme.listWidgetStyleSheet());
    m_categoryTabs->addTab(m_emotionActionList, tr("情绪动作"));

    rightLayout->addWidget(m_categoryTabs, 1);

    splitter->addWidget(rightPanel);
    splitter->setSizes({250, 500});

    contentLayout->addWidget(splitter, 1);

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);
}

void DefaultPetPage::loadMockData()
{
    QStringList actionLibrary = {
        tr("idle_01 - 默认待机"),
        tr("idle_02 - 眨眼待机"),
        tr("wave - 挥手"),
        tr("happy - 开心"),
        tr("sad - 难过"),
        tr("sleepy - 困倦"),
        tr("thinking - 思考"),
        tr("comfort - 安慰"),
        tr("drink_water - 喝水提醒"),
        tr("rest_reminder - 休息提醒")
    };
    m_actionLibraryList->addItems(actionLibrary);

    QStringList dailyActions = {
        tr("idle_01 - 默认待机"),
        tr("idle_02 - 眨眼待机"),
        tr("wave - 挥手")
    };
    m_dailyActionList->addItems(dailyActions);

    QStringList randomActions = {
        tr("happy - 开心"),
        tr("thinking - 思考"),
        tr("comfort - 安慰")
    };
    m_randomActionList->addItems(randomActions);

    QStringList scheduledActions = {
        tr("drink_water - 喝水提醒 / 每 30 分钟"),
        tr("rest_reminder - 休息提醒 / 每 60 分钟")
    };
    m_scheduledActionList->addItems(scheduledActions);

    QStringList emotionActions = {
        tr("happy - 开心"),
        tr("sad - 难过"),
        tr("sleepy - 困倦")
    };
    m_emotionActionList->addItems(emotionActions);
}

void DefaultPetPage::refreshTheme()
{
    applyTheme();
}

void DefaultPetPage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();

    m_scrollArea->setStyleSheet(theme.scrollAreaStyleSheet());
    m_contentWidget->setStyleSheet(theme.pageStyleSheet());

    m_titleLabel->setStyleSheet(theme.titleLabelStyleSheet());

    m_infoCard->setStyleSheet(theme.cardStyleSheet("infoCard"));

    QGridLayout *infoLayout = qobject_cast<QGridLayout *>(m_infoCard->layout());
    if (infoLayout) {
        for (int i = 0; i < infoLayout->rowCount(); ++i) {
            for (int j = 0; j < infoLayout->columnCount(); ++j) {
                QLayoutItem *item = infoLayout->itemAtPosition(i, j);
                if (item && item->widget()) {
                    QLabel *label = qobject_cast<QLabel *>(item->widget());
                    if (label) {
                        if (label == m_petNameLabel || label == m_canvasSizeLabel ||
                            label == m_actionCountLabel || label == m_petDirLabel) {
                            label->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                                   .arg(theme.textPrimaryColor()));
                        } else {
                            label->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                                   .arg(theme.textSecondaryColor()));
                        }
                    }
                }
            }
        }
    }

    m_libraryTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(theme.textPrimaryColor()));
    m_actionLibraryList->setStyleSheet(theme.listWidgetStyleSheet());

    m_configTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(theme.textPrimaryColor()));
    m_categoryTabs->setStyleSheet(theme.tabWidgetStyleSheet());
    m_dailyActionList->setStyleSheet(theme.listWidgetStyleSheet());
    m_randomActionList->setStyleSheet(theme.listWidgetStyleSheet());
    m_scheduledActionList->setStyleSheet(theme.listWidgetStyleSheet());
    m_emotionActionList->setStyleSheet(theme.listWidgetStyleSheet());
}
