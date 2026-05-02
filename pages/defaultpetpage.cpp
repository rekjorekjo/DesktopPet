#include "defaultpetpage.h"

#include "theme/thememanager.h"

#include <QFont>
#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QPushButton>
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
    , m_addToCategoryButton(nullptr)
    , m_configTitleLabel(nullptr)
    , m_categoryTabs(nullptr)
    , m_dailyActionList(nullptr)
    , m_randomActionList(nullptr)
    , m_scheduledActionList(nullptr)
    , m_emotionActionList(nullptr)
    , m_moveUpButton(nullptr)
    , m_moveDownButton(nullptr)
    , m_removeButton(nullptr)
{
    setupUi();
    initData();
    connectSignals();
    refreshActionLibraryList();
    refreshCurrentCategoryList();
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

    m_addToCategoryButton = new QPushButton(tr("添加到当前分类"), leftPanel);
    m_addToCategoryButton->setMinimumHeight(32);
    m_addToCategoryButton->setStyleSheet(theme.primaryButtonStyleSheet());
    leftLayout->addWidget(m_addToCategoryButton);

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

    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(12);

    m_moveUpButton = new QPushButton(tr("上移"), rightPanel);
    m_moveUpButton->setMinimumHeight(32);
    m_moveUpButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    buttonLayout->addWidget(m_moveUpButton);

    m_moveDownButton = new QPushButton(tr("下移"), rightPanel);
    m_moveDownButton->setMinimumHeight(32);
    m_moveDownButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    buttonLayout->addWidget(m_moveDownButton);

    m_removeButton = new QPushButton(tr("移除"), rightPanel);
    m_removeButton->setMinimumHeight(32);
    m_removeButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    buttonLayout->addWidget(m_removeButton);

    buttonLayout->addStretch();
    rightLayout->addLayout(buttonLayout);

    splitter->addWidget(rightPanel);
    splitter->setSizes({250, 500});

    contentLayout->addWidget(splitter, 1);

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);
}

void DefaultPetPage::initData()
{
    m_actionLibrary.append(PetAction("idle_01", tr("默认待机"), "/resources/pets/cat/idle_01", 24, 60));
    m_actionLibrary.append(PetAction("idle_02", tr("眨眼待机"), "/resources/pets/cat/idle_02", 24, 48));
    m_actionLibrary.append(PetAction("wave", tr("挥手"), "/resources/pets/cat/wave", 24, 36));
    m_actionLibrary.append(PetAction("happy", tr("开心"), "/resources/pets/cat/happy", 24, 40));
    m_actionLibrary.append(PetAction("sad", tr("难过"), "/resources/pets/cat/sad", 24, 40));
    m_actionLibrary.append(PetAction("sleepy", tr("困倦"), "/resources/pets/cat/sleepy", 24, 50));
    m_actionLibrary.append(PetAction("thinking", tr("思考"), "/resources/pets/cat/thinking", 24, 45));
    m_actionLibrary.append(PetAction("comfort", tr("安慰"), "/resources/pets/cat/comfort", 24, 42));
    m_actionLibrary.append(PetAction("drink_water", tr("喝水提醒"), "/resources/pets/cat/drink_water", 24, 60));
    m_actionLibrary.append(PetAction("rest_reminder", tr("休息提醒"), "/resources/pets/cat/rest_reminder", 24, 60));

    PetActionRef idle01("idle_01");
    m_playlist.addIdleAction(idle01);
    PetActionRef idle02("idle_02");
    m_playlist.addIdleAction(idle02);

    PetActionRef wave("wave");
    m_playlist.addRandomAction(wave);
    PetActionRef sleepy("sleepy");
    m_playlist.addRandomAction(sleepy);
    PetActionRef thinking("thinking");
    m_playlist.addRandomAction(thinking);

    PetActionRef drinkWater("drink_water");
    drinkWater.intervalSeconds = 1800;
    m_playlist.addTimedAction(drinkWater);
    PetActionRef restReminder("rest_reminder");
    restReminder.intervalSeconds = 3600;
    m_playlist.addTimedAction(restReminder);

    PetActionRef happyRef("happy");
    m_playlist.addEmotionAction("happy", happyRef);
    PetActionRef waveRef("wave");
    m_playlist.addEmotionAction("happy", waveRef);

    PetActionRef comfortRef("comfort");
    m_playlist.addEmotionAction("sad", comfortRef);
    PetActionRef sadRef("sad");
    m_playlist.addEmotionAction("sad", sadRef);
}

void DefaultPetPage::connectSignals()
{
    connect(m_addToCategoryButton, &QPushButton::clicked, this, &DefaultPetPage::onAddToCategory);
    connect(m_moveUpButton, &QPushButton::clicked, this, &DefaultPetPage::onMoveUp);
    connect(m_moveDownButton, &QPushButton::clicked, this, &DefaultPetPage::onMoveDown);
    connect(m_removeButton, &QPushButton::clicked, this, &DefaultPetPage::onRemove);
    connect(m_categoryTabs, &QTabWidget::currentChanged, this, &DefaultPetPage::onTabChanged);
}

void DefaultPetPage::refreshActionLibraryList()
{
    m_actionLibraryList->clear();
    for (const PetAction &action : m_actionLibrary) {
        QString display = QString("%1 - %2").arg(action.id, action.name);
        m_actionLibraryList->addItem(display);
    }
    m_actionCountLabel->setText(QString("%1 个").arg(m_actionLibrary.size()));
}

