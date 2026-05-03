#include "actionsettingspage.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "theme/thememanager.h"

#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QSplitter>
#include <QVBoxLayout>

ActionSettingsPage::ActionSettingsPage(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_titleLabel(nullptr)
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
    , m_saveConfigButton(nullptr)
    , m_actionConfigPanel(nullptr)
    , m_actionConfigTitleLabel(nullptr)
    , m_loopCheckBox(nullptr)
    , m_repeatLabel(nullptr)
    , m_repeatSpinBox(nullptr)
    , m_repeatHintLabel(nullptr)
    , m_moveEnabledCheckBox(nullptr)
    , m_speedLabel(nullptr)
    , m_speedComboBox(nullptr)
    , m_animationSpeedLabel(nullptr)
    , m_animationSpeedComboBox(nullptr)
    , m_loadedSuccessfully(false)
{
    setupUi();
    initData();
    connectSignals();

    if (m_loadedSuccessfully) {
        refreshActionLibraryList();
        refreshCurrentCategoryList();
    }
    setActionConfigPanelEnabled(false);
}

ActionSettingsPage::~ActionSettingsPage() {}

void ActionSettingsPage::setupUi()
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

    m_titleLabel = new QLabel(tr("动作设置"), m_contentWidget);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(theme.titleLabelStyleSheet());
    m_titleLabel->setMargin(0);
    contentLayout->addWidget(m_titleLabel);

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

    buttonLayout->addSpacing(20);

    m_saveConfigButton = new QPushButton(tr("保存配置"), rightPanel);
    m_saveConfigButton->setMinimumHeight(32);
    m_saveConfigButton->setStyleSheet(theme.primaryButtonStyleSheet());
    buttonLayout->addWidget(m_saveConfigButton);

    buttonLayout->addStretch();
    rightLayout->addLayout(buttonLayout);

    m_actionConfigPanel = new QFrame(rightPanel);
    m_actionConfigPanel->setObjectName("actionConfigPanel");
    m_actionConfigPanel->setStyleSheet(theme.cardStyleSheet("actionConfigPanel"));
    QHBoxLayout *configPanelLayout = new QHBoxLayout(m_actionConfigPanel);
    configPanelLayout->setContentsMargins(16, 12, 16, 12);
    configPanelLayout->setSpacing(16);

    m_actionConfigTitleLabel = new QLabel(tr("当前动作配置"), m_actionConfigPanel);
    m_actionConfigTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                              .arg(theme.textPrimaryColor()));
    QFont configPanelTitleFont = m_actionConfigTitleLabel->font();
    configPanelTitleFont.setBold(true);
    m_actionConfigTitleLabel->setFont(configPanelTitleFont);
    configPanelLayout->addWidget(m_actionConfigTitleLabel);

    m_loopCheckBox = new QCheckBox(tr("循环播放"), m_actionConfigPanel);
    m_loopCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    configPanelLayout->addWidget(m_loopCheckBox);

    m_repeatLabel = new QLabel(tr("循环次数"), m_actionConfigPanel);
    m_repeatLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(theme.textSecondaryColor()));
    configPanelLayout->addWidget(m_repeatLabel);

    m_repeatSpinBox = new QSpinBox(m_actionConfigPanel);
    m_repeatSpinBox->setRange(0, 10);
    m_repeatSpinBox->setValue(1);
    m_repeatSpinBox->setMinimumWidth(80);
    m_repeatSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    configPanelLayout->addWidget(m_repeatSpinBox);

    m_repeatHintLabel = new QLabel(tr("0 表示无限循环"), m_actionConfigPanel);
    m_repeatHintLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(theme.textSecondaryColor()));
    configPanelLayout->addWidget(m_repeatHintLabel);

    configPanelLayout->addSpacing(20);

    m_animationSpeedLabel = new QLabel(tr("播放速度"), m_actionConfigPanel);
    m_animationSpeedLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                             .arg(theme.textSecondaryColor()));
    configPanelLayout->addWidget(m_animationSpeedLabel);

    m_animationSpeedComboBox = new QComboBox(m_actionConfigPanel);
    m_animationSpeedComboBox->addItem("0.1x", 0.1);
    m_animationSpeedComboBox->addItem("0.2x", 0.2);
    m_animationSpeedComboBox->addItem("0.5x", 0.5);
    m_animationSpeedComboBox->addItem("0.8x", 0.8);
    m_animationSpeedComboBox->addItem("1.0x", 1.0);
    m_animationSpeedComboBox->addItem("1.2x", 1.2);
    m_animationSpeedComboBox->addItem("1.5x", 1.5);
    m_animationSpeedComboBox->addItem("2.0x", 2.0);
    m_animationSpeedComboBox->addItem("2.5x", 2.5);
    m_animationSpeedComboBox->addItem("3.0x", 3.0);
    m_animationSpeedComboBox->setCurrentIndex(4);
    m_animationSpeedComboBox->setMinimumWidth(80);
    m_animationSpeedComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    configPanelLayout->addWidget(m_animationSpeedComboBox);

    configPanelLayout->addSpacing(20);

    m_moveEnabledCheckBox = new QCheckBox(tr("播放时移动"), m_actionConfigPanel);
    m_moveEnabledCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    configPanelLayout->addWidget(m_moveEnabledCheckBox);

    m_speedLabel = new QLabel(tr("移动速度"), m_actionConfigPanel);
    m_speedLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textSecondaryColor()));
    configPanelLayout->addWidget(m_speedLabel);

    m_speedComboBox = new QComboBox(m_actionConfigPanel);
    m_speedComboBox->addItem("0.1x", 0.1);
    m_speedComboBox->addItem("0.2x", 0.2);
    m_speedComboBox->addItem("0.5x", 0.5);
    m_speedComboBox->addItem("0.8x", 0.8);
    m_speedComboBox->addItem("1.0x", 1.0);
    m_speedComboBox->addItem("1.2x", 1.2);
    m_speedComboBox->addItem("1.5x", 1.5);
    m_speedComboBox->addItem("2.0x", 2.0);
    m_speedComboBox->addItem("2.5x", 2.5);
    m_speedComboBox->addItem("3.0x", 3.0);
    m_speedComboBox->setCurrentIndex(4);
    m_speedComboBox->setMinimumWidth(80);
    m_speedComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    configPanelLayout->addWidget(m_speedComboBox);

    configPanelLayout->addStretch();
    rightLayout->addWidget(m_actionConfigPanel);

    splitter->addWidget(rightPanel);
    splitter->setSizes({250, 500});

    contentLayout->addWidget(splitter, 1);

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);
}

