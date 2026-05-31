#include "actionsettingspage.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "theme/thememanager.h"
#include "widgets/actioncategorylistwidget.h"
#include "widgets/actioncategorytabwidget.h"
#include "widgets/actionlibrarylistwidget.h"

#include <QAbstractItemView>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QListWidgetItem>
#include <QMimeData>
#include <QPushButton>
#include <QSizePolicy>
#include <QSpinBox>
#include <QSplitter>
#include <QThread>
#include <QTimeEdit>
#include <QtGlobal>
#include <QVBoxLayout>

ActionSettingsPage::ActionSettingsPage(QWidget *parent)
    : QWidget(parent)
    , m_scrollArea(nullptr)
    , m_contentWidget(nullptr)
    , m_titleLabel(nullptr)
    , m_libraryTitleLabel(nullptr)
    , m_actionLibraryList(nullptr)
    , m_newActionButton(nullptr)
    , m_importActionButton(nullptr)
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
    , m_timedTriggerModeLabel(nullptr)
    , m_timedTriggerModeComboBox(nullptr)
    , m_timedIntervalLabel(nullptr)
    , m_timedIntervalSpinBox(nullptr)
    , m_triggerTimeLabel(nullptr)
    , m_triggerTimeEdit(nullptr)
    , m_emotionConfigLabel(nullptr)
    , m_emotionConfigComboBox(nullptr)
    , m_loadedSuccessfully(false)
{
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
    setupUi();
    initData();
    connectSignals();

    if (m_loadedSuccessfully) {
        refreshActionLibraryList();
        refreshCurrentCategoryList();
    }
    setActionConfigPanelEnabled(false);
}

ActionSettingsPage::~ActionSettingsPage()
{
    if (m_importThread && m_importThread->isRunning()) {
        m_importThread->quit();
        m_importThread->wait();
    }
}

