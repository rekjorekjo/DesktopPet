#include "actionsettingspage.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "dialogs/addactiondialog.h"
#include "dialogs/importgifdialog.h"
#include "services/actionimportservice.h"
#include "theme/thememanager.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QSignalBlocker>
#include <QSizePolicy>
#include <QSpinBox>
#include <QSplitter>
#include <QtGlobal>
#include <QVBoxLayout>

ActionSettingsPage::ActionSettingsPage(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_titleLabel(nullptr)
    , m_libraryTitleLabel(nullptr)
    , m_actionLibraryList(nullptr)
    , m_addActionButton(nullptr)
    , m_importActionButton(nullptr)
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
    , m_saveAndApplyButton(nullptr)
    , m_actionConfigPanel(nullptr)
    , m_actionConfigTitleLabel(nullptr)
    , m_loopCheckBox(nullptr)
    , m_repeatLabel(nullptr)
    , m_repeatSpinBox(nullptr)
    , m_animationSpeedCheckBox(nullptr)
    , m_animationSpeedLabel(nullptr)
    , m_animationSpeedComboBox(nullptr)
    , m_moveEnabledCheckBox(nullptr)
    , m_speedLabel(nullptr)
    , m_speedComboBox(nullptr)
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
    contentLayout->setContentsMargins(24, 10, 24, 20);
    contentLayout->setSpacing(12);

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(24);

    m_titleLabel = new QLabel(tr("动作设置"), m_contentWidget);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(QString(
        "color: %1;"
        "background-color: transparent;"
        "padding: 0;"
        "margin: 0;"
    ).arg(theme.textPrimaryColor()));
    m_titleLabel->setMargin(0);
    titleLayout->addWidget(m_titleLabel);

    m_saveConfigButton = new QPushButton(tr("保存配置"), m_contentWidget);
    m_saveConfigButton->setMinimumHeight(32);
    m_saveConfigButton->setStyleSheet(theme.primaryButtonStyleSheet());
    titleLayout->addWidget(m_saveConfigButton);

    m_saveAndApplyButton = new QPushButton(tr("保存并应用"), m_contentWidget);
    m_saveAndApplyButton->setMinimumHeight(32);
    m_saveAndApplyButton->setStyleSheet(theme.primaryButtonStyleSheet());
    titleLayout->addWidget(m_saveAndApplyButton);

    titleLayout->addStretch();

    contentLayout->addLayout(titleLayout);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, m_contentWidget);
    splitter->setStyleSheet(theme.splitterStyleSheet());
    splitter->setHandleWidth(1);

    QWidget *leftPanel = new QWidget(splitter);
    leftPanel->setMinimumWidth(240);
    QVBoxLayout *leftLayout = new QVBoxLayout(leftPanel);
    leftLayout->setContentsMargins(0, 0, 0, 0);
    leftLayout->setSpacing(12);

    QHBoxLayout *libraryTitleLayout = new QHBoxLayout();
    libraryTitleLayout->setSpacing(12);

    m_libraryTitleLabel = new QLabel(tr("动作库"), leftPanel);
    QFont libTitleFont = m_libraryTitleLabel->font();
    libTitleFont.setPointSize(12);
    libTitleFont.setBold(true);
    m_libraryTitleLabel->setFont(libTitleFont);
    m_libraryTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(theme.textPrimaryColor()));
    libraryTitleLayout->addWidget(m_libraryTitleLabel);

    m_addActionButton = new QPushButton(tr("添加"), leftPanel);
    m_addActionButton->setMinimumSize(96, 40);
    m_addActionButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    libraryTitleLayout->addWidget(m_addActionButton);

    m_importActionButton = new QPushButton(tr("导入"), leftPanel);
    m_importActionButton->setMinimumSize(96, 40);
    m_importActionButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    libraryTitleLayout->addWidget(m_importActionButton);

    libraryTitleLayout->addStretch();
    leftLayout->addLayout(libraryTitleLayout);

    m_actionLibraryList = new QListWidget(leftPanel);
    m_actionLibraryList->setStyleSheet(theme.listWidgetStyleSheet());
    m_actionLibraryList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_actionLibraryList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
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
    m_dailyActionList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_dailyActionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_dailyActionList->setDragDropMode(QAbstractItemView::InternalMove);
    m_dailyActionList->setDefaultDropAction(Qt::MoveAction);
    m_dailyActionList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_dailyActionList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_categoryTabs->addTab(m_dailyActionList, tr("日常动作"));

    m_randomActionList = new QListWidget(m_categoryTabs);
    m_randomActionList->setStyleSheet(theme.listWidgetStyleSheet());
    m_randomActionList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_randomActionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_randomActionList->setDragDropMode(QAbstractItemView::InternalMove);
    m_randomActionList->setDefaultDropAction(Qt::MoveAction);
    m_randomActionList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_randomActionList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_categoryTabs->addTab(m_randomActionList, tr("随机动作"));

    m_scheduledActionList = new QListWidget(m_categoryTabs);
    m_scheduledActionList->setStyleSheet(theme.listWidgetStyleSheet());
    m_scheduledActionList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scheduledActionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scheduledActionList->setDragDropMode(QAbstractItemView::InternalMove);
    m_scheduledActionList->setDefaultDropAction(Qt::MoveAction);
    m_scheduledActionList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_scheduledActionList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_categoryTabs->addTab(m_scheduledActionList, tr("定时动作"));

    m_emotionActionList = new QListWidget(m_categoryTabs);
    m_emotionActionList->setStyleSheet(theme.listWidgetStyleSheet());
    m_emotionActionList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_emotionActionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_emotionActionList->setDragDropMode(QAbstractItemView::InternalMove);
    m_emotionActionList->setDefaultDropAction(Qt::MoveAction);
    m_emotionActionList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_emotionActionList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_categoryTabs->addTab(m_emotionActionList, tr("情绪动作"));

    rightLayout->addWidget(m_categoryTabs, 1);

    m_moveUpButton = new QPushButton(tr("上移"), rightPanel);
    m_moveUpButton->setMinimumHeight(32);
    m_moveUpButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_moveUpButton->hide();

    m_moveDownButton = new QPushButton(tr("下移"), rightPanel);
    m_moveDownButton->setMinimumHeight(32);
    m_moveDownButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_moveDownButton->hide();

    m_removeButton = new QPushButton(tr("移除"), rightPanel);
    m_removeButton->setMinimumHeight(32);
    m_removeButton->setStyleSheet(theme.secondaryButtonStyleSheet());
    m_removeButton->hide();

    m_actionConfigPanel = new QFrame(rightPanel);
    m_actionConfigPanel->setObjectName("actionConfigPanel");
    m_actionConfigPanel->setStyleSheet(theme.cardStyleSheet("actionConfigPanel"));
    QVBoxLayout *configPanelOuterLayout = new QVBoxLayout(m_actionConfigPanel);
    configPanelOuterLayout->setContentsMargins(16, 10, 16, 10);
    configPanelOuterLayout->setSpacing(8);

    m_actionConfigTitleLabel = new QLabel(tr("当前动作配置"), m_actionConfigPanel);
    m_actionConfigTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                              .arg(theme.textPrimaryColor()));
    QFont configPanelTitleFont = m_actionConfigTitleLabel->font();
    configPanelTitleFont.setBold(true);
    m_actionConfigTitleLabel->setFont(configPanelTitleFont);
    configPanelOuterLayout->addWidget(m_actionConfigTitleLabel);

    QHBoxLayout *row1Layout = new QHBoxLayout();
    row1Layout->setSpacing(16);

    m_loopCheckBox = new QCheckBox(tr("循环播放"), m_actionConfigPanel);
    m_loopCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    row1Layout->addWidget(m_loopCheckBox);

    m_repeatLabel = new QLabel(tr("循环次数"), m_actionConfigPanel);
    m_repeatLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(theme.textSecondaryColor()));
    row1Layout->addWidget(m_repeatLabel);

    m_repeatSpinBox = new QSpinBox(m_actionConfigPanel);
    m_repeatSpinBox->setRange(0, 10);
    m_repeatSpinBox->setValue(1);
    m_repeatSpinBox->setMinimumWidth(120);
    m_repeatSpinBox->setFixedWidth(120);
    m_repeatSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_repeatSpinBox->setToolTip(tr("0 表示无限循环"));
    m_repeatSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    row1Layout->addWidget(m_repeatSpinBox);

    row1Layout->addStretch();
    configPanelOuterLayout->addLayout(row1Layout);

    QHBoxLayout *row2Layout = new QHBoxLayout();
    row2Layout->setSpacing(16);

    m_animationSpeedCheckBox = new QCheckBox(tr("倍速播放"), m_actionConfigPanel);
    m_animationSpeedCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    row2Layout->addWidget(m_animationSpeedCheckBox);

    m_animationSpeedLabel = new QLabel(tr("播放速度"), m_actionConfigPanel);
    m_animationSpeedLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                             .arg(theme.textSecondaryColor()));
    row2Layout->addWidget(m_animationSpeedLabel);

    m_animationSpeedComboBox = new QComboBox(m_actionConfigPanel);
    m_animationSpeedComboBox->addItem("0.1x", 0.1);
    m_animationSpeedComboBox->addItem("0.5x", 0.5);
    m_animationSpeedComboBox->addItem("0.8x", 0.8);
    m_animationSpeedComboBox->addItem("1.0x", 1.0);
    m_animationSpeedComboBox->addItem("1.5x", 1.5);
    m_animationSpeedComboBox->addItem("2.0x", 2.0);
    m_animationSpeedComboBox->addItem("2.5x", 2.5);
    m_animationSpeedComboBox->addItem("3.0x", 3.0);
    m_animationSpeedComboBox->addItem("4.0x", 4.0);
    m_animationSpeedComboBox->addItem("5.0x", 5.0);
    m_animationSpeedComboBox->setCurrentIndex(3);
    m_animationSpeedComboBox->setFixedWidth(100);
    m_animationSpeedComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    row2Layout->addWidget(m_animationSpeedComboBox);

    row2Layout->addStretch();
    configPanelOuterLayout->addLayout(row2Layout);

    QHBoxLayout *row3Layout = new QHBoxLayout();
    row3Layout->setSpacing(16);

    m_moveEnabledCheckBox = new QCheckBox(tr("移动播放"), m_actionConfigPanel);
    m_moveEnabledCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    row3Layout->addWidget(m_moveEnabledCheckBox);

    m_speedLabel = new QLabel(tr("移动速度"), m_actionConfigPanel);
    m_speedLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textSecondaryColor()));
    row3Layout->addWidget(m_speedLabel);

    m_speedComboBox = new QComboBox(m_actionConfigPanel);
    m_speedComboBox->addItem("0.1x", 0.1);
    m_speedComboBox->addItem("0.5x", 0.5);
    m_speedComboBox->addItem("0.8x", 0.8);
    m_speedComboBox->addItem("1.0x", 1.0);
    m_speedComboBox->addItem("1.5x", 1.5);
    m_speedComboBox->addItem("2.0x", 2.0);
    m_speedComboBox->addItem("2.5x", 2.5);
    m_speedComboBox->addItem("3.0x", 3.0);
    m_speedComboBox->addItem("4.0x", 4.0);
    m_speedComboBox->addItem("5.0x", 5.0);
    m_speedComboBox->setCurrentIndex(3);
    m_speedComboBox->setFixedWidth(100);
    m_speedComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    row3Layout->addWidget(m_speedComboBox);

    row3Layout->addStretch();
    configPanelOuterLayout->addLayout(row3Layout);

    rightLayout->addWidget(m_actionConfigPanel);

    splitter->addWidget(rightPanel);
    splitter->setSizes({250, 500});

    contentLayout->addWidget(splitter, 1);

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);
}

