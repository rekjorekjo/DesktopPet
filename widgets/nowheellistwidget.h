#pragma once

#include <QListWidget>

class NoWheelListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit NoWheelListWidget(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
};