void ActionSettingsPage::setupUi()
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

    QHBoxLayout *titleLayout = new QHBoxLayout();
    titleLayout->setSpacing(24);

    m_titleLabel = new QLabel(tr("动作设置"), m_contentWidget);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(18);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);
    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());
    m_titleLabel->setMargin(0);
    titleLayout->addWidget(m_titleLabel);

    titleLayout->addStretch();

    m_saveConfigButton = new QPushButton(tr("保存配置"), m_contentWidget);
    m_saveConfigButton->setMinimumHeight(32);
    m_saveConfigButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    titleLayout->addWidget(m_saveConfigButton);

    m_saveAndApplyButton = new QPushButton(tr("保存并应用"), m_contentWidget);
    m_saveAndApplyButton->setMinimumHeight(32);
    m_saveAndApplyButton->setStyleSheet(theme.softButtonStyleSheet(6, 55));
    titleLayout->addWidget(m_saveAndApplyButton);

    contentLayout->addLayout(titleLayout);

    QSplitter *splitter = new QSplitter(Qt::Horizontal, m_contentWidget);
    splitter->setStyleSheet(theme.splitterStyleSheet());
    splitter->setHandleWidth(6);

    m_libraryCard = new SoftCardWidget(splitter);
    m_libraryCard->setObjectName("libraryCard");
    m_libraryCard->setMinimumWidth(240);
    QVBoxLayout *leftLayout = new QVBoxLayout(m_libraryCard);
    leftLayout->setContentsMargins(16, 16, 16, 16);
    leftLayout->setSpacing(12);

    QHBoxLayout *libraryTitleLayout = new QHBoxLayout();
    libraryTitleLayout->setSpacing(12);

    m_libraryTitleLabel = new QLabel(tr("动作库"), m_libraryCard);
    QFont libTitleFont = m_libraryTitleLabel->font();
    libTitleFont.setPointSize(12);
    libTitleFont.setBold(true);
    m_libraryTitleLabel->setFont(libTitleFont);
    m_libraryTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(p.subtitleText));
    libraryTitleLayout->addWidget(m_libraryTitleLabel);
    libraryTitleLayout->addStretch();
    leftLayout->addLayout(libraryTitleLayout);

    QHBoxLayout *libraryButtonLayout = new QHBoxLayout();
    libraryButtonLayout->setSpacing(8);

    m_newActionButton = new QPushButton(tr("新建动作"), m_libraryCard);
    m_newActionButton->setMinimumHeight(40);
    m_newActionButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_newActionButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    libraryButtonLayout->addWidget(m_newActionButton);

    m_importActionButton = new QPushButton(tr("导入动作"), m_libraryCard);
    m_importActionButton->setMinimumHeight(40);
    m_importActionButton->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_importActionButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    libraryButtonLayout->addWidget(m_importActionButton);

    leftLayout->addLayout(libraryButtonLayout);

    m_actionLibraryList = new ActionLibraryListWidget(m_libraryCard);
    m_actionLibraryList->setStyleSheet(theme.listWidgetStyleSheet());
    m_actionLibraryList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_actionLibraryList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_actionLibraryList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_actionLibraryList->setDragEnabled(true);
    m_actionLibraryList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_actionLibraryList->setToolTip(tr("右键动作可添加到当前分类，或拖拽到右侧分类"));
    leftLayout->addWidget(m_actionLibraryList, 1);

    splitter->addWidget(m_libraryCard);

    m_configCard = new SoftCardWidget(splitter);
    m_configCard->setObjectName("configCard");
    QVBoxLayout *rightLayout = new QVBoxLayout(m_configCard);
    rightLayout->setContentsMargins(16, 16, 16, 16);
    rightLayout->setSpacing(12);

    m_configTitleLabel = new QLabel(tr("动作分类配置"), m_configCard);
    QFont configTitleFont = m_configTitleLabel->font();
    configTitleFont.setPointSize(12);
    configTitleFont.setBold(true);
    m_configTitleLabel->setFont(configTitleFont);
    m_configTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(p.subtitleText));
    rightLayout->addWidget(m_configTitleLabel);

    m_categoryTabs = new ActionCategoryTabWidget(m_configCard);
    m_categoryTabs->setStyleSheet(theme.innerTabWidgetStyleSheet());

    m_dailyActionList = new ActionCategoryListWidget(m_categoryTabs);
    m_dailyActionList->setStyleSheet(theme.innerListWidgetStyleSheet());
    m_dailyActionList->viewport()->setAutoFillBackground(false);
    m_dailyActionList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_dailyActionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_dailyActionList->setDragDropMode(QAbstractItemView::InternalMove);
    m_dailyActionList->setDefaultDropAction(Qt::MoveAction);
    m_dailyActionList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_dailyActionList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_dailyActionList->setEmptyStateText(tr("暂无可用动作"), tr("请前往动作库添加动作"));
    m_categoryTabs->addTab(m_dailyActionList, tr("日常动作"));

    m_randomActionList = new ActionCategoryListWidget(m_categoryTabs);
    m_randomActionList->setStyleSheet(theme.innerListWidgetStyleSheet());
    m_randomActionList->viewport()->setAutoFillBackground(false);
    m_randomActionList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_randomActionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_randomActionList->setDragDropMode(QAbstractItemView::InternalMove);
    m_randomActionList->setDefaultDropAction(Qt::MoveAction);
    m_randomActionList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_randomActionList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_randomActionList->setEmptyStateText(tr("暂无可用动作"), tr("请前往动作库添加动作"));
    m_categoryTabs->addTab(m_randomActionList, tr("随机动作"));

    m_scheduledActionList = new ActionCategoryListWidget(m_categoryTabs);
    m_scheduledActionList->setStyleSheet(theme.innerListWidgetStyleSheet());
    m_scheduledActionList->viewport()->setAutoFillBackground(false);
    m_scheduledActionList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_scheduledActionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_scheduledActionList->setDragDropMode(QAbstractItemView::InternalMove);
    m_scheduledActionList->setDefaultDropAction(Qt::MoveAction);
    m_scheduledActionList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_scheduledActionList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_scheduledActionList->setEmptyStateText(tr("暂无可用动作"), tr("请前往动作库添加动作"));
    m_categoryTabs->addTab(m_scheduledActionList, tr("定时动作"));

    m_emotionActionList = new ActionCategoryListWidget(m_categoryTabs);
    m_emotionActionList->setStyleSheet(theme.innerListWidgetStyleSheet());
    m_emotionActionList->viewport()->setAutoFillBackground(false);
    m_emotionActionList->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_emotionActionList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_emotionActionList->setDragDropMode(QAbstractItemView::DropOnly);
    m_emotionActionList->setSelectionMode(QAbstractItemView::SingleSelection);
    m_emotionActionList->setContextMenuPolicy(Qt::CustomContextMenu);
    m_emotionActionList->setEmptyStateText(tr("暂无可用动作"), tr("请前往动作库添加动作"));
    m_categoryTabs->addTab(m_emotionActionList, tr("情绪动作"));

    rightLayout->addWidget(m_categoryTabs, 1);

    m_moveUpButton = new QPushButton(tr("上移"), m_configCard);
    m_moveUpButton->setMinimumHeight(32);
    m_moveUpButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_moveUpButton->hide();

    m_moveDownButton = new QPushButton(tr("下移"), m_configCard);
    m_moveDownButton->setMinimumHeight(32);
    m_moveDownButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_moveDownButton->hide();

    m_removeButton = new QPushButton(tr("移除"), m_configCard);
    m_removeButton->setMinimumHeight(32);
    m_removeButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_removeButton->hide();

    m_actionConfigPanel = new SoftCardWidget(m_configCard);
    m_actionConfigPanel->setObjectName("actionConfigPanel");
    m_actionConfigPanel->setBorderRadius(10);
    m_actionConfigPanel->setBackgroundOpacity(60);
    m_actionConfigPanel->setHighlightOpacity(30);
    m_actionConfigPanel->setShadowOpacity(20);
    m_actionConfigPanel->setBorderOpacity(70);
    QVBoxLayout *configPanelOuterLayout = new QVBoxLayout(m_actionConfigPanel);
    configPanelOuterLayout->setContentsMargins(16, 10, 16, 10);
    configPanelOuterLayout->setSpacing(6);

    m_actionConfigTitleLabel = new QLabel(tr("当前动作配置"), m_actionConfigPanel);
    m_actionConfigTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                              .arg(theme.textPrimaryColor()));
    QFont configPanelTitleFont = m_actionConfigTitleLabel->font();
    configPanelTitleFont.setBold(true);
    m_actionConfigTitleLabel->setFont(configPanelTitleFont);
    configPanelOuterLayout->addWidget(m_actionConfigTitleLabel);

    // Row 1: 循环 [checkbox] | 次数 [spinbox]
    QHBoxLayout *row1Layout = new QHBoxLayout();
    row1Layout->setSpacing(6);

    m_loopCheckBox = new QCheckBox(tr("循环"), m_actionConfigPanel);
    m_loopCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    row1Layout->addWidget(m_loopCheckBox);

    m_repeatLabel = new QLabel(tr("次数"), m_actionConfigPanel);
    m_repeatLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(theme.textSecondaryColor()));
    row1Layout->addWidget(m_repeatLabel);

    m_repeatSpinBox = new QSpinBox(m_actionConfigPanel);
    m_repeatSpinBox->setRange(1, 10);
    m_repeatSpinBox->setValue(1);
    m_repeatSpinBox->setFixedWidth(88);
    m_repeatSpinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_repeatSpinBox->setToolTip(tr("0 表示无限循环"));
    m_repeatSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    row1Layout->addWidget(m_repeatSpinBox);

    row1Layout->addStretch();
    configPanelOuterLayout->addLayout(row1Layout);

    // Row 2: 倍速 [checkbox] | 速度 [combo]
    QHBoxLayout *row2Layout = new QHBoxLayout();
    row2Layout->setSpacing(6);

    m_animationSpeedCheckBox = new QCheckBox(tr("倍速"), m_actionConfigPanel);
    m_animationSpeedCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    row2Layout->addWidget(m_animationSpeedCheckBox);

    m_animationSpeedLabel = new QLabel(tr("速度"), m_actionConfigPanel);
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
    m_animationSpeedComboBox->setFixedWidth(86);
    m_animationSpeedComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    row2Layout->addWidget(m_animationSpeedComboBox);

    row2Layout->addStretch();
    configPanelOuterLayout->addLayout(row2Layout);

    // Row 3: 移动 [checkbox] | 速度 [combo] | 方向 [combo]
    QHBoxLayout *row3Layout = new QHBoxLayout();
    row3Layout->setSpacing(6);

    m_moveEnabledCheckBox = new QCheckBox(tr("移动"), m_actionConfigPanel);
    m_moveEnabledCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    row3Layout->addWidget(m_moveEnabledCheckBox);

    m_speedLabel = new QLabel(tr("速度"), m_actionConfigPanel);
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
    m_speedComboBox->setFixedWidth(86);
    m_speedComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    row3Layout->addWidget(m_speedComboBox);

    m_moveAxisLabel = new QLabel(tr("方向"), m_actionConfigPanel);
    m_moveAxisLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(theme.textSecondaryColor()));
    row3Layout->addWidget(m_moveAxisLabel);

    m_moveAxisComboBox = new QComboBox(m_actionConfigPanel);
    m_moveAxisComboBox->addItem(tr("随机"), static_cast<int>(MoveAxis::Random));
    m_moveAxisComboBox->addItem(tr("水平"), static_cast<int>(MoveAxis::Horizontal));
    m_moveAxisComboBox->addItem(tr("竖直"), static_cast<int>(MoveAxis::Vertical));
    m_moveAxisComboBox->setCurrentIndex(0);
    m_moveAxisComboBox->setFixedWidth(100);
    m_moveAxisComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    row3Layout->addWidget(m_moveAxisComboBox);

    row3Layout->addStretch();
    configPanelOuterLayout->addLayout(row3Layout);

    // Row 4: 对应情绪 [combo] (only visible for emotion tab)
    QHBoxLayout *rowEmotionLayout = new QHBoxLayout();
    rowEmotionLayout->setSpacing(6);

    m_emotionConfigLabel = new QLabel(tr("对应情绪"), m_actionConfigPanel);
    m_emotionConfigLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                            .arg(theme.textSecondaryColor()));
    rowEmotionLayout->addWidget(m_emotionConfigLabel);

    m_emotionConfigComboBox = new QComboBox(m_actionConfigPanel);
    m_emotionConfigComboBox->addItem("happy");
    m_emotionConfigComboBox->addItem("sad");
    m_emotionConfigComboBox->addItem("angry");
    m_emotionConfigComboBox->addItem("surprised");
    m_emotionConfigComboBox->addItem("fear");
    m_emotionConfigComboBox->addItem("confused");
    m_emotionConfigComboBox->setFixedWidth(130);
    m_emotionConfigComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    rowEmotionLayout->addWidget(m_emotionConfigComboBox);

    rowEmotionLayout->addStretch();
    configPanelOuterLayout->addLayout(rowEmotionLayout);

    // Timed trigger rows (hidden by default)
    QHBoxLayout *rowTimedModeLayout = new QHBoxLayout();
    rowTimedModeLayout->setSpacing(8);

    m_timedTriggerModeLabel = new QLabel(tr("触发方式"), m_actionConfigPanel);
    m_timedTriggerModeLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                               .arg(theme.textPrimaryColor()));
    rowTimedModeLayout->addWidget(m_timedTriggerModeLabel);

    m_timedTriggerModeComboBox = new QComboBox(m_actionConfigPanel);
    m_timedTriggerModeComboBox->addItem(tr("每隔一段时间"), static_cast<int>(TimedTriggerMode::Interval));
    m_timedTriggerModeComboBox->addItem(tr("指定时间"), static_cast<int>(TimedTriggerMode::ClockTime));
    m_timedTriggerModeComboBox->setFixedWidth(120);
    m_timedTriggerModeComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    rowTimedModeLayout->addWidget(m_timedTriggerModeComboBox);

    rowTimedModeLayout->addStretch();
    configPanelOuterLayout->addLayout(rowTimedModeLayout);

    QHBoxLayout *rowTimedIntervalLayout = new QHBoxLayout();
    rowTimedIntervalLayout->setSpacing(8);

    m_timedIntervalLabel = new QLabel(tr("间隔"), m_actionConfigPanel);
    m_timedIntervalLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                            .arg(theme.textPrimaryColor()));
    rowTimedIntervalLayout->addWidget(m_timedIntervalLabel);

    m_timedIntervalSpinBox = new QSpinBox(m_actionConfigPanel);
    m_timedIntervalSpinBox->setRange(1, 86400);
    m_timedIntervalSpinBox->setValue(300);
    m_timedIntervalSpinBox->setSuffix(tr(" 秒"));
    m_timedIntervalSpinBox->setFixedWidth(120);
    m_timedIntervalSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    rowTimedIntervalLayout->addWidget(m_timedIntervalSpinBox);

    rowTimedIntervalLayout->addStretch();
    configPanelOuterLayout->addLayout(rowTimedIntervalLayout);

    QHBoxLayout *rowTriggerTimeLayout = new QHBoxLayout();
    rowTriggerTimeLayout->setSpacing(8);

    m_triggerTimeLabel = new QLabel(tr("时间"), m_actionConfigPanel);
    m_triggerTimeLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                          .arg(theme.textPrimaryColor()));
    rowTriggerTimeLayout->addWidget(m_triggerTimeLabel);

    m_triggerTimeEdit = new QTimeEdit(m_actionConfigPanel);
    m_triggerTimeEdit->setDisplayFormat("HH:mm");
    m_triggerTimeEdit->setTime(QTime(0, 0));
    m_triggerTimeEdit->setFixedWidth(100);
    m_triggerTimeEdit->setStyleSheet(theme.timeEditStyleSheet());
    rowTriggerTimeLayout->addWidget(m_triggerTimeEdit);

    rowTriggerTimeLayout->addStretch();
    configPanelOuterLayout->addLayout(rowTriggerTimeLayout);

    // Hide timed trigger controls by default
    m_emotionConfigLabel->hide();
    m_emotionConfigComboBox->hide();
    m_timedTriggerModeLabel->hide();
    m_timedTriggerModeComboBox->hide();
    m_timedIntervalLabel->hide();
    m_timedIntervalSpinBox->hide();
    m_triggerTimeLabel->hide();
    m_triggerTimeEdit->hide();

    rightLayout->addWidget(m_actionConfigPanel);

    splitter->addWidget(m_configCard);
    splitter->setSizes({250, 500});

    contentLayout->addWidget(splitter, 1);

    m_scrollArea->setWidget(m_contentWidget);
    outerLayout->addWidget(m_scrollArea, 1);
}

