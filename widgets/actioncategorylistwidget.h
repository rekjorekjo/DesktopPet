#ifndef ACTIONCATEGORYLISTWIDGET_H
#define ACTIONCATEGORYLISTWIDGET_H

#include <QListWidget>

class ActionCategoryListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit ActionCategoryListWidget(QWidget *parent = nullptr);

signals:
    void actionDropped(const QString &actionId);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
};

#endif // ACTIONCATEGORYLISTWIDGET_H