void ActionSettingsPage::initData()
{
    PetBasicInfo petInfo;
    if (PetConfigManager::loadPetFromDirectory(PetPaths::defaultPetDirectory(), petInfo, m_actionLibrary, m_playlist)) {
        m_loadedSuccessfully = true;
    } else {
        m_loadedSuccessfully = false;
        m_actionLibraryList->clear();
        m_actionLibraryList->addItem(tr("宠物配置加载失败"));
        m_actionLibraryList->setEnabled(false);

        m_addToCategoryButton->setEnabled(false);
        m_moveUpButton->setEnabled(false);
        m_moveDownButton->setEnabled(false);
        m_removeButton->setEnabled(false);
        m_saveConfigButton->setEnabled(false);
        m_categoryTabs->setEnabled(false);
        m_actionConfigPanel->setEnabled(false);
    }
}

void ActionSettingsPage::connectSignals()
{
    connect(m_addToCategoryButton, &QPushButton::clicked, this, &ActionSettingsPage::onAddToCategory);
    connect(m_moveUpButton, &QPushButton::clicked, this, &ActionSettingsPage::onMoveUp);
    connect(m_moveDownButton, &QPushButton::clicked, this, &ActionSettingsPage::onMoveDown);
    connect(m_removeButton, &QPushButton::clicked, this, &ActionSettingsPage::onRemove);
    connect(m_saveConfigButton, &QPushButton::clicked, this, &ActionSettingsPage::onSaveConfig);
    connect(m_categoryTabs, &QTabWidget::currentChanged, this, &ActionSettingsPage::onTabChanged);

    connect(m_dailyActionList, &QListWidget::itemSelectionChanged, this, &ActionSettingsPage::onCategorySelectionChanged);
    connect(m_randomActionList, &QListWidget::itemSelectionChanged, this, &ActionSettingsPage::onCategorySelectionChanged);
    connect(m_scheduledActionList, &QListWidget::itemSelectionChanged, this, &ActionSettingsPage::onCategorySelectionChanged);
    connect(m_emotionActionList, &QListWidget::itemSelectionChanged, this, &ActionSettingsPage::onCategorySelectionChanged);

    connect(m_loopCheckBox, &QCheckBox::stateChanged, this, &ActionSettingsPage::onLoopChanged);
    connect(m_repeatSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ActionSettingsPage::onRepeatChanged);
    connect(m_moveEnabledCheckBox, &QCheckBox::stateChanged, this, &ActionSettingsPage::onMoveEnabledChanged);
    connect(m_speedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ActionSettingsPage::onSpeedChanged);
    connect(m_animationSpeedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ActionSettingsPage::onAnimationSpeedChanged);
}

