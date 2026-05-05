#ifndef ACTIONSETTINGSPAGE_H
#define ACTIONSETTINGSPAGE_H

#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QTime>
#include <QWidget>

#include "core/petaction.h"
#include "core/petconfigmanager.h"
#include "core/petplaylist.h"

class QCheckBox;
class QComboBox;
class QFrame;
class QPushButton;
class QSpinBox;
class QTimeEdit;
class ActionCategoryListWidget;
class ActionCategoryTabWidget;
class ActionLibraryListWidget;

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

    QString formatActionDisplay(const PetActionRef &ref) const;
    QString getActionName(const QString &actionId) const;
    QString displayNameForRef(const PetActionRef &ref) const;
    QString currentLibraryActionId() const;
    PetAction findLibraryActionById(const QString &actionId) const;
    bool addActionIdToCurrentCategory(const QString &actionId);

    QListWidget* currentCategoryList() const;
    QList<PetActionRef> currentCategoryActions() const;

    PetActionRef currentSelectedRef() const;
    bool updateCurrentSelectedRef(const PetActionRef &ref);
    void updateActionConfigPanel();
    void setActionConfigPanelEnabled(bool enabled);
    void clearCategorySelection();
    int findSpeedIndex(double speed) const;
    bool saveCurrentPlaylist();

private slots:
    void onActionLibraryContextMenu(const QPoint &pos);
    void onAddToCategory();
    void onDeleteLibraryAction();
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
    ActionLibraryListWidget *m_actionLibraryList;
    QPushButton *m_newActionButton;
    QPushButton *m_importActionButton;

    QLabel *m_configTitleLabel;
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

    QFrame *m_actionConfigPanel;
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

    QLabel *m_timedTriggerModeLabel;
    QComboBox *m_timedTriggerModeComboBox;
    QLabel *m_timedIntervalLabel;
    QSpinBox *m_timedIntervalSpinBox;
    QLabel *m_triggerTimeLabel;
    QTimeEdit *m_triggerTimeEdit;

    QList<PetAction> m_actionLibrary;
    PetBasicInfo m_petInfo;
    PetPlaylist m_playlist;
    bool m_loadedSuccessfully;
    bool m_updatingCategoryList = false;
};

#endif // ACTIONSETTINGSPAGE_H
