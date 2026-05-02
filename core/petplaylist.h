#ifndef PETPLAYLIST_H
#define PETPLAYLIST_H

#include <QList>
#include <QMap>
#include <QString>

struct PetActionRef
{
    QString actionId;
    bool loop;
    // repeat 规则：
    // 0 表示无限循环；
    // 1 表示播放一次；
    // 2 表示播放两次；
    // 大于 10 也视为无限循环。 
    int repeat;
    int intervalSeconds;
    QString emotion;

    PetActionRef();
    explicit PetActionRef(const QString &id);

    bool isValid() const;
};

class PetPlaylist
{
public:
    PetPlaylist();

    bool containsAction(const QList<PetActionRef> &list, const QString &actionId) const;

    bool addIdleAction(const PetActionRef &ref);
    bool addRandomAction(const PetActionRef &ref);
    bool addTimedAction(const PetActionRef &ref);
    bool addEmotionAction(const QString &emotion, const PetActionRef &ref);

    void removeIdleAction(const QString &actionId);
    void removeRandomAction(const QString &actionId);
    void removeTimedAction(const QString &actionId);
    void removeEmotionAction(const QString &emotion, const QString &actionId);

    void clearIdleActions();
    void clearRandomActions();
    void clearTimedActions();
    void clearEmotionActions(const QString &emotion);
    void clearAll();

    QList<PetActionRef> idleActions() const;
    QList<PetActionRef> randomActions() const;
    QList<PetActionRef> timedActions() const;
    QList<PetActionRef> emotionActions(const QString &emotion) const;
    QMap<QString, QList<PetActionRef>> allEmotionActions() const;

private:
    QList<PetActionRef> m_idleActions;
    QList<PetActionRef> m_randomActions;
    QList<PetActionRef> m_timedActions;
    QMap<QString, QList<PetActionRef>> m_emotionActions;
};

#endif // PETPLAYLIST_H
