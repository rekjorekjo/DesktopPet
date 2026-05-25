#include "petwidget.h"

#include "core/appsettings.h"
#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "services/actionlibraryindexservice.h"
#include "services/apiprofileservice.h"
#include "services/petlibraryindexservice.h"
#include "theme/thememanager.h"
#include "widgets/petchatwidget.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QDir>
#include <QEvent>
#include <QFile>
#include <QGuiApplication>
#include <QHideEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QMoveEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QRandomGenerator>
#include <QScreen>
#include <QTimer>
#include <QVBoxLayout>

PetWidget::PetWidget(QWidget *parent)
    : QWidget(parent)
    , m_displayLabel(nullptr)
    , m_player(nullptr)
    , m_randomTimer(nullptr)
    , m_timedCheckTimer(nullptr)
    , m_moveTimer(nullptr)
    , m_currentMode(PetPlayMode::Idle)
    , m_petRunning(AppSettings::autoPlayOnLaunch())
    , m_petScaleFactor(1.0)
    , m_idleActionIndex(0)
    , m_dragging(false)
    , m_mousePressing(false)
    , m_mouseDragDetected(false)
    , m_moveEnabled(false)
    , m_moveDirection(0)
    , m_moveVelocity(0.0)
    , m_moveAxis(MoveAxis::Random)
    , m_moveRemainderX(0.0)
    , m_moveRemainderY(0.0)
    , m_chatWidget(nullptr)
    , m_chatVisible(false)
    , m_autoMovementPausedByChat(false)
    , m_clampingPetPosition(false)
    , m_nextSequence(0)
{
    setupUi();

    m_player = new PetAnimationPlayer(this);
    connect(m_player, &PetAnimationPlayer::frameChanged, this, &PetWidget::onFrameChanged);
    connect(m_player, &PetAnimationPlayer::finished, this, &PetWidget::onActionFinished);
    connect(m_player, &PetAnimationPlayer::errorOccurred, this, &PetWidget::onErrorOccurred);

    m_randomTimer = new QTimer(this);
    connect(m_randomTimer, &QTimer::timeout, this, &PetWidget::triggerRandomAction);

    m_timedCheckTimer = new QTimer(this);
    connect(m_timedCheckTimer, &QTimer::timeout, this, &PetWidget::checkTimedActions);

    m_moveTimer = new QTimer(this);
    connect(m_moveTimer, &QTimer::timeout, this, &PetWidget::updateMovement);

    m_chatWidget = new PetChatWidget();
    m_chatWidget->installEventFilter(this);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, [this]() {
        if (m_chatWidget) {
            m_chatWidget->applyTheme();
        }
    });

    setWindowOpacity(AppSettings::petOpacity());

    loadPet(PetPaths::currentPetDirectory());
}

PetWidget::~PetWidget()
{
    if (m_chatWidget) {
        m_chatWidget->hide();
        m_chatWidget->deleteLater();
        m_chatWidget = nullptr;
    }
}

void PetWidget::setupUi()
{
    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint | Qt::Tool);
    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_ShowWithoutActivating);

    setFixedSize(200, 200);
    move(200, 200);

    m_displayLabel = new QLabel(this);
    m_displayLabel->setObjectName("petDisplayLabel");
    m_displayLabel->setAlignment(Qt::AlignCenter);
    m_displayLabel->setScaledContents(false);
    m_displayLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(m_displayLabel, 0, Qt::AlignCenter);

    setLayout(layout);
}

void PetWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.fillRect(rect(), QColor(0, 0, 0, 1));
}

QSize PetWidget::currentDisplaySize() const
{
    QSize baseSize = m_petInfo.displaySize;
    if (!baseSize.isValid() || baseSize.isEmpty()) {
        baseSize = QSize(200, 200);
    }

    return QSize(
        qRound(baseSize.width() * m_petScaleFactor),
        qRound(baseSize.height() * m_petScaleFactor)
    );
}

bool PetWidget::hasAnyActionResources() const
{
    if (m_actions.isEmpty()) {
        return false;
    }

    for (const PetAction &action : m_actions) {
        if (action.enabled && action.frameCount > 0 && !action.frameFiles.isEmpty()) {
            return true;
        }
    }

    return false;
}

bool PetWidget::hasAnyUsableEnabledAction() const
{
    for (const PetAction &action : m_actions) {
        if (action.enabled && action.frameCount > 0 && !action.frameFiles.isEmpty()) {
            return true;
        }
    }
    return false;
}

