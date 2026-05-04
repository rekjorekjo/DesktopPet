#ifndef ACTIONSETTINGSPAGE_H
#define ACTIONSETTINGSPAGE_H

#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QTabWidget>
#include <QWidget>

#include "core/petaction.h"
#include "core/petconfigmanager.h"
#include "core/petplaylist.h"

class QCheckBox;
class QComboBox;
class QFrame;
class QPushButton;
class QSpinBox;

class ActionSettingsPage : public QWidget
{
    Q_OBJECT

public:
    explicit ActionSettingsPage(QWidget *parent = nullptr);
    ~ActionSettingsPage();

public slots:
    void refreshTheme();

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

    QString formatActionDisplay(const PetActionRef &ref) const;
    QString getActionName(const QString &actionId) const;

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
    void onMoveUp();
    void onMoveDown();
    void onRemove();
    void onTabChanged(int index);
    void onCategorySelectionChanged();
    void onLoopChanged(int state);
    void onRepeatChanged(int value);
    void onAnimationSpeedCheckChanged(int state);
    void onAnimationSpeedChanged(int index);
    void onMoveEnabledChanged(int state);
    void onSpeedChanged(int index);
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
    QListWidget *m_actionLibraryList;
    QPushButton *m_newActionButton;
    QPushButton *m_importActionButton;

    QLabel *m_configTitleLabel;
    QTabWidget *m_categoryTabs;
    QListWidget *m_dailyActionList;
    QListWidget *m_randomActionList;
    QListWidget *m_scheduledActionList;
    QListWidget *m_emotionActionList;

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

    QList<PetAction> m_actionLibrary;
    PetBasicInfo m_petInfo;
    PetPlaylist m_playlist;
    bool m_loadedSuccessfully;
    bool m_updatingCategoryList = false;
};

#endif // ACTIONSETTINGSPAGE_H
