#include "petwidget.h"

#include "core/appsettings.h"
#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "services/petlibraryindexservice.h"
#include "theme/thememanager.h"
#include "widgets/petchatwidget.h"

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QPaintEvent>
#include <QPainter>
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