void ActionSettingsPage::refreshActionLibraryList()
{
    m_actionLibraryList->clear();
    for (const PetAction &action : m_actionLibrary) {
        QString display = QString("%1 - %2").arg(action.id, action.name);
        m_actionLibraryList->addItem(display);
    }
}

void ActionSettingsPage::refreshCurrentCategoryList()
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

void ActionSettingsPage::refreshCategoryList(QListWidget *list, const QList<PetActionRef> &actions)
{
    int savedRow = list->currentRow();
    list->clear();
    for (const PetActionRef &ref : actions) {
        list->addItem(formatActionDisplay(ref));
    }
    if (savedRow >= 0 && savedRow < list->count()) {
        list->setCurrentRow(savedRow);
    }
}

QString ActionSettingsPage::formatActionDisplay(const PetActionRef &ref) const
{
    QString name = getActionName(ref.actionId);
    int tabIndex = m_categoryTabs->currentIndex();

    QString repeatText;
    if (ref.repeat == 0 || ref.repeat > 10) {
        repeatText = tr("无限循环");
    } else {
        repeatText = tr("播放 %1 次").arg(ref.repeat);
    }

    if (tabIndex == 2) {
        int minutes = ref.intervalSeconds / 60;
        return QString("%1 (%2) - 每 %3 分钟 / %4").arg(name, ref.actionId).arg(minutes).arg(repeatText);
    } else if (tabIndex == 3) {
        return QString("%1 (%2) - %3 / %4").arg(name, ref.actionId, ref.emotion, repeatText);
    } else {
        return QString("%1 (%2) - %3").arg(name, ref.actionId, repeatText);
    }
}

QString ActionSettingsPage::getActionName(const QString &actionId) const
{
    for (const PetAction &action : m_actionLibrary) {
        if (action.id == actionId) {
            return action.name;
        }
    }
    return actionId;
}

QListWidget* ActionSettingsPage::currentCategoryList() const
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

QList<PetActionRef> ActionSettingsPage::currentCategoryActions() const
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

PetActionRef ActionSettingsPage::currentSelectedRef() const
{
    QListWidget *list = currentCategoryList();
    if (!list) return PetActionRef();

    int row = list->currentRow();
    if (row < 0) return PetActionRef();

    QList<PetActionRef> actions = currentCategoryActions();
    if (row >= actions.size()) return PetActionRef();

    return actions[row];
}

bool ActionSettingsPage::updateCurrentSelectedRef(const PetActionRef &ref)
{
    QListWidget *list = currentCategoryList();
    if (!list) return false;

    int row = list->currentRow();
    if (row < 0) return false;

    int tabIndex = m_categoryTabs->currentIndex();
    switch (tabIndex) {
        case 0: return m_playlist.updateIdleActionAt(row, ref);
        case 1: return m_playlist.updateRandomActionAt(row, ref);
        case 2: return m_playlist.updateTimedActionAt(row, ref);
        case 3: return m_playlist.updateEmotionActionAt("happy", row, ref);
        default: return false;
    }
}

