#pragma once

#include <QListWidget>

class WheelGuardListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit WheelGuardListWidget(QWidget *parent = nullptr);

protected:
    void wheelEvent(QWheelEvent *event) override;
};