bool PetWidget::hasAnyPlaylistAction() const
{
    if (!m_playlist.idleActions().isEmpty()) {
        return true;
    }
    if (!m_playlist.randomActions().isEmpty()) {
        return true;
    }
    if (!m_playlist.timedActions().isEmpty()) {
        return true;
    }
    return false;
}

QString PetWidget::findFirstEnabledPetId() const
{
    return PetLibraryIndexService::findFirstEnabledPetId();
}

bool PetWidget::hasAnyEnabledPet() const
{
    return !findFirstEnabledPetId().isEmpty();
}

void PetWidget::showStatusMessage(const QString &title, const QString &subtitle)
{
    QString html;
    if (subtitle.isEmpty()) {
        html = QString(
            "<div style='text-align: center; padding: 20px;'>"
            "<span style='font-size: 16px; font-weight: bold; color: #374151;'>%1</span>"
            "</div>"
        ).arg(title);
    } else {
        html = QString(
            "<div style='text-align: center; padding: 20px;'>"
            "<span style='font-size: 16px; font-weight: bold; color: #374151;'>%1</span><br/><br/>"
            "<span style='font-size: 13px; color: #6B7280;'>%2</span>"
            "</div>"
        ).arg(title, subtitle);
    }

    m_displayLabel->setText(html);
    m_displayLabel->setTextFormat(Qt::RichText);
    m_displayLabel->setAlignment(Qt::AlignCenter);
    m_displayLabel->setWordWrap(true);
    m_displayLabel->setStyleSheet(
        "QLabel {"
        "  background: transparent;"
        "  border: none;"
        "  padding: 0px;"
        "  color: #4B5563;"
        "}"
    );
}

void PetWidget::clearStatusMessage()
{
    m_displayLabel->clear();
    m_displayLabel->setStyleSheet("");
}

// 准备显示标签以显示图像
//
// 状态提示使用 QLabel 文本模式。
// 播放动画前必须切回 pixmap 模式，否则可能显示残留文字或空白。
void PetWidget::prepareDisplayLabelForPixmap()
{
    m_displayLabel->clear();
    m_displayLabel->setText(QString());
    m_displayLabel->setPixmap(QPixmap());
    m_displayLabel->setTextFormat(Qt::AutoText);
    m_displayLabel->setWordWrap(false);
    m_displayLabel->setAlignment(Qt::AlignCenter);
    m_displayLabel->setStyleSheet(QString());
    m_displayLabel->setVisible(true);
    m_displayLabel->raise();
}

bool PetWidget::loadPet(const QString &petDirPath)
{
    loadGlobalActionLibrary();

    PetLibraryIndexService::ensureValidCurrentPetId();

    QString currentPetId = AppSettings::currentPetId();
    QString currentPetDir = PetPaths::petDirectory(currentPetId);

    QDir petDir(currentPetDir);
    QString petJsonPath = currentPetDir + "/pet.json";
    QString playlistPath = currentPetDir + "/playlist.json";

    if (currentPetId.isEmpty()) {
        QString firstEnabledPet = findFirstEnabledPetId();
        if (!firstEnabledPet.isEmpty()) {
            AppSettings::setCurrentPetId(firstEnabledPet);
            currentPetDir = PetPaths::petDirectory(firstEnabledPet);
            petJsonPath = currentPetDir + "/pet.json";
            playlistPath = currentPetDir + "/playlist.json";
            petDir = QDir(currentPetDir);
        } else {
            m_petRunning = false;
            hideChatWidget();
            showStatusMessage(tr("尚未创建宠物"), tr("请在宠物管理中新建宠物"));
            return false;
        }
    } else if (!petDir.exists() || !QFile::exists(petJsonPath) || !QFile::exists(playlistPath)) {
        m_petRunning = false;
        hideChatWidget();
        showStatusMessage(tr("当前宠物配置缺失"), tr("请在宠物管理中重新创建配置"));
        return false;
    }

    bool petLoaded = PetConfigManager::loadPetInfoJson(petJsonPath, m_petInfo);
    bool playlistLoaded = PetConfigManager::loadPlaylistFromJson(playlistPath, m_playlist);

    if (!petLoaded) {
        m_petRunning = false;
        hideChatWidget();
        showStatusMessage(tr("宠物配置加载失败"), tr("请前往宠物管理检查配置"));
        return false;
    }

    if (!playlistLoaded) {
        m_playlist = PetPlaylist();
    }

    if (!hasAnyActionResources()) {
        m_petRunning = false;
        hideChatWidget();
        showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
        return false;
    }

    if (!hasAnyUsableEnabledAction()) {
        m_petRunning = false;
        hideChatWidget();
        showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
        return false;
    }

    if (!hasAnyPlaylistAction()) {
        m_petRunning = false;
        hideChatWidget();
        showStatusMessage(tr("当前宠物暂无动作配置"), tr("请前往动作设置添加动作"));
        return false;
    }

    QSize displaySize = currentDisplaySize();
    setFixedSize(displaySize);
    m_displayLabel->setFixedSize(displaySize);
    clampPetToScreen();
    keepPetAboveChat();

    clearStatusMessage();

    if (m_petRunning) {
        playIdleAction();
        m_randomTimer->start(30000);
        m_timedCheckTimer->start(1000);
    } else {
        showStatusMessage(tr("已暂停"), QString());
    }

    return true;
}

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

