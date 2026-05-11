#ifndef ACTIONSETTINGSPAGE_H
#define ACTIONSETTINGSPAGE_H

#include <QLabel>
#include <QListWidget>
#include <QPointer>
#include <QScrollArea>
#include <QTime>
#include <QWidget>

#include "core/petconfigmanager.h"
#include "models/petaction.h"
#include "models/petplaylist.h"
#include "services/actionimportworker.h"
#include "widgets/softcardwidget.h"

class QCheckBox;
class QComboBox;
class QPushButton;
class QSpinBox;
class QTimeEdit;
class QThread;
class ActionCategoryListWidget;
class ActionCategoryTabWidget;
class ActionLibraryListWidget;
struct ActionImportResult;
class NewActionDialog;
class ImportActionDialog;

// 动作配置页，管理全局动作库和当前宠物 playlist
//
// 职责：
// - 管理全局动作库（左侧列表），支持新建、导入、移除、删除动作
// - 管理当前宠物的播放列表（右侧分类标签页），支持添加、移除、排序播放项
// - 配置播放项参数（循环、速度、移动等）
// - 保存配置到 playlist.json
//
// 重要语义：
// - 左侧是全局动作库，所有宠物共享
// - 右侧是当前宠物的播放列表，每个宠物独立
// - playlist 是播放项列表，不是 actionId 集合
// - 同一个 actionId 可以多次出现在 playlist 中
// - 资源缺失时只标记 [资源缺失]，不自动删除播放项
//
// 保存并应用：
// - 必须同时刷新 actionlibrary 和 playlist
// - 否则 player 可能仍然使用旧动作缓存
class ActionSettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ActionSettingsPage(QWidget *parent = nullptr);
    ~ActionSettingsPage();

public slots:
    void refreshTheme();
    void reloadData();

signals:
    void applyConfigRequested();

private:
    void setupUi();
    void applyTheme();
    void initData();
    void connectSignals();

    void refreshActionLibraryList();
    void refreshCurrentCategoryList();
    void refreshCategoryList(QListWidget *list, const QList<PetActionRef> &actions);
    void loadGlobalActionLibrary();

    // 格式化动作显示文本
    // 资源缺失时只标记 [资源缺失]，不自动删除 playlist 项
    // 避免用户误删资源后丢失配置
    QString formatActionDisplay(const PetActionRef &ref) const;
    QString getActionName(const QString &actionId) const;
    QString displayNameForRef(const PetActionRef &ref) const;
    bool isActionResourceAvailable(const QString &actionId) const;
    QString currentLibraryActionId() const;
    PetAction findLibraryActionById(const QString &actionId) const;
    bool addActionIdToCurrentCategory(const QString &actionId);
    QString makeUniqueDisplayNameForCategory(
        const QString &baseDisplayName,
        const QString &actionId,
        const QList<PetActionRef> &refs) const;

    QListWidget* currentCategoryList() const;
    QList<PetActionRef> currentCategoryActions() const;

    PetActionRef currentSelectedRef() const;
    bool updateCurrentSelectedRef(const PetActionRef &ref);
    void updateActionConfigPanel();
    void setActionConfigPanelEnabled(bool enabled);
    void clearCategorySelection();
    int findSpeedIndex(double speed) const;
    bool saveCurrentPlaylist();

    void startImportFolderTask(const ActionImportWorker::ImportFolderTask &task, QPointer<ImportActionDialog> dialog = nullptr);
    void startImportGifTask(const ActionImportWorker::ImportGifTask &task, QPointer<NewActionDialog> dialog = nullptr);
    void startImportBatchTask(const ActionImportWorker::ImportBatchTask &task, QPointer<ImportActionDialog> dialog = nullptr);

private slots:
    void onActionLibraryContextMenu(const QPoint &pos);
    void onAddToCategory();
    void onRemoveLibraryAction();
    void onDeleteLibraryAction();
    void onRenameActionId();
    void onMoveUp();
    void onMoveDown();
    void onRemove();
    void onRenameCategoryAction();
    void onTabChanged(int index);
    void onCategorySelectionChanged();
    void onLoopChanged(int state);
    void onRepeatChanged(int value);
    void onAnimationSpeedCheckChanged(int state);
    void onAnimationSpeedChanged(int index);
    void onMoveEnabledChanged(int state);
    void onSpeedChanged(int index);
    void onMoveAxisChanged(int index);
    void onTimedTriggerModeChanged(int index);
    void onTimedIntervalChanged(int value);
    void onTriggerTimeChanged(const QTime &time);
    void onSaveConfig();
    void onSaveAndApplyConfig();
    void onCategoryListRowsMoved();
    void onCategoryListContextMenu(const QPoint &pos);
    void onImportAction();
    void onNewAction();

private:
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;

    QLabel *m_titleLabel;

    QLabel *m_libraryTitleLabel;
    SoftCardWidget *m_libraryCard;
    ActionLibraryListWidget *m_actionLibraryList;
    QPushButton *m_newActionButton;
    QPushButton *m_importActionButton;

    QLabel *m_configTitleLabel;
    SoftCardWidget *m_configCard;
    ActionCategoryTabWidget *m_categoryTabs;
    ActionCategoryListWidget *m_dailyActionList;
    ActionCategoryListWidget *m_randomActionList;
    ActionCategoryListWidget *m_scheduledActionList;
    ActionCategoryListWidget *m_emotionActionList;

    QPushButton *m_moveUpButton;
    QPushButton *m_moveDownButton;
    QPushButton *m_removeButton;
    QPushButton *m_saveConfigButton;
    QPushButton *m_saveAndApplyButton;

    SoftCardWidget *m_actionConfigPanel;
    QLabel *m_actionConfigTitleLabel;
    QCheckBox *m_loopCheckBox;
    QLabel *m_repeatLabel;
    QSpinBox *m_repeatSpinBox;
    QCheckBox *m_animationSpeedCheckBox;
    QLabel *m_animationSpeedLabel;
    QComboBox *m_animationSpeedComboBox;
    QCheckBox *m_moveEnabledCheckBox;
    QLabel *m_speedLabel;
    QComboBox *m_speedComboBox;
    QLabel *m_moveAxisLabel;
    QComboBox *m_moveAxisComboBox;

    QLabel *m_timedTriggerModeLabel;
    QComboBox *m_timedTriggerModeComboBox;
    QLabel *m_timedIntervalLabel;
    QSpinBox *m_timedIntervalSpinBox;
    QLabel *m_triggerTimeLabel;
    QTimeEdit *m_triggerTimeEdit;

    // 全局动作库缓存，从 actionlibrary.json 加载
    QList<PetAction> m_actionLibrary;
    PetBasicInfo m_petInfo;

    // 当前宠物的播放列表，从 playlist.json 加载
    PetPlaylist m_playlist;

    // 加载状态标志
    bool m_loadedSuccessfully;

    // 防止列表更新时触发重复信号
    bool m_updatingCategoryList = false;
    QThread *m_importThread = nullptr;
};

#endif // ACTIONSETTINGSPAGE_H