void ActionSettingsPage::initData()
{
    loadGlobalActionLibrary();

    QString petDir = PetPaths::currentPetDirectory();
    QString petJsonPath = petDir + "/pet.json";
    QString playlistPath = petDir + "/playlist.json";

    bool petLoaded = PetConfigManager::loadPetInfoJson(petJsonPath, m_petInfo);

    if (!petLoaded) {
        m_loadedSuccessfully = false;
        m_actionLibraryList->clear();
        m_actionLibraryList->addItem(tr("宠物配置加载失败"));
        m_actionLibraryList->setEnabled(false);

        m_newActionButton->setEnabled(false);
        m_importActionButton->setEnabled(false);
        m_moveUpButton->setEnabled(false);
        m_moveDownButton->setEnabled(false);
        m_removeButton->setEnabled(false);
        m_saveConfigButton->setEnabled(false);
        m_saveAndApplyButton->setEnabled(false);
        m_categoryTabs->setEnabled(false);
        m_actionConfigPanel->setEnabled(false);
        return;
    }

    bool playlistLoaded = PetConfigManager::loadPlaylistFromJson(playlistPath, m_playlist);

    if (!playlistLoaded) {
        if (!QFile::exists(playlistPath)) {
            m_playlist = PetPlaylist();
            PetConfigManager::savePlaylistToJson(playlistPath, m_playlist);
        } else {
            qWarning() << "Playlist exists but failed to parse:" << playlistPath;
            m_loadedSuccessfully = false;
            m_actionLibraryList->clear();
            m_actionLibraryList->addItem(tr("播放列表解析失败"));
            m_actionLibraryList->setEnabled(false);

            m_newActionButton->setEnabled(false);
            m_importActionButton->setEnabled(false);
            m_moveUpButton->setEnabled(false);
            m_moveDownButton->setEnabled(false);
            m_removeButton->setEnabled(false);
            m_saveConfigButton->setEnabled(false);
            m_saveAndApplyButton->setEnabled(false);
            m_categoryTabs->setEnabled(false);
            m_actionConfigPanel->setEnabled(false);
            return;
        }
    }

    m_loadedSuccessfully = true;
}