// 启动宠物
//
// 不能在动作真正播放成功前进入 running。
// playIdleAction 返回 bool，是为了避免假运行中 / 透明空白。
// PetManagePage 状态通过 petStarted / petStartFailed 回传更新。
void PetWidget::startPet()
{
    if (m_petRunning) {
        return;
    }

    if (m_currentActionId.isEmpty()) {
        reloadPet();
    }

    if (m_actions.isEmpty() || !hasAnyUsableEnabledAction()) {
        loadGlobalActionLibrary();

        QString currentPetId = AppSettings::currentPetId();
        QString currentPetDir = PetPaths::petDirectory(currentPetId);
        QString playlistPath = currentPetDir + "/playlist.json";
        PetConfigManager::loadPlaylistFromJson(playlistPath, m_playlist);
    }

    if (m_actions.isEmpty() || !hasAnyUsableEnabledAction()) {
        showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
        emit petStartFailed(tr("暂无可用动作"));
        return;
    }

    if (!hasAnyPlaylistAction()) {
        showStatusMessage(tr("当前宠物暂无动作配置"), tr("请前往动作设置添加动作"));
        emit petStartFailed(tr("当前宠物暂无动作配置"));
        return;
    }

    m_petRunning = true;

    if (!m_currentActionId.isEmpty() && m_player->isPaused()) {
        m_player->resume();
        if (m_moveEnabled) {
            startMovement();
        }
        m_randomTimer->start(30000);
        m_timedCheckTimer->start(1000);
        emit petStarted();
    } else {
        bool started = playIdleAction();
        if (!started) {
            m_petRunning = false;
            m_randomTimer->stop();
            m_timedCheckTimer->stop();
            stopMovement();
            emit petStartFailed(tr("暂无可用动作"));
            return;
        }
        m_randomTimer->start(30000);
        m_timedCheckTimer->start(1000);
        emit petStarted();
    }
}

void PetWidget::pausePet()
{
    if (!m_petRunning) {
        return;
    }

    m_petRunning = false;
    m_randomTimer->stop();
    m_timedCheckTimer->stop();
    m_moveTimer->stop();
    m_player->pause();
    emit petPaused();
}

void PetWidget::reloadPet()
{
    m_randomTimer->stop();
    m_timedCheckTimer->stop();
    m_moveTimer->stop();
    m_player->clear();

    m_currentAction = PetAction();
    m_currentActionRef = PetActionRef();
    m_currentActionId.clear();
    m_currentMode = PetPlayMode::Idle;
    m_lastTimedTriggerTimes.clear();
    m_clockTimedLastTriggeredDate.clear();
    m_idleActionIndex = 0;
    m_moveEnabled = false;
    m_moveAxis = MoveAxis::Random;
    m_moveRemainderX = 0.0;
    m_moveRemainderY = 0.0;
    m_runtimeQueue.clear();
    m_nextSequence = 0;

    loadPet(PetPaths::currentPetDirectory());

    if (m_chatWidget) {
        m_chatWidget->setPetDisplayName(m_petInfo.name);
    }
}

