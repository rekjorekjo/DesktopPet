#include "petplaylist.h"

PetActionRef::PetActionRef()
    : loop(false)
    , repeat(1)
    , intervalSeconds(0)
    , moveEnabled(false)
    , movementSpeed(1.0)
    , animationSpeed(1.0)
    , timedTriggerMode(TimedTriggerMode::Interval)
    , triggerTime("00:00")
{
}

PetActionRef::PetActionRef(const QString &id)
    : actionId(id)
    , loop(false)
    , repeat(1)
    , intervalSeconds(0)
    , moveEnabled(false)
    , movementSpeed(1.0)
    , animationSpeed(1.0)
    , timedTriggerMode(TimedTriggerMode::Interval)
    , triggerTime("00:00")
{
}

bool PetActionRef::isValid() const
{
    return !actionId.isEmpty();
}

PetPlaylist::PetPlaylist()
{
}

bool PetPlaylist::containsAction(const QList<PetActionRef> &list, const QString &actionId) const
{
    for (const PetActionRef &ref : list) {
        if (ref.actionId == actionId) {
            return true;
        }
    }
    return false;
}

bool PetPlaylist::addIdleAction(const PetActionRef &ref)
{
    if (!ref.isValid()) {
        return false;
    }
    m_idleActions.append(ref);
    return true;
}

bool PetPlaylist::addRandomAction(const PetActionRef &ref)
{
    if (!ref.isValid()) {
        return false;
    }
    m_randomActions.append(ref);
    return true;
}

bool PetPlaylist::addTimedAction(const PetActionRef &ref)
{
    if (!ref.isValid()) {
        return false;
    }
    m_timedActions.append(ref);
    return true;
}

bool PetPlaylist::addEmotionAction(const QString &emotion, const PetActionRef &ref)
{
    if (!ref.isValid() || emotion.isEmpty()) {
        return false;
    }
    if (!m_emotionActions.contains(emotion)) {
        m_emotionActions[emotion] = QList<PetActionRef>();
    }
    PetActionRef newRef = ref;
    newRef.emotion = emotion;
    m_emotionActions[emotion].append(newRef);
    return true;
}

bool PetPlaylist::removeIdleActionAt(int index)
{
    if (index < 0 || index >= m_idleActions.size()) {
        return false;
    }
    m_idleActions.removeAt(index);
    return true;
}

bool PetPlaylist::removeRandomActionAt(int index)
{
    if (index < 0 || index >= m_randomActions.size()) {
        return false;
    }
    m_randomActions.removeAt(index);
    return true;
}

bool PetPlaylist::removeTimedActionAt(int index)
{
    if (index < 0 || index >= m_timedActions.size()) {
        return false;
    }
    m_timedActions.removeAt(index);
    return true;
}

bool PetPlaylist::removeEmotionActionAt(const QString &emotion, int index)
{
    if (!m_emotionActions.contains(emotion)) {
        return false;
    }
    QList<PetActionRef> &list = m_emotionActions[emotion];
    if (index < 0 || index >= list.size()) {
        return false;
    }
    list.removeAt(index);
    return true;
}

bool PetPlaylist::updateIdleActionAt(int index, const PetActionRef &ref)
{
    if (!ref.isValid() || index < 0 || index >= m_idleActions.size()) {
        return false;
    }
    m_idleActions[index] = ref;
    return true;
}

bool PetPlaylist::updateRandomActionAt(int index, const PetActionRef &ref)
{
    if (!ref.isValid() || index < 0 || index >= m_randomActions.size()) {
        return false;
    }
    m_randomActions[index] = ref;
    return true;
}

bool PetPlaylist::updateTimedActionAt(int index, const PetActionRef &ref)
{
    if (!ref.isValid() || index < 0 || index >= m_timedActions.size()) {
        return false;
    }
    m_timedActions[index] = ref;
    return true;
}

bool PetPlaylist::updateEmotionActionAt(const QString &emotion, int index, const PetActionRef &ref)
{
    if (!ref.isValid() || emotion.isEmpty() || !m_emotionActions.contains(emotion)) {
        return false;
    }
    QList<PetActionRef> &list = m_emotionActions[emotion];
    if (index < 0 || index >= list.size()) {
        return false;
    }
    PetActionRef newRef = ref;
    newRef.emotion = emotion;
    list[index] = newRef;
    return true;
}