void ActionSettingsPage::updateActionConfigPanel()
{
    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) {
        setActionConfigPanelEnabled(false);
        return;
    }

    setActionConfigPanelEnabled(true);

    QSignalBlocker loopBlocker(m_loopCheckBox);
    QSignalBlocker repeatBlocker(m_repeatSpinBox);
    QSignalBlocker moveBlocker(m_moveEnabledCheckBox);
    QSignalBlocker speedBlocker(m_speedComboBox);
    QSignalBlocker animSpeedBlocker(m_animationSpeedComboBox);

    m_loopCheckBox->setChecked(ref.loop);
    m_repeatSpinBox->setValue(ref.repeat);
    m_moveEnabledCheckBox->setChecked(ref.moveEnabled);

    int speedIndex = findSpeedIndex(ref.movementSpeed);
    m_speedComboBox->setCurrentIndex(speedIndex);

    int animSpeedIndex = findSpeedIndex(ref.animationSpeed);
    m_animationSpeedComboBox->setCurrentIndex(animSpeedIndex);
}

void ActionSettingsPage::setActionConfigPanelEnabled(bool enabled)
{
    m_actionConfigPanel->setEnabled(enabled);
    m_loopCheckBox->setEnabled(enabled);
    m_repeatSpinBox->setEnabled(enabled);
    m_moveEnabledCheckBox->setEnabled(enabled);
    m_speedComboBox->setEnabled(enabled);
    m_animationSpeedComboBox->setEnabled(enabled);

    if (!enabled) {
        QSignalBlocker loopBlocker(m_loopCheckBox);
        QSignalBlocker repeatBlocker(m_repeatSpinBox);
        QSignalBlocker moveBlocker(m_moveEnabledCheckBox);
        QSignalBlocker speedBlocker(m_speedComboBox);
        QSignalBlocker animSpeedBlocker(m_animationSpeedComboBox);
        m_loopCheckBox->setChecked(false);
        m_repeatSpinBox->setValue(1);
        m_moveEnabledCheckBox->setChecked(false);
        m_speedComboBox->setCurrentIndex(4);
        m_animationSpeedComboBox->setCurrentIndex(4);
    }
}

void ActionSettingsPage::clearCategorySelection()
{
    QListWidget *list = currentCategoryList();
    if (list) {
        list->clearSelection();
    }
    setActionConfigPanelEnabled(false);
}

void ActionSettingsPage::onAddToCategory()
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

void ActionSettingsPage::onMoveUp()
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row <= 0) return;

    int tabIndex = m_categoryTabs->currentIndex();
    bool success = false;

    switch (tabIndex) {
        case 0:
            success = m_playlist.moveIdleActionUp(row);
            break;
        case 1:
            success = m_playlist.moveRandomActionUp(row);
            break;
        case 2:
            success = m_playlist.moveTimedActionUp(row);
            break;
        case 3:
            success = m_playlist.moveEmotionActionUp("happy", row);
            break;
    }

    if (success) {
        refreshCurrentCategoryList();
        list->setCurrentRow(row - 1);
    }
}

void ActionSettingsPage::onMoveDown()
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0 || row >= list->count() - 1) return;

    int tabIndex = m_categoryTabs->currentIndex();
    bool success = false;

    switch (tabIndex) {
        case 0:
            success = m_playlist.moveIdleActionDown(row);
            break;
        case 1:
            success = m_playlist.moveRandomActionDown(row);
            break;
        case 2:
            success = m_playlist.moveTimedActionDown(row);
            break;
        case 3:
            success = m_playlist.moveEmotionActionDown("happy", row);
            break;
    }

    if (success) {
        refreshCurrentCategoryList();
        list->setCurrentRow(row + 1);
    }
}

void ActionSettingsPage::onRemove()
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    int tabIndex = m_categoryTabs->currentIndex();
    bool success = false;

    switch (tabIndex) {
        case 0:
            success = m_playlist.removeIdleActionAt(row);
            break;
        case 1:
            success = m_playlist.removeRandomActionAt(row);
            break;
        case 2:
            success = m_playlist.removeTimedActionAt(row);
            break;
        case 3:
            success = m_playlist.removeEmotionActionAt("happy", row);
            break;
    }

    if (success) {
        refreshCurrentCategoryList();
        clearCategorySelection();
    }
}