void PetWidget::reloadPlaylistPreservePlayback()
{
    QString currentPetId = AppSettings::currentPetId();
    QString currentPetDir = PetPaths::petDirectory(currentPetId);
    QString playlistPath = currentPetDir + "/playlist.json";

    PetPlaylist newPlaylist;
    bool loaded = PetConfigManager::loadPlaylistFromJson(playlistPath, newPlaylist);
    if (!loaded) {
        qWarning() << "reloadPlaylistPreservePlayback: Failed to load playlist.json";
        return;
    }

    m_playlist = newPlaylist;
    m_runtimeQueue.clear();
    m_nextSequence = 0;

    if (!m_currentActionId.isEmpty()) {
        PetActionRef newRef;
        if (findCurrentActionRefInPlaylist(&newRef)) {
            PetActionRef oldRef = m_currentActionRef;

            // Idle 模式下使用运行时副本规则，避免 idle loop=true 导致队列无法消费
            if (m_currentMode == PetPlayMode::Idle && m_petRunning) {
                PetActionRef runtimeRef = newRef;
                runtimeRef.loop = false;
                if (runtimeRef.repeat <= 0) {
                    runtimeRef.repeat = 1;
                }
                m_currentActionRef = runtimeRef;

                if (!qFuzzyCompare(oldRef.animationSpeed, runtimeRef.animationSpeed)) {
                    m_player->setSpeedMultiplier(runtimeRef.animationSpeed);
                }
                m_player->updatePlaybackOptions(runtimeRef.loop, runtimeRef.repeat);
            } else {
                m_currentActionRef = newRef;

                if (!qFuzzyCompare(oldRef.animationSpeed, newRef.animationSpeed)) {
                    m_player->setSpeedMultiplier(newRef.animationSpeed);
                }
                if (oldRef.loop != newRef.loop || oldRef.repeat != newRef.repeat) {
                    m_player->updatePlaybackOptions(newRef.loop, newRef.repeat);
                }
            }

            // 恢复移动状态（使用 m_currentActionRef 中已确定的参数）
            const PetActionRef &activeRef = m_currentActionRef;
            if (activeRef.moveEnabled && activeRef.movementSpeed > 0) {
                m_moveVelocity = AppSettings::baseMoveSpeed() * activeRef.movementSpeed;
                m_moveAxis = activeRef.moveAxis;

                if (oldRef.moveAxis != activeRef.moveAxis) {
                    updateMoveDirection();
                }

                if (!m_moveEnabled && m_petRunning) {
                    m_moveEnabled = true;
                    m_moveRemainderX = 0.0;
                    m_moveRemainderY = 0.0;
                    startMovement();
                }
            } else {
                stopMovement();
            }
        } else {
            stopMovement();
            m_player->stop();
            m_currentAction = PetAction();
            m_currentActionRef = PetActionRef();
            m_currentActionId.clear();

            if (m_petRunning) {
                playNextRuntimeActionOrIdle();
            }
        }
    }
}

