#include "wheelguardlistwidget.h"

#include <QWheelEvent>

WheelGuardListWidget::WheelGuardListWidget(QWidget *parent)
    : QListWidget(parent)
{
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setUniformItemSizes(true);
}

void WheelGuardListWidget::wheelEvent(QWheelEvent *event)
{
    QListWidget::wheelEvent(event);
    event->accept();
}
