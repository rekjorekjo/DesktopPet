#include "petwidget.h"

#include "core/appsettings.h"
#include "core/petconfigmanager.h"
#include "core/petpaths.h"
#include "theme/thememanager.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QDir>
#include <QFile>
#include <QGuiApplication>
#include <QMenu>
#include <QMouseEvent>
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
    , m_moveEnabled(false)
    , m_moveDirection(0)
    , m_moveVelocity(0.0)
    , m_moveAxis(MoveAxis::Random)
    , m_moveRemainderX(0.0)
    , m_moveRemainderY(0.0)
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

    setWindowOpacity(AppSettings::petOpacity());

    loadPet(PetPaths::currentPetDirectory());
}

PetWidget::~PetWidget()
{
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
    QDir actionsDir(PetPaths::actionsDirectory());
    QStringList actionDirs = actionsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    return !actionDirs.isEmpty();
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
    QString petsDir = PetPaths::petsDirectory();
    QDir dir(petsDir);
    if (!dir.exists()) {
        return QString();
    }

    QStringList petFolders = dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &petId : petFolders) {
        QString petJsonPath = petsDir + "/" + petId + "/pet.json";
        if (QFile::exists(petJsonPath)) {
            PetBasicInfo info;
            if (PetConfigManager::loadPetInfoJson(petJsonPath, info)) {
                if (info.enabled) {
                    return petId;
                }
            }
        }
    }

    return QString();
}

bool PetWidget::hasAnyEnabledPet() const
{
    return !findFirstEnabledPetId().isEmpty();
}

void PetWidget::showStatusMessage(const QString &title, const QString &subtitle)
{
    QString html;
    if (subtitle.isEmpty()) {
        html = QString("<div style='text-align: center;'><b>%1</b></div>").arg(title);
    } else {
        html = QString("<div style='text-align: center;'><b>%1</b><br/><span style='font-size: 12px; color: #6B7280;'>%2</span></div>")
               .arg(title, subtitle);
    }

    m_displayLabel->setText(html);
    m_displayLabel->setTextFormat(Qt::RichText);
    m_displayLabel->setAlignment(Qt::AlignCenter);
    m_displayLabel->setWordWrap(true);
    m_displayLabel->setStyleSheet(
        "QLabel {"
        "  background-color: rgba(255, 255, 255, 215);"
        "  color: #4B5563;"
        "  border: 1px solid rgba(120, 120, 120, 90);"
        "  border-radius: 12px;"
        "  padding: 14px 18px;"
        "  font-size: 14px;"
        "}"
    );
}

void PetWidget::clearStatusMessage()
{
    m_displayLabel->clear();
    m_displayLabel->setStyleSheet("");
}

bool PetWidget::loadPet(const QString &petDirPath)
{
    loadGlobalActionLibrary();

    QString currentPetId = AppSettings::currentPetId();
    QString currentPetDir = PetPaths::petDirectory(currentPetId);

    QDir petDir(currentPetDir);
    QString petJsonPath = currentPetDir + "/pet.json";

    if (!petDir.exists() || !QFile::exists(petJsonPath)) {
        QString firstEnabledPet = findFirstEnabledPetId();
        if (!firstEnabledPet.isEmpty()) {
            AppSettings::setCurrentPetId(firstEnabledPet);
            currentPetDir = PetPaths::petDirectory(firstEnabledPet);
            petJsonPath = currentPetDir + "/pet.json";
        } else {
            m_petRunning = false;
            showStatusMessage(tr("尚未创建宠物"), tr("请前往设置 > 宠物管理新建宠物"));
            return false;
        }
    }

    bool petLoaded = PetConfigManager::loadPetInfoJson(petJsonPath, m_petInfo);
    QString playlistPath = currentPetDir + "/playlist.json";
    bool playlistLoaded = PetConfigManager::loadPlaylistFromJson(playlistPath, m_playlist);

    if (!petLoaded) {
        m_petRunning = false;
        showStatusMessage(tr("宠物配置加载失败"), tr("请前往宠物管理检查配置"));
        return false;
    }

    if (!playlistLoaded) {
        m_playlist = PetPlaylist();
    }

    if (!hasAnyActionResources()) {
        m_petRunning = false;
        showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
        return false;
    }

    if (!hasAnyUsableEnabledAction()) {
        m_petRunning = false;
        showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
        return false;
    }

    if (!hasAnyPlaylistAction()) {
        m_petRunning = false;
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

    const QString actionsDirPath = PetPaths::actionsDirectory();
    QDir actionsDir(actionsDirPath);
    if (!actionsDir.exists()) {
        return;
    }

    const QStringList actionFolders = actionsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);

    for (const QString &actionId : actionFolders) {
        const QString actionDirPath = actionsDir.filePath(actionId);
        PetAction action = PetConfigManager::loadGlobalActionFromDirectory(actionId, actionDirPath);

        if (!action.isValid()) {
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

    if (!m_player->loadAction(action, currentDisplaySize())) {
        showStatusMessage(tr("动作加载失败"), QString());
        return false;
    }

    m_player->setSpeedMultiplier(ref.animationSpeed);

    m_currentAction = action;
    m_currentActionRef = ref;
    m_currentActionId = action.id;

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

void PetWidget::playIdleAction()
{
    if (!m_petRunning) {
        return;
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
                return;
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
                    return;
                }
            }
        }
    }

    showStatusMessage(tr("暂无可用动作"), tr("请前往设置 > 动作设置新增动作"));
}

void PetWidget::startPet()
{
    if (m_petRunning) {
        return;
    }

    m_petRunning = true;

    if (m_player->isPaused()) {
        m_player->resume();
        if (m_moveEnabled) {
            startMovement();
        }
    } else {
        playIdleAction();
    }

    m_randomTimer->start(30000);
    m_timedCheckTimer->start(1000);
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
}

void PetWidget::reloadPet()
{
    m_randomTimer->stop();
    m_timedCheckTimer->stop();
    m_moveTimer->stop();
    m_player->stop();

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
        m_dragging = true;
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
        return;
    }
    QWidget::mousePressEvent(event);
}

void PetWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
        return;
    }
    QWidget::mouseMoveEvent(event);
}

void PetWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
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
    m_displayLabel->setPixmap(pixmap);
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
    if (!m_moveEnabled || m_dragging) {
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
