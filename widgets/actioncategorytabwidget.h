#ifndef ACTIONCATEGORYTABWIDGET_H
#define ACTIONCATEGORYTABWIDGET_H

#include <QTabWidget>

class ActionCategoryTabWidget : public QTabWidget
{
    Q_OBJECT

public:
    explicit ActionCategoryTabWidget(QWidget *parent = nullptr);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dragLeaveEvent(QDragLeaveEvent *event) override;

private:
    int m_hoveredTabIndex;
};

#endif