void ActionSettingsPage::onTabChanged(int)
{
    refreshCurrentCategoryList();
    clearCategorySelection();
}

void ActionSettingsPage::onCategorySelectionChanged()
{
    updateActionConfigPanel();
}

void ActionSettingsPage::onLoopChanged(int state)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    bool loopChecked = (state == Qt::Checked);
    ref.loop = loopChecked;

    if (loopChecked && ref.repeat == 1) {
        ref.repeat = 0;
        QSignalBlocker blocker(m_repeatSpinBox);
        m_repeatSpinBox->setValue(0);
    } else if (!loopChecked && ref.repeat == 0) {
        ref.repeat = 1;
        QSignalBlocker blocker(m_repeatSpinBox);
        m_repeatSpinBox->setValue(1);
    }

    updateCurrentSelectedRef(ref);
    refreshCurrentCategoryList();
    list->setCurrentRow(row);
}

void ActionSettingsPage::onRepeatChanged(int value)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    ref.repeat = value;

    if (value == 0) {
        ref.loop = true;
        QSignalBlocker blocker(m_loopCheckBox);
        m_loopCheckBox->setChecked(true);
    }

    updateCurrentSelectedRef(ref);
    refreshCurrentCategoryList();
    list->setCurrentRow(row);
}

int ActionSettingsPage::findSpeedIndex(double speed) const
{
    QList<double> speeds = {0.1, 0.2, 0.5, 0.8, 1.0, 1.2, 1.5, 2.0, 2.5, 3.0};

    int closestIndex = 4;
    double minDiff = qAbs(speeds[4] - speed);

    for (int i = 0; i < speeds.size(); ++i) {
        double diff = qAbs(speeds[i] - speed);
        if (diff < minDiff) {
            minDiff = diff;
            closestIndex = i;
        }
    }

    return closestIndex;
}

void ActionSettingsPage::onMoveEnabledChanged(int state)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    ref.moveEnabled = (state == Qt::Checked);
    updateCurrentSelectedRef(ref);
}

void ActionSettingsPage::onSpeedChanged(int index)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    double speed = m_speedComboBox->itemData(index).toDouble();
    ref.movementSpeed = speed;
    updateCurrentSelectedRef(ref);
}

void ActionSettingsPage::onAnimationSpeedChanged(int index)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    double speed = m_animationSpeedComboBox->itemData(index).toDouble();
    ref.animationSpeed = speed;
    updateCurrentSelectedRef(ref);
}

void ActionSettingsPage::onSaveConfig()
{
    if (!m_loadedSuccessfully) {
        QMessageBox::warning(this, tr("保存失败"), tr("宠物配置未正确加载，无法保存。"));
        return;
    }

    QString petDir = PetPaths::defaultPetDirectory();
    QString playlistPath = QDir(petDir).filePath("playlist.json");

    if (PetConfigManager::savePlaylistToJson(playlistPath, m_playlist)) {
        QMessageBox::information(this, tr("保存成功"), tr("配置已保存"));
    } else {
        QMessageBox::warning(this, tr("保存失败"), tr("配置保存失败，请检查文件权限。"));
    }
}

void ActionSettingsPage::refreshTheme()
{
    applyTheme();
}

void ActionSettingsPage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();

    m_scrollArea->setStyleSheet(theme.scrollAreaStyleSheet());
    m_contentWidget->setStyleSheet(theme.pageStyleSheet());

    m_titleLabel->setStyleSheet(theme.titleLabelStyleSheet());

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
    m_saveConfigButton->setStyleSheet(theme.primaryButtonStyleSheet());

    m_actionConfigPanel->setStyleSheet(theme.cardStyleSheet("actionConfigPanel"));
    m_actionConfigTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                              .arg(theme.textPrimaryColor()));
    m_loopCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_repeatLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(theme.textSecondaryColor()));
    m_repeatSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    m_repeatHintLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                       .arg(theme.textSecondaryColor()));
    m_moveEnabledCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_speedLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textSecondaryColor()));
    m_speedComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    m_animationSpeedLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                             .arg(theme.textSecondaryColor()));
    m_animationSpeedComboBox->setStyleSheet(theme.comboBoxStyleSheet());
}
