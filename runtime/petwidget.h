#ifndef PETWIDGET_H
#define PETWIDGET_H

#include <QDateTime>
#include <QElapsedTimer>
#include <QHash>
#include <QLabel>
#include <QMap>
#include <QPoint>
#include <QWidget>

#include "core/petconfigmanager.h"
#include "models/petaction.h"
#include "models/petplaylist.h"
#include "runtime/petanimationplayer.h"

class QContextMenuEvent;
class QTimer;
class PetChatWidget;

enum class PetPlayMode
{
    Idle,
    Random,
    Timed,
    Emotion
};

// 运行时队列动作类型
enum class QueuedActionType
{
    Random,
    TimedInterval,
    TimedClock,
    Emotion
};

// 运行时队列动作项
struct QueuedAction
{
    PetActionRef ref;
    QueuedActionType type;
    int priority;       // 数值越小优先级越高
    qint64 sequence;    // 同优先级按 FIFO
    QString reason;     // 触发原因描述

    QueuedAction() : priority(0), sequence(0) {}
};

// 宠物加载状态枚举
enum class PetLoadStatus
{
    Ok,                  // 加载成功
    NoPet,               // 没有找到宠物
    PetConfigError,      // 宠物配置错误
    NoActionLibrary,     // 没有动作库
    NoPlaylistAction     // 没有播放列表动作
};

// 宠物主窗口类，负责宠物的显示、动画播放和交互
// 
// 职责：
// - 管理宠物运行状态机（m_petRunning），确保 UI 显示与实际状态一致
// - 加载和播放动作动画，处理播放完成后的状态转换
// - 处理鼠标拖拽、右键菜单等交互
// - 管理聊天窗口的显示和隐藏
// - 响应配置变更，刷新动作库和播放列表
//
// 状态机说明：
// - m_petRunning 为 true 表示宠物正在运行
// - m_currentActionId 为空表示当前没有播放任何动作
// - startPet() 只有在动作播放成功后才设置 m_petRunning = true
// - playIdleAction() 返回 bool，让调用者判断播放是否成功
class PetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PetWidget(QWidget *parent = nullptr);
    ~PetWidget();

    // 加载指定目录的宠物配置
    bool loadPet(const QString &petDirPath);

    // 播放空闲动作，返回是否成功播放
    // 按顺序尝试 playlist 中的空闲动作，失败则尝试全局动作库
    bool playIdleAction();

public slots:

    // 启动宠物
    // 只有在动作播放成功后才进入 running 状态，避免假运行中
    // 播放失败会发出 petStartFailed 信号
    void startPet();

    void pausePet();

    // 完整重载宠物配置和资源
    // 清空所有旧状态和帧缓存，重新加载宠物配置和动作资源
    // 用于配置变更后的完全刷新
    void reloadPet();

    void reloadPlaylistPreservePlayback();

    // 重新加载动作库和播放列表，同时保持当前播放状态
    // 保存并应用后调用，确保 player 使用最新配置
    // 与 reloadPet 的区别：尽量保持当前播放状态，只更新配置变更部分
    void reloadActionsAndPlaylistPreservePlayback();

    void setPetScaleFactor(double scale);
    void setPetOpacity(double opacity);
    void playEmotion(const QString &emotion);
    void setBaseMoveSpeed(int speed);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void hideEvent(QHideEvent *event) override;

signals:
    void openSettingsRequested();
    void hidePetRequested();
    void quitRequested();

    // 宠物成功启动时发出，仅在动作成功播放后发出
    void petStarted();

    // 宠物启动失败时发出，参数为失败原因
    void petStartFailed(const QString &message);

    void petPaused();

private slots:
    void onFrameChanged(const QPixmap &pixmap);
    void onActionFinished();
    void onErrorOccurred(const QString &message);
    void triggerRandomAction();
    void checkTimedActions();
    void updateMovement();

private:
    void setupUi();
    QSize currentDisplaySize() const;
    PetAction findActionById(const QString &actionId) const;
    void loadGlobalActionLibrary();
    bool playAction(const PetAction &action, const PetActionRef &ref);
    bool playActionByRef(const PetActionRef &ref);
    bool hasAnyActionResources() const;
    bool hasAnyUsableEnabledAction() const;
    bool hasAnyPlaylistAction() const;
    QString findFirstEnabledPetId() const;
    bool hasAnyEnabledPet() const;
    void showStatusMessage(const QString &title, const QString &subtitle);
    void clearStatusMessage();

    // 准备显示标签以显示图像
    // 清除文本、样式等，确保标签处于可显示 QPixmap 的状态
    // 必须在设置 pixmap 之前调用，避免文本和图像混合显示
    void prepareDisplayLabelForPixmap();

    void startMovement();
    void stopMovement();
    void updateMoveDirection();
    QRect getAvailableScreenGeometry() const;

    void toggleChatWidget();
    void showChatWidget();
    void hideChatWidget();
    void updateChatWidgetGeometry();
    bool isAutoMoving() const;
    void pauseAutoMovementForChat();
    void resumeAutoMovementAfterChat();

    QLabel *m_displayLabel;
    PetAnimationPlayer *m_player;
    QTimer *m_randomTimer;
    QTimer *m_timedCheckTimer;
    QTimer *m_moveTimer;
    QElapsedTimer m_moveElapsedTimer;
    QMap<int, QDateTime> m_lastTimedTriggerTimes;
    QHash<QString, QDate> m_clockTimedLastTriggeredDate;

    PetBasicInfo m_petInfo;

    // 全局动作库缓存，从 actionlibrary.json 加载
    QList<PetAction> m_actions;

    // 当前宠物的播放列表，从 playlist.json 加载
    // 注意：playlist 是播放项列表，不是 actionId 集合
    // 同一个 actionId 可以多次出现，用不同参数表达不同播放项
    PetPlaylist m_playlist;

    PetAction m_currentAction;
    PetActionRef m_currentActionRef;

    // 当前播放的动作 ID，为空表示无动作播放
    QString m_currentActionId;

    PetPlayMode m_currentMode;

    // 宠物运行状态标志，true 表示正在运行
    // 只有在动作播放成功后才设置为 true
    bool m_petRunning;

    double m_petScaleFactor;
    int m_idleActionIndex;

    bool m_dragging;
    QPoint m_dragPosition;

    bool m_mousePressing;
    QPoint m_mousePressGlobalPos;

    // 鼠标拖拽检测标志，用于区分点击和拖拽
    bool m_mouseDragDetected;

    bool m_moveEnabled;
    int m_moveDirection;
    qreal m_moveVelocity;
    MoveAxis m_moveAxis;
    qreal m_moveRemainderX;
    qreal m_moveRemainderY;

    PetChatWidget *m_chatWidget;

    // 聊天窗口可见性标志
    bool m_chatVisible;

    // 聊天时自动移动暂停标志
    // 聊天窗口显示时暂停自动移动，隐藏后恢复
    bool m_autoMovementPausedByChat;

    // 运行时动作队列，存储待播放的 random/timed/emotion 动作
    // 不写入 playlist.json，只存在于内存中
    QList<QueuedAction> m_runtimeQueue;
    qint64 m_nextSequence;

    // 将动作加入运行时队列
    void enqueueAction(const PetActionRef &ref, QueuedActionType type, const QString &reason);

    // 从运行时队列取出最高优先级的动作并播放
    // 返回 true 表示成功播放了队列中的动作
    bool playNextFromQueue();

    // 统一的调度入口：先尝试队列，队列为空则播放 idle
    void playNextRuntimeActionOrIdle();
};

#endif // PETWIDGET_H
