#include "actionsettingspage.h"

#include "theme/thememanager.h"

#include <QMenu>

void ActionSettingsPage::onActionLibraryContextMenu(const QPoint &pos)
{
    QListWidgetItem *item = m_actionLibraryList->itemAt(pos);
    if (!item) {
        return;
    }

    m_actionLibraryList->setCurrentItem(item);

    QMenu menu(this);
    ThemeManager &theme = ThemeManager::instance();
    menu.setStyleSheet(theme.menuStyleSheet());

    QAction *addAction = menu.addAction(tr("添加到当前分类"));
    connect(addAction, &QAction::triggered, this, &ActionSettingsPage::onAddToCategory);

    menu.exec(m_actionLibraryList->mapToGlobal(pos));
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
            ref.intervalSeconds = 300;
            ref.timedTriggerMode = TimedTriggerMode::Interval;
            ref.triggerTime = "00:00";
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

void ActionSettingsPage::clearCategorySelection()
{
    QListWidget *list = currentCategoryList();
    if (list) {
        list->clearSelection();
    }
    setActionConfigPanelEnabled(false);
}