// 重新加载动作库和播放列表，同时保持当前播放状态
//
// 保存并应用后必须同时刷新 actionlibrary 和 playlist。
// 只刷新 playlist 会导致 player 看不到新导入的动作。
// 从空资源状态恢复时可能需要完整 reloadPet。
void PetWidget::reloadActionsAndPlaylistPreservePlayback()
{
    bool recoveryFromEmptyState = m_currentActionId.isEmpty() && !m_petRunning;

    loadGlobalActionLibrary();

    QString currentPetId = AppSettings::currentPetId();
    QString currentPetDir = PetPaths::petDirectory(currentPetId);
    QString playlistPath = currentPetDir + "/playlist.json";

    PetPlaylist newPlaylist;
    bool loaded = PetConfigManager::loadPlaylistFromJson(playlistPath, newPlaylist);
    if (!loaded) {
        qWarning() << "reloadActionsAndPlaylistPreservePlayback: Failed to load playlist.json";
    } else {
        m_playlist = newPlaylist;
    }

    m_runtimeQueue.clear();
    m_nextSequence = 0;

    if (recoveryFromEmptyState && hasAnyUsableEnabledAction() && hasAnyPlaylistAction()) {
        reloadPet();
        return;
    }

    if (!m_actions.isEmpty() && !m_currentActionId.isEmpty()) {
        PetAction newAction = findActionById(m_currentActionId);
        PetActionRef newRef;

        // 记录 reload 前的播放状态，loadAction 会重置这些状态
        bool wasRunning = m_petRunning;
        bool wasPaused = m_player->isPaused();

        if (newAction.isValid() && findCurrentActionRefInPlaylist(&newRef)) {
            PetActionRef oldRef = m_currentActionRef;
            m_currentAction = newAction;
            m_currentActionRef = newRef;

            bool loadOk = m_player->loadAction(newAction, currentDisplaySize());

            if (!loadOk) {
                // 当前动作资源加载失败，不能继续假运行
                stopMovement();
                m_player->stop();
                m_currentAction = PetAction();
                m_currentActionRef = PetActionRef();
                m_currentActionId.clear();

                if (wasRunning) {
                    // 尝试从队列或 idle fallback
                    playNextRuntimeActionOrIdle();
                }
            } else {
                // loadAction 成功，恢复播放参数

                if (wasRunning) {
                    if (m_currentMode == PetPlayMode::Idle) {
                        // Idle 模式：用运行时副本规则重启当前 idle 动作，不切换到下一个 idle
                        // 避免 playNextRuntimeActionOrIdle -> playIdleAction 推进 m_idleActionIndex 导致画面闪烁
                        PetActionRef runtimeRef = newRef;
                        runtimeRef.loop = false;
                        if (runtimeRef.repeat <= 0) {
                            runtimeRef.repeat = 1;
                        }

                        m_currentAction = newAction;
                        m_currentActionRef = runtimeRef;

                        m_player->setSpeedMultiplier(runtimeRef.animationSpeed);
                        m_player->play(runtimeRef.loop, runtimeRef.repeat);

                        if (wasPaused) {
                            m_player->pause();
                        }

                        // 恢复移动状态
                        if (runtimeRef.moveEnabled && runtimeRef.movementSpeed > 0) {
                            m_moveVelocity = AppSettings::baseMoveSpeed() * runtimeRef.movementSpeed;
                            m_moveAxis = runtimeRef.moveAxis;

                            if (oldRef.moveAxis != runtimeRef.moveAxis) {
                                updateMoveDirection();
                            }

                            if (!m_moveEnabled) {
                                m_moveEnabled = true;
                                m_moveRemainderX = 0.0;
                                m_moveRemainderY = 0.0;
                                startMovement();
                            }
                        } else {
                            stopMovement();
                        }

                        return;
                    } else {
                        // 非 Idle 模式（Random/Timed/Emotion）：用用户配置的 loop/repeat 恢复播放
                        m_player->setSpeedMultiplier(newRef.animationSpeed);
                        m_player->play(newRef.loop, newRef.repeat);

                        if (wasPaused) {
                            m_player->pause();
                        }
                    }
                } else {
                    // 非运行状态，只更新参数不播放
                    if (!qFuzzyCompare(oldRef.animationSpeed, newRef.animationSpeed)) {
                        m_player->setSpeedMultiplier(newRef.animationSpeed);
                    }
                }

                // 恢复移动状态（仅非 Idle 模式会走到这里）
                if (newRef.moveEnabled && newRef.movementSpeed > 0) {
                    m_moveVelocity = AppSettings::baseMoveSpeed() * newRef.movementSpeed;
                    m_moveAxis = newRef.moveAxis;

                    if (oldRef.moveAxis != newRef.moveAxis) {
                        updateMoveDirection();
                    }

                    if (!m_moveEnabled && wasRunning) {
                        m_moveEnabled = true;
                        m_moveRemainderX = 0.0;
                        m_moveRemainderY = 0.0;
                        startMovement();
                    }
                } else {
                    stopMovement();
                }
            }
        } else {
            // 当前动作在新 playlist 中不存在或动作库中已删除
            stopMovement();
            m_player->stop();
            m_currentAction = PetAction();
            m_currentActionRef = PetActionRef();
            m_currentActionId.clear();

            if (wasRunning) {
                playNextRuntimeActionOrIdle();
            }
        }
    } else if (m_actions.isEmpty()) {
        m_petRunning = false;
        m_randomTimer->stop();
        m_timedCheckTimer->stop();
        stopMovement();
        m_player->stop();
        m_currentAction = PetAction();
        m_currentActionRef = PetActionRef();
        m_currentActionId.clear();

        showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
        emit petPaused();
    } else if (m_currentActionId.isEmpty() && m_petRunning) {
        if (hasAnyUsableEnabledAction() && hasAnyPlaylistAction()) {
            if (!playIdleAction()) {
                m_petRunning = false;
                m_randomTimer->stop();
                m_timedCheckTimer->stop();
                stopMovement();
                m_player->stop();
                showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
                emit petPaused();
            }
        } else {
            m_petRunning = false;
            m_randomTimer->stop();
            m_timedCheckTimer->stop();
            stopMovement();
            m_player->stop();
            showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
            emit petPaused();
        }
    } else if (m_currentActionId.isEmpty() && !m_petRunning) {
        if (!hasAnyUsableEnabledAction()) {
            stopMovement();
            m_player->stop();
            m_currentAction = PetAction();
            m_currentActionRef = PetActionRef();
            m_currentActionId.clear();
            showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
        } else if (!hasAnyPlaylistAction()) {
            stopMovement();
            m_player->stop();
            m_currentAction = PetAction();
            m_currentActionRef = PetActionRef();
            m_currentActionId.clear();
            showStatusMessage(tr("当前宠物暂无动作配置"), tr("请前往动作设置添加动作"));
        } else {
            stopMovement();
            m_player->stop();
            clearStatusMessage();
            showStatusMessage(tr("已暂停"), QString());
        }
    }
}