bool PetPlaylist::moveActionUp(QList<PetActionRef> &list, int index)
{
    if (index <= 0 || index >= list.size()) {
        return false;
    }
    list.swapItemsAt(index, index - 1);
    return true;
}

bool PetPlaylist::moveActionDown(QList<PetActionRef> &list, int index)
{
    if (index < 0 || index >= list.size() - 1) {
        return false;
    }
    list.swapItemsAt(index, index + 1);
    return true;
}

bool PetPlaylist::moveIdleActionUp(int index)
{
    return moveActionUp(m_idleActions, index);
}

bool PetPlaylist::moveIdleActionDown(int index)
{
    return moveActionDown(m_idleActions, index);
}

bool PetPlaylist::moveRandomActionUp(int index)
{
    return moveActionUp(m_randomActions, index);
}

bool PetPlaylist::moveRandomActionDown(int index)
{
    return moveActionDown(m_randomActions, index);
}

bool PetPlaylist::moveTimedActionUp(int index)
{
    return moveActionUp(m_timedActions, index);
}

bool PetPlaylist::moveTimedActionDown(int index)
{
    return moveActionDown(m_timedActions, index);
}

bool PetPlaylist::moveEmotionActionUp(const QString &emotion, int index)
{
    if (!m_emotionActions.contains(emotion)) {
        return false;
    }
    return moveActionUp(m_emotionActions[emotion], index);
}

bool PetPlaylist::moveEmotionActionDown(const QString &emotion, int index)
{
    if (!m_emotionActions.contains(emotion)) {
        return false;
    }
    return moveActionDown(m_emotionActions[emotion], index);
}

void PetPlaylist::clearIdleActions()
{
    m_idleActions.clear();
}

void PetPlaylist::clearRandomActions()
{
    m_randomActions.clear();
}

void PetPlaylist::clearTimedActions()
{
    m_timedActions.clear();
}

void PetPlaylist::clearEmotionActions(const QString &emotion)
{
    m_emotionActions.remove(emotion);
}

void PetPlaylist::clearAll()
{
    m_idleActions.clear();
    m_randomActions.clear();
    m_timedActions.clear();
    m_emotionActions.clear();
}

void PetPlaylist::setIdleActions(const QList<PetActionRef> &actions)
{
    m_idleActions = actions;
}

void PetPlaylist::setRandomActions(const QList<PetActionRef> &actions)
{
    m_randomActions = actions;
}

void PetPlaylist::setTimedActions(const QList<PetActionRef> &actions)
{
    m_timedActions = actions;
}

void PetPlaylist::setEmotionActions(const QString &emotion, const QList<PetActionRef> &actions)
{
    m_emotionActions[emotion] = actions;
}

QList<PetActionRef> PetPlaylist::idleActions() const
{
    return m_idleActions;
}

QList<PetActionRef> PetPlaylist::randomActions() const
{
    return m_randomActions;
}

QList<PetActionRef> PetPlaylist::timedActions() const
{
    return m_timedActions;
}

QList<PetActionRef> PetPlaylist::emotionActions(const QString &emotion) const
{
    return m_emotionActions.value(emotion);
}

QMap<QString, QList<PetActionRef>> PetPlaylist::allEmotionActions() const
{
    return m_emotionActions;
}

int PetPlaylist::removeActionReferences(const QString &actionId)
{
    int count = 0;

    for (int i = m_idleActions.size() - 1; i >= 0; --i) {
        if (m_idleActions[i].actionId == actionId) {
            m_idleActions.removeAt(i);
            ++count;
        }
    }

    for (int i = m_randomActions.size() - 1; i >= 0; --i) {
        if (m_randomActions[i].actionId == actionId) {
            m_randomActions.removeAt(i);
            ++count;
        }
    }

    for (int i = m_timedActions.size() - 1; i >= 0; --i) {
        if (m_timedActions[i].actionId == actionId) {
            m_timedActions.removeAt(i);
            ++count;
        }
    }

    for (const QString &emotion : m_emotionActions.keys()) {
        QList<PetActionRef> &list = m_emotionActions[emotion];
        for (int i = list.size() - 1; i >= 0; --i) {
            if (list[i].actionId == actionId) {
                list.removeAt(i);
                ++count;
            }
        }
    }

    return count;
}