void ActionSettingsPage::initData()
{
    if (PetConfigManager::loadPetFromDirectory(PetPaths::defaultPetDirectory(), m_petInfo, m_actionLibrary, m_playlist)) {
        m_loadedSuccessfully = true;
    } else {
        m_loadedSuccessfully = false;
        m_actionLibraryList->clear();
        m_actionLibraryList->addItem(tr("宠物配置加载失败"));
        m_actionLibraryList->setEnabled(false);

        m_addActionButton->setEnabled(false);
        m_addToCategoryButton->setEnabled(false);
        m_moveUpButton->setEnabled(false);
        m_moveDownButton->setEnabled(false);
        m_removeButton->setEnabled(false);
        m_saveConfigButton->setEnabled(false);
        m_saveAndApplyButton->setEnabled(false);
        m_categoryTabs->setEnabled(false);
        m_actionConfigPanel->setEnabled(false);
    }
}

void ActionSettingsPage::connectSignals()
{
    connect(m_addActionButton, &QPushButton::clicked, this, &ActionSettingsPage::onImportGif);
    connect(m_importActionButton, &QPushButton::clicked, this, &ActionSettingsPage::onAddAction);
    connect(m_addToCategoryButton, &QPushButton::clicked, this, &ActionSettingsPage::onAddToCategory);
    connect(m_moveUpButton, &QPushButton::clicked, this, &ActionSettingsPage::onMoveUp);
    connect(m_moveDownButton, &QPushButton::clicked, this, &ActionSettingsPage::onMoveDown);
    connect(m_removeButton, &QPushButton::clicked, this, &ActionSettingsPage::onRemove);
    connect(m_saveConfigButton, &QPushButton::clicked, this, &ActionSettingsPage::onSaveConfig);
    connect(m_saveAndApplyButton, &QPushButton::clicked, this, &ActionSettingsPage::onSaveAndApplyConfig);
    connect(m_categoryTabs, &QTabWidget::currentChanged, this, &ActionSettingsPage::onTabChanged);

    connect(m_dailyActionList, &QListWidget::itemSelectionChanged, this, &ActionSettingsPage::onCategorySelectionChanged);
    connect(m_randomActionList, &QListWidget::itemSelectionChanged, this, &ActionSettingsPage::onCategorySelectionChanged);
    connect(m_scheduledActionList, &QListWidget::itemSelectionChanged, this, &ActionSettingsPage::onCategorySelectionChanged);
    connect(m_emotionActionList, &QListWidget::itemSelectionChanged, this, &ActionSettingsPage::onCategorySelectionChanged);

    connect(m_dailyActionList, &QListWidget::customContextMenuRequested, this, &ActionSettingsPage::onCategoryListContextMenu);
    connect(m_randomActionList, &QListWidget::customContextMenuRequested, this, &ActionSettingsPage::onCategoryListContextMenu);
    connect(m_scheduledActionList, &QListWidget::customContextMenuRequested, this, &ActionSettingsPage::onCategoryListContextMenu);
    connect(m_emotionActionList, &QListWidget::customContextMenuRequested, this, &ActionSettingsPage::onCategoryListContextMenu);

    connect(m_dailyActionList->model(), &QAbstractItemModel::rowsMoved, this, &ActionSettingsPage::onCategoryListRowsMoved);
    connect(m_randomActionList->model(), &QAbstractItemModel::rowsMoved, this, &ActionSettingsPage::onCategoryListRowsMoved);
    connect(m_scheduledActionList->model(), &QAbstractItemModel::rowsMoved, this, &ActionSettingsPage::onCategoryListRowsMoved);
    connect(m_emotionActionList->model(), &QAbstractItemModel::rowsMoved, this, &ActionSettingsPage::onCategoryListRowsMoved);

    connect(m_loopCheckBox, &QCheckBox::stateChanged, this, &ActionSettingsPage::onLoopChanged);
    connect(m_repeatSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ActionSettingsPage::onRepeatChanged);
    connect(m_animationSpeedCheckBox, &QCheckBox::stateChanged, this, &ActionSettingsPage::onAnimationSpeedCheckChanged);
    connect(m_animationSpeedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ActionSettingsPage::onAnimationSpeedChanged);
    connect(m_moveEnabledCheckBox, &QCheckBox::stateChanged, this, &ActionSettingsPage::onMoveEnabledChanged);
    connect(m_speedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ActionSettingsPage::onSpeedChanged);
}

