#include "petwidget.h"

#include "core/appsettings.h"
#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "services/actionlibraryindexservice.h"
#include "services/petlibraryindexservice.h"
#include "theme/thememanager.h"
#include "widgets/petchatwidget.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QDir>
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
            const PetActionRef &ref = idleRefs.at(startIndex);
            if (playActionByRef(ref)) {
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

    loadPet(PetPaths::currentPetDirectory());
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

    if (!m_currentActionId.isEmpty()) {
        PetActionRef newRef;
        if (m_playlist.findFirstActionRef(m_currentActionId, &newRef)) {
            PetActionRef oldRef = m_currentActionRef;
            m_currentActionRef = newRef;

            if (!qFuzzyCompare(oldRef.animationSpeed, newRef.animationSpeed)) {
                m_player->setSpeedMultiplier(newRef.animationSpeed);
            }

            if (oldRef.loop != newRef.loop || oldRef.repeat != newRef.repeat) {
                m_player->updatePlaybackOptions(newRef.loop, newRef.repeat);
            }

            MoveAxis oldMoveAxis = oldRef.moveAxis;

            if (newRef.moveEnabled && newRef.movementSpeed > 0) {
                m_moveVelocity = AppSettings::baseMoveSpeed() * newRef.movementSpeed;
                m_moveAxis = newRef.moveAxis;

                if (oldMoveAxis != newRef.moveAxis) {
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
                playIdleAction();
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

    if (recoveryFromEmptyState && hasAnyUsableEnabledAction() && hasAnyPlaylistAction()) {
        reloadPet();
        return;
    }

    if (!m_actions.isEmpty() && !m_currentActionId.isEmpty()) {
        PetAction newAction = findActionById(m_currentActionId);
        PetActionRef newRef;

        if (newAction.isValid() && m_playlist.findFirstActionRef(m_currentActionId, &newRef)) {
            PetActionRef oldRef = m_currentActionRef;
            m_currentAction = newAction;
            m_currentActionRef = newRef;

            m_player->loadAction(newAction, currentDisplaySize());

            if (!qFuzzyCompare(oldRef.animationSpeed, newRef.animationSpeed)) {
                m_player->setSpeedMultiplier(newRef.animationSpeed);
            }

            if (oldRef.loop != newRef.loop || oldRef.repeat != newRef.repeat) {
                m_player->updatePlaybackOptions(newRef.loop, newRef.repeat);
            }

            MoveAxis oldMoveAxis = oldRef.moveAxis;

            if (newRef.moveEnabled && newRef.movementSpeed > 0) {
                m_moveVelocity = AppSettings::baseMoveSpeed() * newRef.movementSpeed;
                m_moveAxis = newRef.moveAxis;

                if (oldMoveAxis != newRef.moveAxis) {
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
                if (hasAnyUsableEnabledAction() && hasAnyPlaylistAction()) {
                    playIdleAction();
                } else {
                    showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
                }
            }
        }
    } else if (m_actions.isEmpty()) {
        stopMovement();
        m_player->stop();
        m_currentAction = PetAction();
        m_currentActionRef = PetActionRef();
        m_currentActionId.clear();

        showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
    } else if (m_currentActionId.isEmpty() && m_petRunning) {
        if (hasAnyUsableEnabledAction() && hasAnyPlaylistAction()) {
            playIdleAction();
        } else {
            showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
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

    if (m_currentAction.isValid() && m_currentActionRef.isValid()) {
        m_player->loadAction(m_currentAction, displaySize);
        if (m_player->isPaused()) {
            m_player->resume();
            m_player->pause();
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

    if (playActionByRef(ref)) {
        m_currentMode = PetPlayMode::Emotion;
    }
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
            move(currentPos - m_dragPosition);
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

    playIdleAction();
}

void PetWidget::triggerRandomAction()
{
    if (!m_petRunning || m_currentMode != PetPlayMode::Idle) {
        return;
    }

    QList<PetActionRef> randomRefs = m_playlist.randomActions();
    if (randomRefs.isEmpty()) {
        return;
    }

    int index = QRandomGenerator::global()->bounded(randomRefs.size());
    PetActionRef ref = randomRefs.at(index);

    if (playActionByRef(ref)) {
        m_currentMode = PetPlayMode::Random;
    } else {
        m_currentMode = PetPlayMode::Idle;
    }
}

void PetWidget::checkTimedActions()
{
    if (!m_petRunning || m_currentMode != PetPlayMode::Idle) {
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
                QString key = QString("%1|%2").arg(ref.actionId, ref.triggerTime);

                if (!m_clockTimedLastTriggeredDate.contains(key) || m_clockTimedLastTriggeredDate[key] != today) {
                    if (playActionByRef(ref)) {
                        m_currentMode = PetPlayMode::Timed;
                        m_clockTimedLastTriggeredDate[key] = today;
                        return;
                    }
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
                if (playActionByRef(ref)) {
                    m_currentMode = PetPlayMode::Timed;
                    m_lastTimedTriggerTimes[i] = now;
                    return;
                } else {
                    m_lastTimedTriggerTimes[i] = now;
                }
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

    QRect screenRect = getAvailableScreenGeometry();
    QRect widgetRect = QRect(newPos, size());

    if (widgetRect.left() < screenRect.left()) {
        newPos.setX(screenRect.left());
        m_moveRemainderX = 0.0;
        if (m_moveAxis == MoveAxis::Horizontal) {
            m_moveDirection = 1;
        } else if (m_moveAxis == MoveAxis::Random) {
            m_moveDirection = 1;
        }
    } else if (widgetRect.right() > screenRect.right()) {
        newPos.setX(screenRect.right() - width());
        m_moveRemainderX = 0.0;
        if (m_moveAxis == MoveAxis::Horizontal) {
            m_moveDirection = 0;
        } else if (m_moveAxis == MoveAxis::Random) {
            m_moveDirection = 0;
        }
    }

    if (widgetRect.top() < screenRect.top()) {
        newPos.setY(screenRect.top());
        m_moveRemainderY = 0.0;
        if (m_moveAxis == MoveAxis::Vertical) {
            m_moveDirection = 3;
        } else if (m_moveAxis == MoveAxis::Random) {
            m_moveDirection = 3;
        }
    } else if (widgetRect.bottom() > screenRect.bottom()) {
        newPos.setY(screenRect.bottom() - height());
        m_moveRemainderY = 0.0;
        if (m_moveAxis == MoveAxis::Vertical) {
            m_moveDirection = 2;
        } else if (m_moveAxis == MoveAxis::Random) {
            m_moveDirection = 2;
        }
    }

    move(newPos);
}

QRect PetWidget::getAvailableScreenGeometry() const
{
    QScreen *screen = QGuiApplication::screenAt(pos());
    if (!screen) {
        screen = QGuiApplication::primaryScreen();
    }
    if (screen) {
        return screen->availableGeometry();
    }
    return QRect(0, 0, 1920, 1080);
}

void PetWidget::moveEvent(QMoveEvent *event)
{
    QWidget::moveEvent(event);
    if (m_chatWidget && m_chatVisible) {
        updateChatWidgetGeometry();
    }
}

void PetWidget::hideEvent(QHideEvent *event)
{
    QWidget::hideEvent(event);
    hideChatWidget();
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
    m_chatWidget->setApiConfigName(AppSettings::currentApiConfigName());
    m_chatWidget->applyTheme();

    updateChatWidgetGeometry();
    m_chatWidget->show();
    m_chatVisible = true;
    m_chatWidget->raise();
    m_chatWidget->activateWindow();
    m_chatWidget->focusInput();
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

    int chatHeight = qMax(180, static_cast<int>(petSize.height() * 1.35));
    chatHeight = qMin(chatHeight, screenRect.height() / 3);

    m_chatWidget->setFixedSize(chatWidth, chatHeight);

    QPoint petCenter = geometry().center();
    int chatX = petCenter.x() - chatWidth / 2;
    int chatY = geometry().bottom() + 4;

    if (chatX < screenRect.left()) {
        chatX = screenRect.left() + 10;
    } else if (chatX + chatWidth > screenRect.right()) {
        chatX = screenRect.right() - chatWidth - 10;
    }

    if (chatY + chatHeight > screenRect.bottom()) {
        int spaceBelow = screenRect.bottom() - chatY;
        if (spaceBelow < 100) {
            chatY = geometry().top() - chatHeight - 4;
            if (chatY < screenRect.top()) {
                chatY = screenRect.top() + 10;
            }
        }
    }

    m_chatWidget->move(chatX, chatY);
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
