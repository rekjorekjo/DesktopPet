#include "petwidget.h"

#include "theme/thememanager.h"

#include <QContextMenuEvent>
#include <QDebug>
#include <QGuiApplication>
#include <QHideEvent>
#include <QMenu>
#include <QMouseEvent>
#include <QMoveEvent>
#include <QRandomGenerator>
#include <QScreen>
#include <QTimer>

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
