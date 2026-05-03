#include "petwidget.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QDir>
#include <QMenu>
#include <QMouseEvent>
#include <QVBoxLayout>

PetWidget::PetWidget(QWidget *parent)
    : QWidget(parent)
    , m_displayLabel(nullptr)
    , m_player(nullptr)
{
    setupUi();

    m_player = new PetAnimationPlayer(this);
    connect(m_player, &PetAnimationPlayer::frameChanged, this, &PetWidget::onFrameChanged);
    connect(m_player, &PetAnimationPlayer::finished, this, &PetWidget::onActionFinished);
    connect(m_player, &PetAnimationPlayer::errorOccurred, this, &PetWidget::onErrorOccurred);

    QString defaultPetDir = QDir::currentPath() + "/pets/default_pet";
    loadPet(defaultPetDir);
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

    setFixedSize(m_petInfo.displaySize);
    m_displayLabel->setFixedSize(m_petInfo.displaySize);

    playIdleAction();

    return true;
}

void PetWidget::playAction(const PetAction &action, const PetActionRef &ref)
{
    if (!action.isValid()) {
        m_displayLabel->setText("动作无效");
        m_displayLabel->setStyleSheet("color: orange; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
        return;
    }

    if (!m_player->loadAction(action, m_petInfo.displaySize)) {
        m_displayLabel->setText("动作加载失败");
        m_displayLabel->setStyleSheet("color: red; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
        return;
    }

    m_currentAction = action;
    m_currentActionRef = ref;
    m_currentActionId = action.id;

    m_displayLabel->setStyleSheet("background-color: rgba(0, 0, 0, 50); border-radius: 10px;");

    m_player->play(ref.loop, ref.repeat);
}

void PetWidget::playActionByRef(const PetActionRef &ref)
{
    PetAction action = findActionById(ref.actionId);
    if (!action.isValid()) {
        m_displayLabel->setText("找不到动作: " + ref.actionId);
        m_displayLabel->setStyleSheet("color: orange; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
        return;
    }

    playAction(action, ref);
}

void PetWidget::playIdleAction()
{
    QList<PetActionRef> idleRefs = m_playlist.idleActions();
    if (!idleRefs.isEmpty()) {
        playActionByRef(idleRefs.first());
        return;
    }

    if (!m_actions.isEmpty()) {
        PetActionRef defaultRef(m_actions.first().id);
        defaultRef.loop = true;
        defaultRef.repeat = 0;
        playAction(m_actions.first(), defaultRef);
        return;
    }

    m_displayLabel->setText("没有可用动作");
    m_displayLabel->setStyleSheet("color: orange; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
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
    QAction *settingsAction = menu.addAction(tr("打开设置"));
    menu.addSeparator();
    QAction *quitAction = menu.addAction(tr("退出"));

    QAction *selectedAction = menu.exec(event->globalPos());

    if (selectedAction == settingsAction) {
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
    playIdleAction();
}

void PetWidget::onErrorOccurred(const QString &message)
{
    m_displayLabel->setText("播放错误: " + message);
    m_displayLabel->setStyleSheet("color: red; background-color: rgba(0, 0, 0, 180); border-radius: 10px;");
}
