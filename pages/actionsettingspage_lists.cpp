#include "actionsettingspage.h"

#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "services/actionlibraryindexservice.h"
#include "widgets/actioncategorylistwidget.h"
#include "widgets/actionlibrarylistwidget.h"
#include "widgets/actioncategorytabwidget.h"

#include <QDir>
#include <QListWidgetItem>

namespace {

struct EmotionActionDisplayItem {
    QString emotion;
    int indexInEmotion;
    PetActionRef ref;
};

} // anonymous namespace

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

    ActionLibraryIndexService::ensureLibrary();

    QList<ActionLibraryEntry> entries = ActionLibraryIndexService::loadEntries();

    for (const ActionLibraryEntry &entry : entries) {
        QString dirName = entry.dir.isEmpty() ? entry.id : entry.dir;
        QString actionDir = QDir(PetPaths::actionsDirectory()).filePath(dirName);
        PetAction action = PetConfigManager::loadGlobalActionFromDirectory(entry.id, actionDir);

        if (!action.isValid()) {
            action.id = entry.id;
            action.name = entry.name.isEmpty() ? entry.id : entry.name;
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
            refreshEmotionCategoryList();
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

// 格式化动作显示文本
//
// 资源缺失时只标记 [资源缺失]，不自动删除 playlist 项。
// 避免用户误删资源后丢失配置。
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

    QString resourceMissingSuffix;
    if (!isActionResourceAvailable(actionId)) {
        resourceMissingSuffix = tr(" [资源缺失]");
    }

    if (tabIndex == 2) {
        QString triggerText;
        if (ref.timedTriggerMode == TimedTriggerMode::ClockTime) {
            triggerText = tr("每天 %1").arg(ref.triggerTime);
        } else {
            triggerText = tr("每 %1 秒").arg(ref.intervalSeconds);
        }
        if (showId) {
            return QString("%1 (%2) - %3 / %4%5").arg(namePart, actionId, triggerText, repeatText, resourceMissingSuffix);
        } else {
            return QString("%1 - %2 / %3%4").arg(namePart, triggerText, repeatText, resourceMissingSuffix);
        }
    } else if (tabIndex == 3) {
        if (showId) {
            return QString("%1 (%2) - %3 / %4%5").arg(namePart, actionId, ref.emotion, repeatText, resourceMissingSuffix);
        } else {
            return QString("%1 - %2 / %3%4").arg(namePart, ref.emotion, repeatText, resourceMissingSuffix);
        }
    } else {
        if (showId) {
            return QString("%1 (%2) - %3%4").arg(namePart, actionId, repeatText, resourceMissingSuffix);
        } else {
            return QString("%1 - %2%3").arg(namePart, repeatText, resourceMissingSuffix);
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

bool ActionSettingsPage::isActionResourceAvailable(const QString &actionId) const
{
    for (const PetAction &action : m_actionLibrary) {
        if (action.id == actionId) {
            return action.isValid() && action.frameCount > 0 && !action.frameFiles.isEmpty();
        }
    }
    return false;
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
        case 3: {
            // Emotion tab: return all emotion actions in supported order
            QList<PetActionRef> all;
            for (const QString &emotion : supportedEmotionKeys()) {
                all.append(m_playlist.emotionActions(emotion));
            }
            // Append any unknown emotions
            QMap<QString, QList<PetActionRef>> allMap = m_playlist.allEmotionActions();
            QStringList known = supportedEmotionKeys();
            for (auto it = allMap.constBegin(); it != allMap.constEnd(); ++it) {
                if (!known.contains(it.key())) {
                    all.append(it.value());
                }
            }
            return all;
        }
        default: return QList<PetActionRef>();
    }
}

PetActionRef ActionSettingsPage::currentSelectedRef() const
{
    QListWidget *list = currentCategoryList();
    if (!list) return PetActionRef();

    QListWidgetItem *item = list->currentItem();
    if (!item) return PetActionRef();

    int tabIndex = m_categoryTabs->currentIndex();

    if (tabIndex == 3) {
        // Emotion tab: read emotion and index from item data
        QString emotion = item->data(Qt::UserRole + 1).toString();
        int index = item->data(Qt::UserRole).toInt();
        QList<PetActionRef> refs = m_playlist.emotionActions(emotion);
        if (index >= 0 && index < refs.size()) {
            return refs[index];
        }
        return PetActionRef();
    }

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

    int tabIndex = m_categoryTabs->currentIndex();

    if (tabIndex == 3) {
        // Emotion tab: read emotion and index from item data
        QListWidgetItem *item = list->currentItem();
        if (!item) return false;
        QString emotion = item->data(Qt::UserRole + 1).toString();
        int index = item->data(Qt::UserRole).toInt();
        return m_playlist.updateEmotionActionAt(emotion, index, ref);
    }

    int row = list->currentRow();
    if (row < 0) return false;

    switch (tabIndex) {
        case 0: return m_playlist.updateIdleActionAt(row, ref);
        case 1: return m_playlist.updateRandomActionAt(row, ref);
        case 2: return m_playlist.updateTimedActionAt(row, ref);
        default: return false;
    }
}

QStringList ActionSettingsPage::supportedEmotionKeys() const
{
    return {"happy", "sad", "angry", "surprised", "fear", "confused"};
}

QString ActionSettingsPage::defaultEmotionForNewAction() const
{
    if (m_emotionConfigComboBox && !m_emotionConfigComboBox->currentText().trimmed().isEmpty()) {
        return m_emotionConfigComboBox->currentText().trimmed();
    }
    return QStringLiteral("happy");
}

void ActionSettingsPage::refreshEmotionCategoryList()
{
    m_updatingCategoryList = true;

    // Save current selection
    QString savedActionId;
    QString savedDisplayName;
    QString savedEmotion;
    QListWidgetItem *currentItem = m_emotionActionList->currentItem();
    if (currentItem) {
        PetActionRef savedRef = currentSelectedRef();
        if (savedRef.isValid()) {
            savedActionId = savedRef.actionId;
            savedDisplayName = savedRef.displayName;
            savedEmotion = currentItem->data(Qt::UserRole + 1).toString();
        }
    }

    m_emotionActionList->clear();

    // Build display list in supported emotion order
    QList<EmotionActionDisplayItem> displayItems;

    QStringList knownEmotions = supportedEmotionKeys();
    QMap<QString, QList<PetActionRef>> allEmotions = m_playlist.allEmotionActions();

    // First: supported emotions in order
    for (const QString &emotion : knownEmotions) {
        QList<PetActionRef> refs = m_playlist.emotionActions(emotion);
        for (int i = 0; i < refs.size(); ++i) {
            EmotionActionDisplayItem item;
            item.emotion = emotion;
            item.indexInEmotion = i;
            item.ref = refs[i];
            displayItems.append(item);
        }
    }

    // Then: unknown emotions (alphabetical)
    QStringList unknownEmotions;
    for (auto it = allEmotions.constBegin(); it != allEmotions.constEnd(); ++it) {
        if (!knownEmotions.contains(it.key())) {
            unknownEmotions.append(it.key());
        }
    }
    unknownEmotions.sort();
    for (const QString &emotion : unknownEmotions) {
        QList<PetActionRef> refs = m_playlist.emotionActions(emotion);
        for (int i = 0; i < refs.size(); ++i) {
            EmotionActionDisplayItem item;
            item.emotion = emotion;
            item.indexInEmotion = i;
            item.ref = refs[i];
            displayItems.append(item);
        }
    }

    // Populate list
    for (const EmotionActionDisplayItem &displayItem : displayItems) {
        QListWidgetItem *listItem = new QListWidgetItem(formatActionDisplay(displayItem.ref));
        listItem->setData(Qt::UserRole, displayItem.indexInEmotion);
        listItem->setData(Qt::UserRole + 1, displayItem.emotion);
        m_emotionActionList->addItem(listItem);
    }

    // Restore selection
    if (!savedActionId.isEmpty()) {
        for (int i = 0; i < m_emotionActionList->count(); ++i) {
            QListWidgetItem *item = m_emotionActionList->item(i);
            QString emotion = item->data(Qt::UserRole + 1).toString();
            int index = item->data(Qt::UserRole).toInt();
            QList<PetActionRef> refs = m_playlist.emotionActions(emotion);
            if (index >= 0 && index < refs.size()
                && refs[index].actionId == savedActionId
                && refs[index].displayName == savedDisplayName
                && emotion == savedEmotion) {
                m_emotionActionList->setCurrentItem(item);
                break;
            }
        }
    }

    m_updatingCategoryList = false;
}