void ActionSettingsPage::connectSignals()
{
    connect(m_newActionButton, &QPushButton::clicked, this, &ActionSettingsPage::onNewAction);
    connect(m_importActionButton, &QPushButton::clicked, this, &ActionSettingsPage::onImportAction);
    connect(m_actionLibraryList, &QListWidget::customContextMenuRequested, this, &ActionSettingsPage::onActionLibraryContextMenu);
    connect(m_moveUpButton, &QPushButton::clicked, this, &ActionSettingsPage::onMoveUp);
    connect(m_moveDownButton, &QPushButton::clicked, this, &ActionSettingsPage::onMoveDown);
    connect(m_removeButton, &QPushButton::clicked, this, &ActionSettingsPage::onRemove);
    connect(m_saveConfigButton, &QPushButton::clicked, this, &ActionSettingsPage::onSaveConfig);
    connect(m_saveAndApplyButton, &QPushButton::clicked, this, &ActionSettingsPage::onSaveAndApplyConfig);
    connect(m_categoryTabs, &ActionCategoryTabWidget::currentChanged, this, &ActionSettingsPage::onTabChanged);

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

    connect(m_dailyActionList, &ActionCategoryListWidget::actionDropped, this, [this](const QString &actionId) {
        m_categoryTabs->setCurrentIndex(0);
        if (addActionIdToCurrentCategory(actionId)) {
            refreshCurrentCategoryList();
        }
    });
    connect(m_randomActionList, &ActionCategoryListWidget::actionDropped, this, [this](const QString &actionId) {
        m_categoryTabs->setCurrentIndex(1);
        if (addActionIdToCurrentCategory(actionId)) {
            refreshCurrentCategoryList();
        }
    });
    connect(m_scheduledActionList, &ActionCategoryListWidget::actionDropped, this, [this](const QString &actionId) {
        m_categoryTabs->setCurrentIndex(2);
        if (addActionIdToCurrentCategory(actionId)) {
            refreshCurrentCategoryList();
        }
    });
    connect(m_emotionActionList, &ActionCategoryListWidget::actionDropped, this, [this](const QString &actionId) {
        m_categoryTabs->setCurrentIndex(3);
        if (addActionIdToCurrentCategory(actionId)) {
            refreshCurrentCategoryList();
        }
    });

    connect(m_loopCheckBox, &QCheckBox::stateChanged, this, &ActionSettingsPage::onLoopChanged);
    connect(m_repeatSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ActionSettingsPage::onRepeatChanged);
    connect(m_animationSpeedCheckBox, &QCheckBox::stateChanged, this, &ActionSettingsPage::onAnimationSpeedCheckChanged);
    connect(m_animationSpeedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ActionSettingsPage::onAnimationSpeedChanged);
    connect(m_moveEnabledCheckBox, &QCheckBox::stateChanged, this, &ActionSettingsPage::onMoveEnabledChanged);
    connect(m_speedComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ActionSettingsPage::onSpeedChanged);
    connect(m_moveAxisComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ActionSettingsPage::onMoveAxisChanged);
    connect(m_timedTriggerModeComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ActionSettingsPage::onTimedTriggerModeChanged);
    connect(m_timedIntervalSpinBox, QOverload<int>::of(&QSpinBox::valueChanged), this, &ActionSettingsPage::onTimedIntervalChanged);
    connect(m_triggerTimeEdit, &QTimeEdit::timeChanged, this, &ActionSettingsPage::onTriggerTimeChanged);
    connect(m_emotionConfigComboBox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, &ActionSettingsPage::onEmotionConfigChanged);
}

