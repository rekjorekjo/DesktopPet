#include "petwidget.h"

#include "core/petpaths.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <QMouseEvent>
#include <QRandomGenerator>
#include <QTimer>
#include <QVBoxLayout>

PetWidget::PetWidget(QWidget *parent)
    : QWidget(parent)
    , m_displayLabel(nullptr)
    , m_player(nullptr)
    , m_randomTimer(nullptr)
    , m_timedCheckTimer(nullptr)
    , m_currentMode(PetPlayMode::Idle)
    , m_petRunning(true)
    , m_petScaleFactor(1.0)
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

    loadPet(PetPaths::defaultPetDirectory());
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

bool PetWidget::loadPet(const QString &petDirPath)
{
    if (!PetConfigManager::loadPetFromDirectory(petDirPath, m_petInfo, m_actions, m_playlist)) {
        m_displayLabel->setText("宠物资源加载失败");
        m_displayLabel->setStyleSheet("color: red; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
        return false;
    }

    if (m_actions.isEmpty()) {
        m_displayLabel->setText("没有可用动作");
        m_displayLabel->setStyleSheet("color: orange; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
        return false;
    }

    QSize displaySize = currentDisplaySize();
    setFixedSize(displaySize);
    m_displayLabel->setFixedSize(displaySize);

    playIdleAction();

    m_randomTimer->start(30000);
    m_timedCheckTimer->start(1000);

    return true;
}

bool PetWidget::playAction(const PetAction &action, const PetActionRef &ref)
{
    if (!action.isValid()) {
        m_displayLabel->setText("动作无效");
        m_displayLabel->setStyleSheet("color: orange; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
        return false;
    }

    if (!m_player->loadAction(action, currentDisplaySize())) {
        m_displayLabel->setText("动作加载失败");
        m_displayLabel->setStyleSheet("color: red; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
        return false;
    }

    m_player->setSpeedMultiplier(ref.animationSpeed);

    m_currentAction = action;
    m_currentActionRef = ref;
    m_currentActionId = action.id;

    m_displayLabel->setStyleSheet("background-color: rgba(0, 0, 0, 50); border-radius: 10px;");

    m_player->play(ref.loop, ref.repeat);
    return true;
}

bool PetWidget::playActionByRef(const PetActionRef &ref)
{
    PetAction action = findActionById(ref.actionId);
    if (!action.isValid()) {
        m_displayLabel->setText("找不到动作: " + ref.actionId);
        m_displayLabel->setStyleSheet("color: orange; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
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
        if (playActionByRef(idleRefs.first())) {
            return;
        }
    }

    if (!m_actions.isEmpty()) {
        PetActionRef defaultRef(m_actions.first().id);
        defaultRef.loop = true;
        defaultRef.repeat = 0;
        if (playAction(m_actions.first(), defaultRef)) {
            return;
        }
    }

    m_displayLabel->setText("没有可用动作");
    m_displayLabel->setStyleSheet("color: orange; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
}

void PetWidget::startPet()
{
    if (m_petRunning) {
        return;
    }

    m_petRunning = true;

    if (m_player->isPaused()) {
        m_player->resume();
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
    m_player->pause();
}

void PetWidget::reloadPet()
{
    m_randomTimer->stop();
    m_timedCheckTimer->stop();
    m_player->stop();

    m_currentAction = PetAction();
    m_currentActionRef = PetActionRef();
    m_currentActionId.clear();
    m_currentMode = PetPlayMode::Idle;
    m_lastTimedTriggerTimes.clear();

    bool wasRunning = m_petRunning;

    if (!PetConfigManager::loadPetFromDirectory(PetPaths::defaultPetDirectory(), m_petInfo, m_actions, m_playlist)) {
        m_petRunning = false;
        m_displayLabel->setText("宠物资源加载失败");
        m_displayLabel->setStyleSheet("color: red; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
        return;
    }

    if (m_actions.isEmpty()) {
        m_petRunning = false;
        m_displayLabel->setText("没有可用动作");
        m_displayLabel->setStyleSheet("color: orange; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
        return;
    }

    QSize displaySize = currentDisplaySize();
    setFixedSize(displaySize);
    m_displayLabel->setFixedSize(displaySize);

    if (wasRunning) {
        m_petRunning = true;
        playIdleAction();
        m_randomTimer->start(30000);
        m_timedCheckTimer->start(1000);
    } else {
        m_petRunning = false;
        m_displayLabel->setText("已暂停");
        m_displayLabel->setStyleSheet("color: gray; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
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

PetAction PetWidget::findActionById(const QString &actionId) const
{
    for (const PetAction &action : m_actions) {
        if (action.id == actionId) {
            return action;
        }
    }
    return PetAction();
}

void PetWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
    }
}

void PetWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        move(event->globalPosition().toPoint() - m_dragPosition);
        event->accept();
    }
}

void PetWidget::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu menu(this);
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

    for (int i = 0; i < timedRefs.size(); ++i) {
        const PetActionRef &ref = timedRefs.at(i);

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

void PetWidget::onErrorOccurred(const QString &message)
{
    m_displayLabel->setText("播放错误: " + message);
    m_displayLabel->setStyleSheet("color: red; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
}
