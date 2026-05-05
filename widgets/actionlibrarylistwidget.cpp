#include "actionlibrarylistwidget.h"

#include <QDrag>
#include <QMimeData>

static const char *ACTION_ID_MIME_TYPE = "application/x-desktoppet-action-id";

ActionLibraryListWidget::ActionLibraryListWidget(QWidget *parent)
    : QListWidget(parent)
{
}

void ActionLibraryListWidget::startDrag(Qt::DropActions supportedActions)
{
    QListWidgetItem *item = currentItem();
    if (!item) {
        return;
    }

    QString actionId = item->data(Qt::UserRole).toString();
    if (actionId.isEmpty()) {
        return;
    }

    QDrag *drag = new QDrag(this);
    QMimeData *mimeData = new QMimeData();
    mimeData->setData(ACTION_ID_MIME_TYPE, actionId.toUtf8());
    drag->setMimeData(mimeData);

    drag->exec(supportedActions);
}
