#include "actionsettingspage.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "widgets/actioncategorylistwidget.h"
#include "widgets/actionlibrarylistwidget.h"
#include "widgets/actioncategorytabwidget.h"

#include <QDir>
#include <QListWidgetItem>

void ActionSettingsPage::refreshActionLibraryList()
{
    m_actionLibraryList->clear();

    for (const PetAction &action : m_actionLibrary) {
        QListWidgetItem *item = new QListWidgetItem(action.id, m_actionLibraryList);
        item->setData(Qt::UserRole, action.id);
    }
}

void ActionSettingsPage::loadGlobalActionLibrary()
{
    m_actionLibrary.clear();

    QString actionsDir = PetPaths::actionsDirectory();
    QDir dir(actionsDir);
    if (!dir.exists()) {
        return;
    }

    QStringList actionFolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &actionId : actionFolders) {
        QString actionDir = dir.filePath(actionId);
        PetAction action = PetConfigManager::loadGlobalActionFromDirectory(actionId, actionDir);

        if (!action.isValid()) {
            continue;
        }

        m_actionLibrary.append(action);
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

QString ActionSettingsPage::displayNameForRef(const PetActionRef &ref) const
{
    if (!ref.displayName.trimmed().isEmpty()) {
        return ref.displayName.trimmed();
    }
    return getActionName(ref.actionId);
}

QString ActionSettingsPage::formatActionDisplay(const PetActionRef &ref) const
{
    QString name = displayNameForRef(ref);
    QString trimmedName = name.trimmed();
    QString actionId = ref.actionId;

    bool showId = !trimmedName.isEmpty() && trimmedName != actionId;
    QString namePart = trimmedName.isEmpty() ? actionId : trimmedName;

    int tabIndex = m_categoryTabs->currentIndex();

    QString repeatText;
    if (ref.loop && ref.repeat == 0) {
        repeatText = tr("无限循环");
    } else if (ref.loop) {
        repeatText = tr("播放 %1 次").arg(qBound(1, ref.repeat, 10));
    } else {
        repeatText = tr("播放 1 次");
    }

    if (tabIndex == 2) {
        QString triggerText;
        if (ref.timedTriggerMode == TimedTriggerMode::ClockTime) {
            triggerText = tr("每天 %1").arg(ref.triggerTime);
        } else {
            triggerText = tr("每 %1 秒").arg(ref.intervalSeconds);
        }
        if (showId) {
            return QString("%1 (%2) - %3 / %4").arg(namePart, actionId, triggerText, repeatText);
        } else {
            return QString("%1 - %2 / %3").arg(namePart, triggerText, repeatText);
        }
    } else if (tabIndex == 3) {
        if (showId) {
            return QString("%1 (%2) - %3 / %4").arg(namePart, actionId, ref.emotion, repeatText);
        } else {
            return QString("%1 - %2 / %3").arg(namePart, ref.emotion, repeatText);
        }
    } else {
        if (showId) {
            return QString("%1 (%2) - %3").arg(namePart, actionId, repeatText);
        } else {
            return QString("%1 - %2").arg(namePart, repeatText);
        }
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

QString ActionSettingsPage::currentLibraryActionId() const
{
    QListWidgetItem *item = m_actionLibraryList->currentItem();
    if (!item) {
        return QString();
    }
    return item->data(Qt::UserRole).toString();
}

PetAction ActionSettingsPage::findLibraryActionById(const QString &actionId) const
{
    for (const PetAction &action : m_actionLibrary) {
        if (action.id == actionId) {
            return action;
        }
    }
    return PetAction();
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