void ActionSettingsPage::refreshTheme()
{
    applyTheme();
}

void ActionSettingsPage::reloadData()
{
    initData();

    if (m_loadedSuccessfully) {
        m_actionLibraryList->setEnabled(true);
        m_newActionButton->setEnabled(true);
        m_importActionButton->setEnabled(true);
        m_categoryTabs->setEnabled(true);
        m_saveConfigButton->setEnabled(true);
        m_saveAndApplyButton->setEnabled(true);

        refreshActionLibraryList();
        refreshCurrentCategoryList();
        updateActionConfigPanel();
    }
}

void ActionSettingsPage::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    m_scrollArea->setStyleSheet(theme.softScrollAreaStyleSheet());
    m_contentWidget->setStyleSheet(theme.softPageStyleSheet());

    m_titleLabel->setStyleSheet(theme.softTitleLabelStyleSheet());

    m_libraryTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                         .arg(p.subtitleText));
    m_actionLibraryList->setStyleSheet(theme.listWidgetStyleSheet());
    m_newActionButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));
    m_importActionButton->setStyleSheet(theme.softSecondaryButtonStyleSheet(6, 24));

    m_configTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(p.subtitleText));
    m_categoryTabs->setStyleSheet(theme.innerTabWidgetStyleSheet());
    m_dailyActionList->setStyleSheet(theme.innerListWidgetStyleSheet());
    m_randomActionList->setStyleSheet(theme.innerListWidgetStyleSheet());
    m_scheduledActionList->setStyleSheet(theme.innerListWidgetStyleSheet());
    m_emotionActionList->setStyleSheet(theme.innerListWidgetStyleSheet());

    m_dailyActionList->applyTheme();
    m_randomActionList->applyTheme();
    m_scheduledActionList->applyTheme();
    m_emotionActionList->applyTheme();

    m_moveUpButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_moveDownButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_removeButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_saveConfigButton->setStyleSheet(theme.softButtonStyleSheet(6, 40));
    m_saveAndApplyButton->setStyleSheet(theme.softButtonStyleSheet(6, 55));

    m_actionConfigTitleLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                              .arg(p.subtitleText));
    m_loopCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_repeatLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                   .arg(p.textSecondary));
    m_repeatSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    m_moveEnabledCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_speedLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                    .arg(p.textSecondary));
    m_speedComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    m_moveAxisLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                        .arg(p.textSecondary));
    m_moveAxisComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    m_animationSpeedCheckBox->setStyleSheet(theme.checkBoxStyleSheet());
    m_animationSpeedLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                             .arg(p.textSecondary));
    m_animationSpeedComboBox->setStyleSheet(theme.comboBoxStyleSheet());

    m_timedTriggerModeLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                               .arg(p.textPrimary));
    m_timedTriggerModeComboBox->setStyleSheet(theme.comboBoxStyleSheet());
    m_timedIntervalLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                            .arg(p.textPrimary));
    m_timedIntervalSpinBox->setStyleSheet(theme.spinBoxStyleSheet());
    m_triggerTimeLabel->setStyleSheet(QString("color: %1; border: none; background: transparent;")
                                          .arg(p.textPrimary));
    m_triggerTimeEdit->setStyleSheet(theme.timeEditStyleSheet());
}