void ActionSettingsPage::refreshActionLibraryList()
{
    m_actionLibraryList->clear();
    for (const PetAction &action : m_actionLibrary) {
        m_actionLibraryList->addItem(action.id);
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
    m_updatingCategoryList = true;
    int savedRow = list->currentRow();
    list->clear();
    for (int i = 0; i < actions.size(); ++i) {
        QListWidgetItem *item = new QListWidgetItem(formatActionDisplay(actions[i]));
        item->setData(Qt::UserRole, i);
        list->addItem(item);
    }
    if (savedRow >= 0 && savedRow < list->count()) {
        list->setCurrentRow(savedRow);
    }
    m_updatingCategoryList = false;
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
        QString intervalText;
        if (ref.intervalSeconds < 60) {
            intervalText = tr("每 %1 秒").arg(ref.intervalSeconds);
        } else {
            int minutes = ref.intervalSeconds / 60;
            intervalText = tr("每 %1 分钟").arg(minutes);
        }
        return QString("%1 (%2) - %3 / %4").arg(name, ref.actionId, intervalText, repeatText);
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
    QSignalBlocker animSpeedCheckBlocker(m_animationSpeedCheckBox);
    QSignalBlocker animSpeedBlocker(m_animationSpeedComboBox);
    QSignalBlocker moveBlocker(m_moveEnabledCheckBox);
    QSignalBlocker speedBlocker(m_speedComboBox);

    m_loopCheckBox->setChecked(ref.loop);
    m_repeatSpinBox->setValue(ref.repeat);
    m_moveEnabledCheckBox->setChecked(ref.moveEnabled);
    m_speedComboBox->setEnabled(ref.moveEnabled);

    int speedIndex = findSpeedIndex(ref.movementSpeed);
    m_speedComboBox->setCurrentIndex(speedIndex);

    bool customAnimSpeed = !qFuzzyCompare(ref.animationSpeed, 1.0);
    m_animationSpeedCheckBox->setChecked(customAnimSpeed);
    m_animationSpeedComboBox->setEnabled(customAnimSpeed);

    int animSpeedIndex = findSpeedIndex(customAnimSpeed ? ref.animationSpeed : 1.0);
    m_animationSpeedComboBox->setCurrentIndex(animSpeedIndex);
}

void ActionSettingsPage::setActionConfigPanelEnabled(bool enabled)
{
    m_actionConfigPanel->setEnabled(enabled);
    m_loopCheckBox->setEnabled(enabled);
    m_repeatSpinBox->setEnabled(enabled);
    m_animationSpeedCheckBox->setEnabled(enabled);
    m_moveEnabledCheckBox->setEnabled(enabled);

    if (!enabled) {
        QSignalBlocker loopBlocker(m_loopCheckBox);
        QSignalBlocker repeatBlocker(m_repeatSpinBox);
        QSignalBlocker animSpeedCheckBlocker(m_animationSpeedCheckBox);
        QSignalBlocker animSpeedBlocker(m_animationSpeedComboBox);
        QSignalBlocker moveBlocker(m_moveEnabledCheckBox);
        QSignalBlocker speedBlocker(m_speedComboBox);

        m_loopCheckBox->setChecked(false);
        m_repeatSpinBox->setValue(1);

        m_animationSpeedCheckBox->setChecked(false);
        m_animationSpeedComboBox->setEnabled(false);
        m_animationSpeedComboBox->setCurrentIndex(3);

        m_moveEnabledCheckBox->setChecked(false);
        m_speedComboBox->setEnabled(false);
        m_speedComboBox->setCurrentIndex(3);
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
    QList<double> speeds = {0.1, 0.5, 0.8, 1.0, 1.5, 2.0, 2.5, 3.0, 4.0, 5.0};

    int closestIndex = 3;
    double minDiff = qAbs(speeds[3] - speed);

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

    bool enabled = (state == Qt::Checked);
    ref.moveEnabled = enabled;
    m_speedComboBox->setEnabled(enabled);
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

void ActionSettingsPage::onAnimationSpeedCheckChanged(int state)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    bool checked = (state == Qt::Checked);
    m_animationSpeedComboBox->setEnabled(checked);

    if (checked) {
        double speed = m_animationSpeedComboBox->currentData().toDouble();
        ref.animationSpeed = speed;
    } else {
        ref.animationSpeed = 1.0;
        QSignalBlocker blocker(m_animationSpeedComboBox);
        int index = findSpeedIndex(1.0);
        m_animationSpeedComboBox->setCurrentIndex(index);
    }
    updateCurrentSelectedRef(ref);
}

bool ActionSettingsPage::saveCurrentPlaylist()
{
    if (!m_loadedSuccessfully) {
        return false;
    }

    QString petDir = PetPaths::defaultPetDirectory();
    QString playlistPath = QDir(petDir).filePath("playlist.json");

    return PetConfigManager::savePlaylistToJson(playlistPath, m_playlist);
}

void ActionSettingsPage::onSaveConfig()
{
    if (!m_loadedSuccessfully) {
        QMessageBox::warning(this, tr("保存失败"), tr("宠物配置未正确加载，无法保存。"));
        return;
    }

    if (saveCurrentPlaylist()) {
        QMessageBox::information(this, tr("保存成功"), tr("配置已保存"));
    } else {
        QMessageBox::warning(this, tr("保存失败"), tr("配置保存失败，请检查文件权限。"));
    }
}

void ActionSettingsPage::onSaveAndApplyConfig()
{
    if (!m_loadedSuccessfully) {
        QMessageBox::warning(this, tr("保存失败"), tr("宠物配置未正确加载，无法保存。"));
        return;
    }

    if (saveCurrentPlaylist()) {
        QMessageBox::information(this, tr("保存成功"), tr("配置已保存并应用"));
        emit applyConfigRequested();
    } else {
        QMessageBox::warning(this, tr("保存失败"), tr("配置保存失败，请检查文件权限。"));
    }
}

void ActionSettingsPage::onCategoryListRowsMoved()
{
    if (m_updatingCategoryList) return;

    QListWidget *list = currentCategoryList();
    if (!list) return;

    QList<PetActionRef> originalActions = currentCategoryActions();
    QList<PetActionRef> newOrder;

    for (int i = 0; i < list->count(); ++i) {
        QListWidgetItem *item = list->item(i);
        int originalIndex = item->data(Qt::UserRole).toInt();
        if (originalIndex >= 0 && originalIndex < originalActions.size()) {
            newOrder.append(originalActions[originalIndex]);
        }
    }

    int tabIndex = m_categoryTabs->currentIndex();
    switch (tabIndex) {
        case 0:
            m_playlist.setIdleActions(newOrder);
            break;
        case 1:
            m_playlist.setRandomActions(newOrder);
            break;
        case 2:
            m_playlist.setTimedActions(newOrder);
            break;
        case 3:
            m_playlist.setEmotionActions("happy", newOrder);
            break;
    }

    refreshCurrentCategoryList();
}

void ActionSettingsPage::onCategoryListContextMenu(const QPoint &pos)
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    QListWidgetItem *item = list->itemAt(pos);
    if (!item) return;

    list->setCurrentItem(item);

    QMenu contextMenu(tr("操作"), this);
    QAction *moveUpAction = contextMenu.addAction(tr("上移"));
    QAction *moveDownAction = contextMenu.addAction(tr("下移"));
    contextMenu.addSeparator();
    QAction *removeAction = contextMenu.addAction(tr("移除"));

    QAction *selectedAction = contextMenu.exec(list->mapToGlobal(pos));

    if (selectedAction == moveUpAction) {
        onMoveUp();
    } else if (selectedAction == moveDownAction) {
        onMoveDown();
    } else if (selectedAction == removeAction) {
        onRemove();
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

    m_titleLabel->setStyleSheet(QString(
        "color: %1;"
        "background-color: transparent;"
        "padding: 0;"
        "margin: 0;"
    ).arg(theme.textPrimaryColor()));

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
    m_saveAndApplyButton->setStyleSheet(theme.primaryButtonStyleSheet());

    m_actionConfigPanel->setStyleSheet(theme.cardStyleSheet("actionConfigPanel"));
    m_actionConfigTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                              .arg(theme.textPrimaryColor()));
    m_loopCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_repeatLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(theme.textSecondaryColor()));
    m_repeatSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    m_moveEnabledCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_speedLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(theme.textSecondaryColor()));
    m_speedComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    m_animationSpeedCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_animationSpeedLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                             .arg(theme.textSecondaryColor()));
    m_animationSpeedComboBox->setStyleSheet(theme.comboBoxStyleSheet());
}

