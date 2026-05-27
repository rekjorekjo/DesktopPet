#include "petwidget.h"

#include "core/appsettings.h"
#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "services/actionlibraryindexservice.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QRandomGenerator>
#include <QTimer>

void PetWidget::loadGlobalActionLibrary()
{
    m_actions.clear();

    ActionLibraryIndexService::ensureLibrary();

    QList<ActionLibraryEntry> entries = ActionLibraryIndexService::loadEntries();

    for (const ActionLibraryEntry &entry : entries) {
        QString dirName = entry.dir.isEmpty() ? entry.id : entry.dir;
        QString actionDirPath = QDir(PetPaths::actionsDirectory()).filePath(dirName);
        PetAction action = PetConfigManager::loadGlobalActionFromDirectory(entry.id, actionDirPath);

        if (!action.isValid()) {
            qWarning() << "Failed to load action from actionlibrary.json:" << entry.id;
            continue;
        }

        m_actions.append(action);
    }
}

bool PetWidget::playAction(const PetAction &action, const PetActionRef &ref)
{
    stopMovement();

    if (!action.isValid()) {
        showStatusMessage(tr("动作无效"), QString());
        return false;
    }

    QSize displaySize = currentDisplaySize();
    setFixedSize(displaySize);
    m_displayLabel->setFixedSize(displaySize);
    clampPetToScreen();
    keepPetAboveChat();

    if (!m_player->loadAction(action, displaySize)) {
        showStatusMessage(tr("动作加载失败"), QString());
        return false;
    }

    m_player->setSpeedMultiplier(ref.animationSpeed);

    m_currentAction = action;
    m_currentActionRef = ref;
    m_currentActionId = action.id;

    prepareDisplayLabelForPixmap();
    clearStatusMessage();

    m_player->play(ref.loop, ref.repeat);

    if (ref.moveEnabled && ref.movementSpeed > 0) {
        m_moveEnabled = true;
        m_moveVelocity = AppSettings::baseMoveSpeed() * ref.movementSpeed;
        m_moveAxis = ref.moveAxis;
        m_moveRemainderX = 0.0;
        m_moveRemainderY = 0.0;
        updateMoveDirection();
        startMovement();
    }

    return true;
}

bool PetWidget::playActionByRef(const PetActionRef &ref)
{
    PetAction action = findActionById(ref.actionId);
    if (!action.isValid()) {
        showStatusMessage(tr("找不到动作"), ref.actionId);
        return false;
    }

    return playAction(action, ref);
}

// 播放空闲动作
//
// 先尝试当前宠物 playlist，再 fallback 到全局动作库。
// 返回 false 表示没有任何动作成功播放。
//
// 重要：idle 作为默认背景动作播放时，使用运行时副本并强制 loop=false。
// 原因：如果 idle 无限 loop，onActionFinished() 永远不会触发，
// 运行时队列中的 random/timed/emotion 动作将永远无法被消费。
// 此修改不改变 playlist.json，只影响运行时播放副本。
bool PetWidget::playIdleAction()
{
    if (!m_petRunning) {
        return false;
    }

    m_currentMode = PetPlayMode::Idle;

    QList<PetActionRef> idleRefs = m_playlist.idleActions();
    if (!idleRefs.isEmpty()) {
        int startIndex = m_idleActionIndex % idleRefs.size();
        int tried = 0;

        while (tried < idleRefs.size()) {
            const PetActionRef &originalRef = idleRefs.at(startIndex);

            // 创建运行时副本，强制 loop=false 保证每轮自然结束
            PetActionRef runtimeRef = originalRef;
            runtimeRef.loop = false;
            if (runtimeRef.repeat <= 0) {
                runtimeRef.repeat = 1;
            }

            if (playActionByRef(runtimeRef)) {
                m_idleActionIndex = (startIndex + 1) % idleRefs.size();
                return true;
            }
            startIndex = (startIndex + 1) % idleRefs.size();
            ++tried;
        }
    }

    if (!m_actions.isEmpty()) {
        for (const PetAction &action : m_actions) {
            if (action.enabled && action.isValid() && action.frameCount > 0 && !action.frameFiles.isEmpty()) {
                PetActionRef defaultRef(action.id);
                defaultRef.loop = false;
                defaultRef.repeat = 1;
                if (playAction(action, defaultRef)) {
                    return true;
                }
            }
        }
    }

    showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
    return false;
}

