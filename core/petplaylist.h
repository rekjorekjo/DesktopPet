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
    // - repeat = 0 表示无限循环
    // - repeat = 1 表示播放一次
    // - repeat = 2 表示播放两次
    // - repeat > 10 也视为无限循环
    int repeat;
    int intervalSeconds;
    QString emotion;
    // 移动配置
    bool moveEnabled;
    double movementSpeed;

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

    bool removeIdleActionAt(int index);
    bool removeRandomActionAt(int index);
    bool removeTimedActionAt(int index);
    bool removeEmotionActionAt(const QString &emotion, int index);

    bool updateIdleActionAt(int index, const PetActionRef &ref);
    bool updateRandomActionAt(int index, const PetActionRef &ref);
    bool updateTimedActionAt(int index, const PetActionRef &ref);
    bool updateEmotionActionAt(const QString &emotion, int index, const PetActionRef &ref);

    bool moveIdleActionUp(int index);
    bool moveIdleActionDown(int index);
    bool moveRandomActionUp(int index);
    bool moveRandomActionDown(int index);
    bool moveTimedActionUp(int index);
    bool moveTimedActionDown(int index);
    bool moveEmotionActionUp(const QString &emotion, int index);
    bool moveEmotionActionDown(const QString &emotion, int index);

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
    bool moveActionUp(QList<PetActionRef> &list, int index);
    bool moveActionDown(QList<PetActionRef> &list, int index);

    QList<PetActionRef> m_idleActions;
    QList<PetActionRef> m_randomActions;
    QList<PetActionRef> m_timedActions;
    QMap<QString, QList<PetActionRef>> m_emotionActions;
};

#endif // PETPLAYLIST_H
