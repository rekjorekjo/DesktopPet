#include "petplaylist.h"

PetActionRef::PetActionRef()
    : loop(false)
    , repeat(1)
    , intervalSeconds(0)
{
}

PetActionRef::PetActionRef(const QString &id)
    : actionId(id)
    , loop(false)
    , repeat(1)
    , intervalSeconds(0)
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
    if (!ref.isValid() || containsAction(m_idleActions, ref.actionId)) {
        return false;
    }
    m_idleActions.append(ref);
    return true;
}

bool PetPlaylist::addRandomAction(const PetActionRef &ref)
{
    if (!ref.isValid() || containsAction(m_randomActions, ref.actionId)) {
        return false;
    }
    m_randomActions.append(ref);
    return true;
}

bool PetPlaylist::addTimedAction(const PetActionRef &ref)
{
    if (!ref.isValid() || containsAction(m_timedActions, ref.actionId)) {
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
    if (containsAction(m_emotionActions[emotion], ref.actionId)) {
        return false;
    }
    m_emotionActions[emotion].append(ref);
    return true;
}

void PetPlaylist::removeIdleAction(const QString &actionId)
{
    for (int i = m_idleActions.size() - 1; i >= 0; --i) {
        if (m_idleActions[i].actionId == actionId) {
            m_idleActions.removeAt(i);
        }
    }
}

void PetPlaylist::removeRandomAction(const QString &actionId)
{
    for (int i = m_randomActions.size() - 1; i >= 0; --i) {
        if (m_randomActions[i].actionId == actionId) {
            m_randomActions.removeAt(i);
        }
    }
}

void PetPlaylist::removeTimedAction(const QString &actionId)
{
    for (int i = m_timedActions.size() - 1; i >= 0; --i) {
        if (m_timedActions[i].actionId == actionId) {
            m_timedActions.removeAt(i);
        }
    }
}

void PetPlaylist::removeEmotionAction(const QString &emotion, const QString &actionId)
{
    if (!m_emotionActions.contains(emotion)) {
        return;
    }
    QList<PetActionRef> &list = m_emotionActions[emotion];
    for (int i = list.size() - 1; i >= 0; --i) {
        if (list[i].actionId == actionId) {
            list.removeAt(i);
        }
    }
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