void PetWidget::playEmotion(const QString &emotion)
{
    if (!m_petRunning) {
        return;
    }

    if (emotion.isEmpty()) {
        return;
    }

    QList<PetActionRef> emotionRefs = m_playlist.emotionActions(emotion);
    if (emotionRefs.isEmpty()) {
        return;
    }

    int index = QRandomGenerator::global()->bounded(emotionRefs.size());
    PetActionRef ref = emotionRefs.at(index);

    enqueueAction(ref, QueuedActionType::Emotion, "emotion " + emotion);
}

PetAction PetWidget::findActionById(const QString &actionId) const
{
    for (const PetAction &action : m_actions) {
        if (action.id == actionId && action.enabled) {
            return action;
        }
    }
    return PetAction();
}

// 优先按 actionId + displayName 匹配当前播放项，找不到再 fallback 到只按 actionId
bool PetWidget::findCurrentActionRefInPlaylist(PetActionRef *outRef) const
{
    if (m_currentActionId.isEmpty()) {
        return false;
    }
    return m_playlist.findMatchingActionRef(m_currentActionId, m_currentActionRef.displayName, outRef);
}

void PetWidget::onFrameChanged(const QPixmap &pixmap)
{
    prepareDisplayLabelForPixmap();
    m_displayLabel->setPixmap(pixmap);
    m_displayLabel->update();
    update();
}

void PetWidget::onActionFinished()
{
    if (!m_petRunning) {
        return;
    }

    playNextRuntimeActionOrIdle();
}

void PetWidget::triggerRandomAction()
{
    if (!m_petRunning) {
        return;
    }

    // 防止 random 堆积：如果队列中已有 random 项则跳过
    for (const QueuedAction &item : m_runtimeQueue) {
        if (item.type == QueuedActionType::Random) {
            return;
        }
    }

    QList<PetActionRef> randomRefs = m_playlist.randomActions();
    if (randomRefs.isEmpty()) {
        return;
    }

    int index = QRandomGenerator::global()->bounded(randomRefs.size());
    PetActionRef ref = randomRefs.at(index);

    enqueueAction(ref, QueuedActionType::Random, "random timer");
}