void PetWidget::setPetScaleFactor(double scale)
{
    if (scale < 0.5) {
        scale = 0.5;
    } else if (scale > 2.0) {
        scale = 2.0;
    }

    if (qFuzzyCompare(m_petScaleFactor, scale)) {
        return;
    }

    m_petScaleFactor = scale;

    if (m_actions.isEmpty()) {
        return;
    }

    QSize displaySize = currentDisplaySize();
    setFixedSize(displaySize);
    m_displayLabel->setFixedSize(displaySize);
    clampPetToScreen();
    keepPetAboveChat();

    if (m_currentAction.isValid() && m_currentActionRef.isValid()) {
        bool wasRunning = m_petRunning;
        bool wasPaused = m_player->isPaused();

        bool loadOk = m_player->loadAction(m_currentAction, displaySize);

        if (loadOk) {
            if (wasRunning) {
                m_player->setSpeedMultiplier(m_currentActionRef.animationSpeed);
                m_player->play(m_currentActionRef.loop, m_currentActionRef.repeat);
                if (wasPaused) {
                    m_player->pause();
                }
            }
        } else {
            stopMovement();
            m_player->stop();
            m_currentAction = PetAction();
            m_currentActionRef = PetActionRef();
            m_currentActionId.clear();
            if (wasRunning) {
                playNextRuntimeActionOrIdle();
            }
        }
    }
}

void PetWidget::setPetOpacity(double opacity)
{
    if (opacity < 0.2) {
        opacity = 0.2;
    } else if (opacity > 1.0) {
        opacity = 1.0;
    }

    setWindowOpacity(opacity);
}

void PetWidget::setBaseMoveSpeed(int speed)
{
    Q_UNUSED(speed);

    if (m_moveEnabled && m_currentActionRef.moveEnabled && m_currentActionRef.movementSpeed > 0) {
        m_moveVelocity = AppSettings::baseMoveSpeed() * m_currentActionRef.movementSpeed;
        m_moveElapsedTimer.restart();
    }
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

void PetWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_mousePressing = true;
        m_mousePressGlobalPos = event->globalPosition().toPoint();
        m_mouseDragDetected = false;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void PetWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_mousePressing && (event->buttons() & Qt::LeftButton)) {
        QPoint currentPos = event->globalPosition().toPoint();
        int distance = (currentPos - m_mousePressGlobalPos).manhattanLength();

        if (!m_mouseDragDetected && distance > 5) {
            m_mouseDragDetected = true;
            m_dragging = true;
        }

        if (m_dragging) {
            move(clampedPetPosition(currentPos - m_dragPosition));
            event->accept();
            return;
        }
    }
    QWidget::mouseMoveEvent(event);
}

void PetWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (!m_mouseDragDetected && m_mousePressing) {
            toggleChatWidget();
        }
        m_mousePressing = false;
        m_mouseDragDetected = false;
        m_dragging = false;
        event->accept();
        return;
    }
    QWidget::mouseReleaseEvent(event);
}

void PetWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
    menu.setStyleSheet(ThemeManager::instance().menuStyleSheet());

    QAction *startAction = menu.addAction(tr("开始"));
    QAction *pauseAction = menu.addAction(tr("暂停"));
    menu.addSeparator();
    QAction *hideAction = menu.addAction(tr("隐藏桌宠"));
    QAction *settingsAction = menu.addAction(tr("打开设置"));
    menu.addSeparator();
    QAction *quitAction = menu.addAction(tr("退出"));

    QAction *selectedAction = menu.exec(event->globalPos());

    if (selectedAction == startAction) {
        startPet();
    } else if (selectedAction == pauseAction) {
        pausePet();
    } else if (selectedAction == hideAction) {
        emit hidePetRequested();
    } else if (selectedAction == settingsAction) {
        qDebug() << "Open settings requested";
        emit openSettingsRequested();
    } else if (selectedAction == quitAction) {
        qDebug() << "Quit requested";
        emit quitRequested();
    }
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

void PetWidget::startMovement()
{
    if (!m_moveEnabled) {
        return;
    }

    m_moveElapsedTimer.start();
    m_moveTimer->start(16);
}