void DefaultPetPage::refreshCurrentCategoryList()
{
    int tabIndex = m_categoryTabs->currentIndex();
    switch (tabIndex) {
        case 0:
            refreshCategoryList(m_dailyActionList, m_playlist.idleActions());
            break;
        case 1:
            refreshCategoryList(m_randomActionList, m_playlist.randomActions());
            break;
        case 2:
            refreshCategoryList(m_scheduledActionList, m_playlist.timedActions());
            break;
        case 3:
            refreshCategoryList(m_emotionActionList, m_playlist.emotionActions("happy"));
            break;
    }
}

void DefaultPetPage::refreshCategoryList(QListWidget *list, const QList<PetActionRef> &actions)
{
    list->clear();
    for (const PetActionRef &ref : actions) {
        list->addItem(formatActionDisplay(ref));
    }
}

QString DefaultPetPage::formatActionDisplay(const PetActionRef &ref) const
{
    QString name = getActionName(ref.actionId);
    int tabIndex = m_categoryTabs->currentIndex();

    if (tabIndex == 2) {
        int minutes = ref.intervalSeconds / 60;
        return QString("%1 (%2) - 每 %3 分钟").arg(name, ref.actionId).arg(minutes);
    } else if (tabIndex == 3) {
        return QString("%1 (%2) - %3").arg(name, ref.actionId, ref.emotion);
    } else {
        return QString("%1 (%2) - 播放 %3 次").arg(name, ref.actionId).arg(ref.repeat);
    }
}

QString DefaultPetPage::getActionName(const QString &actionId) const
{
    for (const PetAction &action : m_actionLibrary) {
        if (action.id == actionId) {
            return action.name;
        }
    }
    return actionId;
}

QListWidget* DefaultPetPage::currentCategoryList() const
{
    int tabIndex = m_categoryTabs->currentIndex();
    switch (tabIndex) {
        case 0: return m_dailyActionList;
        case 1: return m_randomActionList;
        case 2: return m_scheduledActionList;
        case 3: return m_emotionActionList;
        default: return nullptr;
    }
}

QList<PetActionRef> DefaultPetPage::currentCategoryActions() const
{
    int tabIndex = m_categoryTabs->currentIndex();
    switch (tabIndex) {
        case 0: return m_playlist.idleActions();
        case 1: return m_playlist.randomActions();
        case 2: return m_playlist.timedActions();
        case 3: return m_playlist.emotionActions("happy");
        default: return QList<PetActionRef>();
    }
}

void DefaultPetPage::onAddToCategory()
{
    int row = m_actionLibraryList->currentRow();
    if (row < 0 || row >= m_actionLibrary.size()) {
        return;
    }

    const PetAction &action = m_actionLibrary[row];
    PetActionRef ref(action.id);

    int tabIndex = m_categoryTabs->currentIndex();
    bool success = false;

    switch (tabIndex) {
        case 0:
            success = m_playlist.addIdleAction(ref);
            break;
        case 1:
            success = m_playlist.addRandomAction(ref);
            break;
        case 2:
            ref.intervalSeconds = 1800;
            success = m_playlist.addTimedAction(ref);
            break;
        case 3:
            ref.emotion = "happy";
            success = m_playlist.addEmotionAction("happy", ref);
            break;
    }

    if (success) {
        refreshCurrentCategoryList();
    }
}

void DefaultPetPage::onMoveUp()
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row <= 0) return;

    int tabIndex = m_categoryTabs->currentIndex();
    bool success = false;

    switch (tabIndex) {
        case 0: success = m_playlist.moveIdleActionUp(row); break;
        case 1: success = m_playlist.moveRandomActionUp(row); break;
        case 2: success = m_playlist.moveTimedActionUp(row); break;
        case 3: success = m_playlist.moveEmotionActionUp("happy", row); break;
    }

    if (success) {
        refreshCurrentCategoryList();
        list->setCurrentRow(row - 1);
    }
}

void DefaultPetPage::onMoveDown()
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    int tabIndex = m_categoryTabs->currentIndex();
    bool success = false;

    switch (tabIndex) {
        case 0: success = m_playlist.moveIdleActionDown(row); break;
        case 1: success = m_playlist.moveRandomActionDown(row); break;
        case 2: success = m_playlist.moveTimedActionDown(row); break;
        case 3: success = m_playlist.moveEmotionActionDown("happy", row); break;
    }

    if (success) {
        refreshCurrentCategoryList();
        list->setCurrentRow(row + 1);
    }
}

void DefaultPetPage::onRemove()
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    int tabIndex = m_categoryTabs->currentIndex();

    switch (tabIndex) {
        case 0: m_playlist.removeIdleActionAt(row); break;
        case 1: m_playlist.removeRandomActionAt(row); break;
        case 2: m_playlist.removeTimedActionAt(row); break;
        case 3: m_playlist.removeEmotionActionAt("happy", row); break;
    }

    refreshCurrentCategoryList();
}

void DefaultPetPage::onTabChanged(int)
{
    refreshCurrentCategoryList();
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
    m_addToCategoryButton->setStyleSheet(theme.primaryButtonStyleSheet());

    m_configTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(theme.textPrimaryColor()));
    m_categoryTabs->setStyleSheet(theme.tabWidgetStyleSheet());
    m_dailyActionList->setStyleSheet(theme.listWidgetStyleSheet());
    m_randomActionList->setStyleSheet(theme.listWidgetStyleSheet());
    m_scheduledActionList->setStyleSheet(theme.listWidgetStyleSheet());
    m_emotionActionList->setStyleSheet(theme.listWidgetStyleSheet());

    m_moveUpButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_moveDownButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_removeButton->setStyleSheet(theme.secondaryButtonStyleSheet());
}