void PetWidget::checkTimedActions()
{
    if (!m_petRunning) {
        return;
    }

    QList<PetActionRef> timedRefs = m_playlist.timedActions();
    if (timedRefs.isEmpty()) {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    QTime currentTime = now.time();
    QDate today = now.date();

    for (int i = 0; i < timedRefs.size(); ++i) {
        const PetActionRef &ref = timedRefs.at(i);

        if (ref.timedTriggerMode == TimedTriggerMode::ClockTime) {
            QTime targetTime = QTime::fromString(ref.triggerTime, "HH:mm");
            if (!targetTime.isValid()) {
                targetTime = QTime(0, 0);
            }

            if (currentTime.hour() == targetTime.hour() && currentTime.minute() == targetTime.minute()) {
                // key 包含 displayName 以区分同一 actionId 的不同播放项
                QString key = QString("%1|%2|%3").arg(ref.actionId, ref.displayName, ref.triggerTime);

                if (!m_clockTimedLastTriggeredDate.contains(key) || m_clockTimedLastTriggeredDate[key] != today) {
                    m_clockTimedLastTriggeredDate[key] = today;
                    enqueueAction(ref, QueuedActionType::TimedClock, "timed clock " + ref.triggerTime);
                    return;
                }
            }
        } else {
            if (ref.intervalSeconds <= 0) {
                continue;
            }

            if (!m_lastTimedTriggerTimes.contains(i)) {
                m_lastTimedTriggerTimes[i] = now;
                continue;
            }

            QDateTime lastTrigger = m_lastTimedTriggerTimes[i];
            qint64 elapsedSeconds = lastTrigger.secsTo(now);

            if (elapsedSeconds >= ref.intervalSeconds) {
                m_lastTimedTriggerTimes[i] = now;

                // 去重：队列中已有相同 type + actionId + displayName 的 TimedInterval 则跳过
                bool alreadyQueued = false;
                for (const QueuedAction &item : m_runtimeQueue) {
                    if (item.type == QueuedActionType::TimedInterval
                        && item.ref.actionId == ref.actionId
                        && item.ref.displayName == ref.displayName) {
                        alreadyQueued = true;
                        break;
                    }
                }

                if (!alreadyQueued) {
                    enqueueAction(ref, QueuedActionType::TimedInterval, "timed interval");
                }
                return;
            }
        }
    }
}

void PetWidget::onErrorOccurred(const QString &message)
{
    showStatusMessage(tr("播放错误"), message);
}

// 将动作加入运行时队列
// 优先级：Emotion(0) > TimedClock(1) > TimedInterval(2) > Random(3)
void PetWidget::enqueueAction(const PetActionRef &ref, QueuedActionType type, const QString &reason)
{
    QueuedAction item;
    item.ref = ref;
    item.type = type;
    item.reason = reason;
    item.sequence = m_nextSequence++;

    switch (type) {
    case QueuedActionType::Emotion:
        item.priority = 0;
        break;
    case QueuedActionType::TimedClock:
        item.priority = 1;
        break;
    case QueuedActionType::TimedInterval:
        item.priority = 2;
        break;
    case QueuedActionType::Random:
        item.priority = 3;
        break;
    }

    // 队列长度限制：避免极端情况下队列无限增长
    if (m_runtimeQueue.size() >= MaxRuntimeQueueSize) {
        if (item.priority == 0) {
            // Emotion 优先级最高，驱逐队尾最低优先级项腾出空间
            m_runtimeQueue.removeLast();
        } else {
            qWarning() << "Runtime queue full (" << MaxRuntimeQueueSize
                       << "), dropping:" << ref.actionId << "type:" << static_cast<int>(type);
            return;
        }
    }

    // 按优先级插入，同优先级追加到末尾（FIFO）
    int insertPos = m_runtimeQueue.size();
    for (int i = 0; i < m_runtimeQueue.size(); ++i) {
        if (m_runtimeQueue[i].priority > item.priority) {
            insertPos = i;
            break;
        }
    }
    m_runtimeQueue.insert(insertPos, item);

    qDebug() << "Enqueued action:" << ref.actionId << "type:" << static_cast<int>(type)
             << "priority:" << item.priority << "reason:" << reason
             << "queue size:" << m_runtimeQueue.size();
}

// 从运行时队列取出最高优先级的动作并播放
// 跳过加载失败的项，返回 true 表示成功播放
bool PetWidget::playNextFromQueue()
{
    while (!m_runtimeQueue.isEmpty()) {
        QueuedAction item = m_runtimeQueue.takeFirst();

        if (playActionByRef(item.ref)) {
            switch (item.type) {
            case QueuedActionType::Emotion:
                m_currentMode = PetPlayMode::Emotion;
                break;
            case QueuedActionType::TimedClock:
            case QueuedActionType::TimedInterval:
                m_currentMode = PetPlayMode::Timed;
                break;
            case QueuedActionType::Random:
                m_currentMode = PetPlayMode::Random;
                break;
            }
            return true;
        }

        // 播放失败，跳过该项继续尝试下一个
        qWarning() << "Failed to play queued action:" << item.ref.actionId
                    << "reason:" << item.reason << ", skipping";
    }

    return false;
}

// 统一的调度入口：先尝试队列，队列为空则播放 idle
void PetWidget::playNextRuntimeActionOrIdle()
{
    if (!m_petRunning) {
        return;
    }

    // 先尝试从队列中取动作
    if (playNextFromQueue()) {
        return;
    }

    // 队列为空，播放 idle
    if (playIdleAction()) {
        return;
    }

    // idle 也播放失败，进入明确的停止状态，避免 UI 假运行中
    m_petRunning = false;
    m_randomTimer->stop();
    m_timedCheckTimer->stop();
    stopMovement();
    m_player->stop();
    showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
    emit petPaused();
}
