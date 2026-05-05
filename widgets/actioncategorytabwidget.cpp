#include "actioncategorytabwidget.h"

#include <QDragEnterEvent>
#include <QDragMoveEvent>
#include <QTabBar>
#include <QMimeData>
#include <QTimer>

namespace {
const QString ACTION_ID_MIME_TYPE = "application/x-desktoppet-action-id";
}

ActionCategoryTabWidget::ActionCategoryTabWidget(QWidget *parent)
    : QTabWidget(parent)
    , m_hoveredTabIndex(-1)
{
    setAcceptDrops(true);
}

void ActionCategoryTabWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(ACTION_ID_MIME_TYPE)) {
        event->acceptProposedAction();
    } else {
        QTabWidget::dragEnterEvent(event);
    }
}

void ActionCategoryTabWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(ACTION_ID_MIME_TYPE)) {
        event->acceptProposedAction();

        QPoint pos = event->pos();
        QTabBar *bar = tabBar();
        int tabBarHeight = bar->height();

        if (pos.y() <= tabBarHeight) {
            int tabIndex = bar->tabAt(pos);

            if (tabIndex >= 0 && tabIndex != currentIndex() && tabIndex != m_hoveredTabIndex) {
                m_hoveredTabIndex = tabIndex;
                setCurrentIndex(tabIndex);
            }
        } else {
            m_hoveredTabIndex = -1;
        }
    } else {
        QTabWidget::dragMoveEvent(event);
    }
}

void ActionCategoryTabWidget::dragLeaveEvent(QDragLeaveEvent *event)
{
    m_hoveredTabIndex = -1;
    QTabWidget::dragLeaveEvent(event);
}