void PetWidget::stopMovement()
{
    m_moveEnabled = false;
    m_moveTimer->stop();
    m_moveRemainderX = 0.0;
    m_moveRemainderY = 0.0;
}

void PetWidget::updateMoveDirection()
{
    switch (m_moveAxis) {
    case MoveAxis::Random:
        m_moveDirection = QRandomGenerator::global()->bounded(4);
        break;
    case MoveAxis::Horizontal:
        m_moveDirection = QRandomGenerator::global()->bounded(2);
        break;
    case MoveAxis::Vertical:
        m_moveDirection = 2 + QRandomGenerator::global()->bounded(2);
        break;
    }
}

void PetWidget::updateMovement()
{
    if (!m_moveEnabled || m_dragging || m_chatVisible || m_autoMovementPausedByChat) {
        m_moveElapsedTimer.restart();
        return;
    }

    qint64 elapsedMs = m_moveElapsedTimer.elapsed();
    m_moveElapsedTimer.restart();

    qreal elapsedSec = elapsedMs / 1000.0;
    qreal distance = m_moveVelocity * elapsedSec;

    qreal floatDx = 0.0;
    qreal floatDy = 0.0;

    switch (m_moveDirection) {
    case 0:
        floatDx = -distance;
        break;
    case 1:
        floatDx = distance;
        break;
    case 2:
        floatDy = -distance;
        break;
    case 3:
        floatDy = distance;
        break;
    }

    m_moveRemainderX += floatDx;
    m_moveRemainderY += floatDy;

    int dx = 0;
    int dy = 0;

    if (qAbs(m_moveRemainderX) >= 1.0) {
        dx = static_cast<int>(m_moveRemainderX);
        m_moveRemainderX -= dx;
    }

    if (qAbs(m_moveRemainderY) >= 1.0) {
        dy = static_cast<int>(m_moveRemainderY);
        m_moveRemainderY -= dy;
    }

    if (dx == 0 && dy == 0) {
        return;
    }

    QPoint currentPos = pos();
    QPoint newPos = currentPos + QPoint(dx, dy);
    QPoint clampedPos = clampedPetPosition(newPos);

    if (clampedPos.x() != newPos.x()) {
        m_moveRemainderX = 0.0;
        if (newPos.x() < clampedPos.x()) {
            // Hit left edge -> move right
            if (m_moveAxis == MoveAxis::Horizontal || m_moveAxis == MoveAxis::Random)
                m_moveDirection = 1;
        } else {
            // Hit right edge -> move left
            if (m_moveAxis == MoveAxis::Horizontal || m_moveAxis == MoveAxis::Random)
                m_moveDirection = 0;
        }
    }

    if (clampedPos.y() != newPos.y()) {
        m_moveRemainderY = 0.0;
        if (newPos.y() < clampedPos.y()) {
            // Hit top edge -> move down
            if (m_moveAxis == MoveAxis::Vertical || m_moveAxis == MoveAxis::Random)
                m_moveDirection = 3;
        } else {
            // Hit bottom edge -> move up
            if (m_moveAxis == MoveAxis::Vertical || m_moveAxis == MoveAxis::Random)
                m_moveDirection = 2;
        }
    }

    move(clampedPos);
}

QRect PetWidget::getAvailableScreenGeometry() const
{
    return availableScreenGeometryForRect(geometry());
}

QRect PetWidget::availableScreenGeometryForRect(const QRect &rect) const
{
    QScreen *screen = QGuiApplication::screenAt(rect.center());
    if (!screen) {
        int bestArea = 0;
        for (QScreen *s : QGuiApplication::screens()) {
            QRect inter = s->availableGeometry().intersected(rect);
            int area = inter.width() * inter.height();
            if (area > bestArea) {
                bestArea = area;
                screen = s;
            }
        }
    }
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (screen) {
        return screen->availableGeometry();
    }
    return QRect(0, 0, 1920, 1080);
}

QPoint PetWidget::clampedPetPosition(const QPoint &topLeft) const
{
    QRect candidate(topLeft, size());
    QRect screen = availableScreenGeometryForRect(candidate);

    int minX = screen.left();
    int maxX = screen.right() - width() + 1;
    int minY = screen.top();
    int maxY = screen.bottom() - height() + 1;

    if (maxX < minX) maxX = minX;
    if (maxY < minY) maxY = minY;

    return QPoint(qBound(minX, topLeft.x(), maxX),
                  qBound(minY, topLeft.y(), maxY));
}

