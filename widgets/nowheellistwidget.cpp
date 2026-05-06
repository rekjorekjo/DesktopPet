#include "nowheellistwidget.h"

#include <QWheelEvent>

NoWheelListWidget::NoWheelListWidget(QWidget *parent)
    : QListWidget(parent)
{
}

void NoWheelListWidget::wheelEvent(QWheelEvent *event)
{
    event->ignore();
}
