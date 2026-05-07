#include "actionsettingspage.h"

#include "services/actionlibraryservice.h"
#include "theme/thememanager.h"
#include "widgets/actionlibrarylistwidget.h"
#include "widgets/actioncategorylistwidget.h"
#include "widgets/actioncategorytabwidget.h"
#include "widgets/softmessagebox.h"

#include <QAction>
#include <QInputDialog>
#include <QListWidgetItem>
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

    menu.addSeparator();

    QAction *renameAction = menu.addAction(tr("重命名动作 ID"));
    connect(renameAction, &QAction::triggered, this, &ActionSettingsPage::onRenameActionId);

    QAction *deleteAction = menu.addAction(tr("删除动作"));
    connect(deleteAction, &QAction::triggered, this, &ActionSettingsPage::onDeleteLibraryAction);

    menu.exec(m_actionLibraryList->mapToGlobal(pos));
}

void ActionSettingsPage::onAddToCategory()
{
    QString actionId = currentLibraryActionId();
    if (actionId.isEmpty()) {
        return;
    }

    if (addActionIdToCurrentCategory(actionId)) {
        refreshCurrentCategoryList();
    }
}

bool ActionSettingsPage::addActionIdToCurrentCategory(const QString &actionId)
{
    if (actionId.isEmpty()) {
        return false;
    }

    PetAction action = findLibraryActionById(actionId);
    if (!action.isValid()) {
        return false;
    }

    PetActionRef ref(action.id);
    ref.loop = false;
    ref.repeat = 1;
    ref.animationSpeed = 1.0;
    ref.moveEnabled = false;
    ref.movementSpeed = 1.0;

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

    return success;
}

void ActionSettingsPage::onDeleteLibraryAction()
{
    QString actionId = currentLibraryActionId();
    if (actionId.isEmpty()) {
        return;
    }

    SoftMessageBox::StandardButton reply = SoftMessageBox::question(
        this,
        tr("删除动作"),
        tr("确定要删除动作 %1 吗？\n\n动作将从全局动作库中删除，并从所有宠物的播放列表中移除！此操作不可撤销！").arg(actionId),
        SoftMessageBox::Yes | SoftMessageBox::No,
        SoftMessageBox::No
    );

    if (reply != SoftMessageBox::Yes) {
        return;
    }

    ActionLibraryOperationResult result = ActionLibraryService::deleteAction(actionId);
    if (!result.success) {
        SoftMessageBox::warning(this, tr("删除动作失败"), result.message);
        return;
    }

    initData();
    refreshActionLibraryList();
    refreshCurrentCategoryList();

    if (result.warning) {
        SoftMessageBox::warning(this, tr("删除动作"), result.message);
        return;
    }

    SoftMessageBox::information(this, tr("删除动作"), result.message);
}

void ActionSettingsPage::onRenameActionId()
{
    QString oldActionId = currentLibraryActionId();
    if (oldActionId.isEmpty()) {
        return;
    }

    bool ok = false;
    QString newActionId = QInputDialog::getText(
        this,
        tr("重命名动作 ID"),
        tr("请输入新的动作 ID:"),
        QLineEdit::Normal,
        oldActionId,
        &ok
    );

    if (!ok) {
        return;
    }

    newActionId = newActionId.trimmed();
    if (newActionId.isEmpty()) {
        SoftMessageBox::warning(this, tr("重命名动作 ID"), tr("动作 ID 不能为空。"));
        return;
    }

    if (newActionId == oldActionId) {
        SoftMessageBox::information(this, tr("重命名动作 ID"), tr("新动作 ID 与原动作 ID 相同，无需修改。"));
        return;
    }

    ActionLibraryOperationResult result = ActionLibraryService::renameActionId(oldActionId, newActionId);
    if (!result.success) {
        SoftMessageBox::warning(this, tr("重命名动作 ID 失败"), result.message);
        return;
    }

    loadGlobalActionLibrary();
    refreshActionLibraryList();

    for (int i = 0; i < m_actionLibraryList->count(); ++i) {
        QListWidgetItem *item = m_actionLibraryList->item(i);
        if (item && item->data(Qt::UserRole).toString() == newActionId) {
            m_actionLibraryList->setCurrentItem(item);
            break;
        }
    }

    m_playlist.replaceActionReferences(oldActionId, newActionId);
    refreshCurrentCategoryList();
    updateActionConfigPanel();

    if (result.warning) {
        SoftMessageBox::warning(this, tr("重命名动作 ID"), result.message);
        return;
    }

    SoftMessageBox::information(this, tr("重命名动作 ID"), result.message);
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

void ActionSettingsPage::onRenameCategoryAction()
{
    QListWidget *list = currentCategoryList();
    if (!list) return;

    int row = list->currentRow();
    if (row < 0) return;

    PetActionRef ref = currentSelectedRef();
    if (!ref.isValid()) return;

    QString defaultName = displayNameForRef(ref);

    bool ok = false;
    QString newName = QInputDialog::getText(
        this,
        tr("重命名"),
        tr("请输入新的显示名称（留空则使用动作名称）："),
        QLineEdit::Normal,
        defaultName,
        &ok
    );

    if (!ok) {
        return;
    }

    ref.displayName = newName.trimmed();

    if (updateCurrentSelectedRef(ref)) {
        refreshCurrentCategoryList();
        list->setCurrentRow(row);
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
    QAction *renameAction = contextMenu.addAction(tr("重命名"));
    QAction *moveUpAction = contextMenu.addAction(tr("上移"));
    QAction *moveDownAction = contextMenu.addAction(tr("下移"));
    contextMenu.addSeparator();
    QAction *removeAction = contextMenu.addAction(tr("移除"));

    QAction *selectedAction = contextMenu.exec(list->mapToGlobal(pos));

    if (selectedAction == renameAction) {
        onRenameCategoryAction();
    } else if (selectedAction == moveUpAction) {
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