void ActionSettingsPage::onAddAction()
{
    QString petDir = PetPaths::defaultPetDirectory();
    AddActionDialog dialog(petDir, this);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    ActionImportResult result = ActionImportService::registerExistingAction(
        petDir,
        m_petInfo,
        m_actionLibrary,
        m_playlist,
        dialog.actionId(),
        dialog.actionFolderPath(),
        dialog.fps(),
        dialog.targetCategory(),
        dialog.timedIntervalSeconds(),
        dialog.emotionName()
    );

    if (!result.success) {
        QMessageBox::warning(this, tr("提示"), result.message);
        return;
    }

    m_actionLibrary.clear();
    m_playlist.clearAll();
    if (!PetConfigManager::loadPetFromDirectory(petDir, m_petInfo, m_actionLibrary, m_playlist)) {
        QMessageBox::warning(this, tr("提示"), tr("重新加载宠物配置失败。"));
        return;
    }

    if (result.warning) {
        QMessageBox::warning(this, tr("提示"), result.message);
    } else if (!result.message.isEmpty()) {
        QMessageBox::information(this, tr("提示"), result.message);
    }

    refreshActionLibraryList();
    refreshCurrentCategoryList();
}

void ActionSettingsPage::onImportGif()
{
    QString petDir = PetPaths::defaultPetDirectory();
    ImportGifDialog dialog(petDir, this);

    if (dialog.exec() != QDialog::Accepted) {
        return;
    }

    ActionImportResult result = ActionImportService::importGifAction(
        petDir,
        m_petInfo,
        m_actionLibrary,
        m_playlist,
        dialog.gifPath(),
        dialog.actionId(),
        dialog.fps(),
        dialog.targetCategory(),
        dialog.timedIntervalSeconds(),
        dialog.emotionName()
    );

    if (!result.success) {
        QMessageBox::warning(this, tr("提示"), result.message);
        return;
    }

    m_actionLibrary.clear();
    m_playlist.clearAll();
    if (!PetConfigManager::loadPetFromDirectory(petDir, m_petInfo, m_actionLibrary, m_playlist)) {
        QMessageBox::warning(this, tr("提示"), tr("重新加载宠物配置失败。"));
        return;
    }

    if (result.warning) {
        QMessageBox::warning(this, tr("提示"), result.message);
    } else if (!result.message.isEmpty()) {
        QMessageBox::information(this, tr("提示"), result.message);
    }

    refreshActionLibraryList();
    refreshCurrentCategoryList();
}

void ActionSettingsPage::onCategoryTabChanged(int index)
