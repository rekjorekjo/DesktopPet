#include "actioncategorylistwidget.h"

#include <QDragEnterEvent>
#include <QDropEvent>
#include <QMimeData>

static const char *ACTION_ID_MIME_TYPE = "application/x-desktoppet-action-id";

ActionCategoryListWidget::ActionCategoryListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setAcceptDrops(true);
    setDragDropMode(QAbstractItemView::DropOnly);
    setDropIndicatorShown(true);
}

void ActionCategoryListWidget::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasFormat(ACTION_ID_MIME_TYPE)) {
        event->acceptProposedAction();
    } else {
        QListWidget::dragEnterEvent(event);
    }
}

void ActionCategoryListWidget::dragMoveEvent(QDragMoveEvent *event)
{
    if (event->mimeData()->hasFormat(ACTION_ID_MIME_TYPE)) {
        event->acceptProposedAction();
    } else {
        QListWidget::dragMoveEvent(event);
    }
}

void ActionCategoryListWidget::dropEvent(QDropEvent *event)
{
    if (event->mimeData()->hasFormat(ACTION_ID_MIME_TYPE)) {
        QString actionId = QString::fromUtf8(event->mimeData()->data(ACTION_ID_MIME_TYPE));
        if (!actionId.isEmpty()) {
            emit actionDropped(actionId);
        }
        event->acceptProposedAction();
    } else {
        QListWidget::dropEvent(event);
    }
}
