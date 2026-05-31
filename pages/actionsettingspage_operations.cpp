#include "actionsettingspage.h"

#include "services/actionlibraryservice.h"
#include "theme/thememanager.h"
#include "widgets/actionlibrarylistwidget.h"
#include "widgets/actioncategorylistwidget.h"
#include "widgets/actioncategorytabwidget.h"
#include "widgets/softmessagebox.h"
#include "widgets/softinputdialog.h"

#include <QAction>
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

    QAction *removeAction = menu.addAction(tr("移除动作"));
    connect(removeAction, &QAction::triggered, this, &ActionSettingsPage::onRemoveLibraryAction);

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

    QString baseDisplayName = action.name.isEmpty() ? actionId : action.name;

    int tabIndex = m_categoryTabs->currentIndex();
    QList<PetActionRef> existingRefs;

    switch (tabIndex) {
        case 0:
            existingRefs = m_playlist.idleActions();
            break;
        case 1:
            existingRefs = m_playlist.randomActions();
            break;
        case 2:
            existingRefs = m_playlist.timedActions();
            break;
        case 3:
            existingRefs = m_playlist.emotionActions(defaultEmotionForNewAction());
            break;
    }

    QString uniqueDisplayName = makeUniqueDisplayNameForCategory(baseDisplayName, actionId, existingRefs);

    PetActionRef ref(action.id);
    ref.displayName = uniqueDisplayName;
    ref.loop = false;
    ref.repeat = 1;
    ref.animationSpeed = 1.0;
    ref.moveEnabled = false;
    ref.movementSpeed = 1.0;

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
            ref.emotion = defaultEmotionForNewAction();
            success = m_playlist.addEmotionAction(ref.emotion, ref);
            break;
    }

    return success;
}

QString ActionSettingsPage::makeUniqueDisplayNameForCategory(
    const QString &baseDisplayName,
    const QString &actionId,
    const QList<PetActionRef> &refs) const
{
    auto isDuplicate = [&](const QString &displayName) -> bool {
        for (const PetActionRef &ref : refs) {
            if (ref.actionId == actionId && ref.displayName.trimmed() == displayName.trimmed()) {
                return true;
            }
        }
        return false;
    };

    if (!isDuplicate(baseDisplayName)) {
        return baseDisplayName;
    }

    int suffix = 2;
    while (true) {
        QString candidate = baseDisplayName + " " + QString::number(suffix);
        if (!isDuplicate(candidate)) {
            return candidate;
        }
        ++suffix;
        if (suffix > 1000) {
            break;
        }
    }

    return baseDisplayName;
}

void ActionSettingsPage::onRemoveLibraryAction()
{
    QString actionId = currentLibraryActionId();
    if (actionId.isEmpty()) {
        return;
    }

    SoftMessageBox::StandardButton reply = SoftMessageBox::question(
        this,
        tr("移除动作"),
        tr("确定要从动作库移除动作 %1 吗？\n\n动作将从动作库索引中移除，并从所有宠物的播放列表中清理引用，但动作文件仍保留。").arg(actionId),
        SoftMessageBox::Yes | SoftMessageBox::No,
        SoftMessageBox::No
    );

    if (reply != SoftMessageBox::Yes) {
        return;
    }

    ActionLibraryOperationResult result = ActionLibraryService::removeAction(actionId);
    if (!result.success) {
        SoftMessageBox::warning(this, tr("移除动作失败"), result.message);
        return;
    }

    initData();
    refreshActionLibraryList();
    refreshCurrentCategoryList();
    emit applyConfigRequested();

    if (result.warning) {
        SoftMessageBox::warning(this, tr("移除动作"), result.message);
        return;
    }

    SoftMessageBox::information(this, tr("移除动作"), result.message);
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
    emit applyConfigRequested();

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
    QString newActionId = SoftInputDialog::getText(
        this,
        tr("重命名动作 ID"),
        tr("请输入新的动作 ID:"),
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

    int tabIndex = m_categoryTabs->currentIndex();

    if (tabIndex == 3) {
        // Emotion tab: move within the same emotion
        QListWidgetItem *item = list->currentItem();
        if (!item) return;
        QString emotion = item->data(Qt::UserRole + 1).toString();
        int index = item->data(Qt::UserRole).toInt();
        if (index <= 0) return;

        if (m_playlist.moveEmotionActionUp(emotion, index)) {
            refreshEmotionCategoryList();
            // Re-select the moved item
            for (int i = 0; i < m_emotionActionList->count(); ++i) {
                QListWidgetItem *newItem = m_emotionActionList->item(i);
                if (newItem->data(Qt::UserRole + 1).toString() == emotion
                    && newItem->data(Qt::UserRole).toInt() == index - 1) {
                    m_emotionActionList->setCurrentItem(newItem);
                    break;
                }
            }
        }
        return;
    }

    int row = list->currentRow();
    if (row <= 0) return;

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

    int tabIndex = m_categoryTabs->currentIndex();

    if (tabIndex == 3) {
        // Emotion tab: move within the same emotion
        QListWidgetItem *item = list->currentItem();
        if (!item) return;
        QString emotion = item->data(Qt::UserRole + 1).toString();
        int index = item->data(Qt::UserRole).toInt();
        int emotionCount = m_playlist.emotionActions(emotion).size();
        if (index < 0 || index >= emotionCount - 1) return;

        if (m_playlist.moveEmotionActionDown(emotion, index)) {
            refreshEmotionCategoryList();
            // Re-select the moved item
            for (int i = 0; i < m_emotionActionList->count(); ++i) {
                QListWidgetItem *newItem = m_emotionActionList->item(i);
                if (newItem->data(Qt::UserRole + 1).toString() == emotion
                    && newItem->data(Qt::UserRole).toInt() == index + 1) {
                    m_emotionActionList->setCurrentItem(newItem);
                    break;
                }
            }
        }
        return;
    }

    int row = list->currentRow();
    if (row < 0 || row >= list->count() - 1) return;

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

    int tabIndex = m_categoryTabs->currentIndex();

    if (tabIndex == 3) {
        // Emotion tab: remove using emotion and index from item data
        QListWidgetItem *item = list->currentItem();
        if (!item) return;
        QString emotion = item->data(Qt::UserRole + 1).toString();
        int index = item->data(Qt::UserRole).toInt();
        if (m_playlist.removeEmotionActionAt(emotion, index)) {
            refreshEmotionCategoryList();
            clearCategorySelection();
        }
        return;
    }

    int row = list->currentRow();
    if (row < 0) return;

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
    QString newName = SoftInputDialog::getText(
        this,
        tr("重命名"),
        tr("请输入新的显示名称（留空则使用动作名称）："),
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

void ActionSettingsPage::onTabChanged(int index)
{
    Q_UNUSED(index);
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

    int tabIndex = m_categoryTabs->currentIndex();

    // Emotion tab uses NoDragDrop, so ignore rowsMoved
    if (tabIndex == 3) return;

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
    ThemeManager &theme = ThemeManager::instance();
    contextMenu.setStyleSheet(theme.menuStyleSheet());

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
