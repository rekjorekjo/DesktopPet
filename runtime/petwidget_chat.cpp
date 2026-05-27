#include "petwidget.h"

#include "services/apiprofileservice.h"
#include "widgets/petchatwidget.h"

#include <QEvent>
#include <QTimer>
#include <QWidget>

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

void PetWidget::keepPetAboveChat()
{
    if (m_chatWidget && m_chatVisible && m_chatWidget->isVisible()) {
        raise();
    }
}
