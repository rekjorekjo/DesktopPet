#ifndef ACTIONLIBRARYLISTWIDGET_H
#define ACTIONLIBRARYLISTWIDGET_H

#include <QListWidget>

class ActionLibraryListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit ActionLibraryListWidget(QWidget *parent = nullptr);

protected:
    void startDrag(Qt::DropActions supportedActions) override;
};

#endif // ACTIONLIBRARYLISTWIDGET_H