void PetWidget::clampPetToScreen()
{
    QPoint fixed = clampedPetPosition(pos());
    if (fixed == pos()) return;
    m_clampingPetPosition = true;
    move(fixed);
    m_clampingPetPosition = false;
}

void PetWidget::keepPetAboveChat()
{
    if (m_chatWidget && m_chatVisible && m_chatWidget->isVisible()) {
        raise();
    }
}

void PetWidget::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);

    if (!m_clampingPetPosition) {
        QPoint fixed = clampedPetPosition(pos());
        if (fixed != pos()) {
            m_clampingPetPosition = true;
            move(fixed);
            m_clampingPetPosition = false;
            return;
        }
    }

    if (m_chatWidget && m_chatVisible) {
        updateChatWidgetGeometry();
        keepPetAboveChat();
    }
}

void PetWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    hideChatWidget();
}

bool PetWidget::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == m_chatWidget && m_chatWidget && m_chatVisible) {
        switch (event->type()) {
        case QEvent::Show:
        case QEvent::WindowActivate:
        case QEvent::MouseButtonPress:
        case QEvent::Move:
        case QEvent::Resize:
            QTimer::singleShot(0, this, [this]() {
                keepPetAboveChat();
            });
            break;
        default:
            break;
        }
    }

    return QWidget::eventFilter(watched, event);
}

void PetWidget::toggleChatWidget()
{
    if (m_chatVisible) {
        hideChatWidget();
    } else {
        showChatWidget();
    }
}

void PetWidget::showChatWidget()
{
    if (!m_chatWidget) {
        return;
    }

    if (isAutoMoving()) {
        pauseAutoMovementForChat();
    }

    m_chatWidget->setPetName(m_petInfo.name);
    m_chatWidget->setPetDisplayName(m_petInfo.name);

    ApiConfig cfg;
    ApiProfileService &svc = ApiProfileService::instance();
    if (svc.currentProfile(&cfg)) {
        m_chatWidget->setApiInfo(svc.currentProfileName(), cfg.model);
    } else {
        m_chatWidget->setApiInfo(QString(), QString());
    }

    m_chatWidget->applyTheme();

    updateChatWidgetGeometry();
    m_chatWidget->show();
    m_chatVisible = true;
    m_chatWidget->raise();
    m_chatWidget->activateWindow();
    m_chatWidget->focusInput();
    keepPetAboveChat();
}

void PetWidget::hideChatWidget()
{
    if (!m_chatWidget) {
        return;
    }

    m_chatWidget->hide();
    m_chatVisible = false;

    resumeAutoMovementAfterChat();
}

void PetWidget::updateChatWidgetGeometry()
{
    if (!m_chatWidget) {
        return;
    }

    QSize petSize = currentDisplaySize();
    QRect screenRect = getAvailableScreenGeometry();

    int chatWidth = qMax(260, petSize.width() * 2);
    chatWidth = qMin(chatWidth, screenRect.width() - 20);

    int chatHeight = qMax(520, petSize.width() * 2);
    chatHeight = qMin(chatHeight, screenRect.height() - 20);

    m_chatWidget->setFixedSize(chatWidth, chatHeight);

    const int gap = 4;
    const int margin = 10;

    QPoint petCenter = geometry().center();

    int desiredX = petCenter.x() - chatWidth / 2;
    int minX = screenRect.left() + margin;
    int maxX = screenRect.right() - chatWidth - margin + 1;
    if (maxX < minX) maxX = minX;
    int chatX = qBound(minX, desiredX, maxX);

    int desiredY = geometry().bottom() + gap;
    int minY = screenRect.top() + margin;
    int maxY = screenRect.bottom() - chatHeight - margin + 1;
    if (maxY < minY) maxY = minY;
    int chatY = qBound(minY, desiredY, maxY);

    m_chatWidget->move(chatX, chatY);
    keepPetAboveChat();
}

bool PetWidget::isAutoMoving() const
{
    return m_moveEnabled && m_moveTimer->isActive() && !m_dragging;
}

void PetWidget::pauseAutoMovementForChat()
{
    if (m_moveEnabled && m_moveTimer->isActive()) {
        m_autoMovementPausedByChat = true;
        m_moveTimer->stop();
    }
}

void PetWidget::resumeAutoMovementAfterChat()
{
    if (m_autoMovementPausedByChat && m_moveEnabled && m_petRunning && !m_dragging) {
        m_autoMovementPausedByChat = false;
        m_moveElapsedTimer.restart();
        m_moveTimer->start(16);
    }
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
