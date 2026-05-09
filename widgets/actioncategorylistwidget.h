#ifndef ACTIONCATEGORYLISTWIDGET_H
#define ACTIONCATEGORYLISTWIDGET_H

#include <QListWidget>

class EmptyStateWidget;

class ActionCategoryListWidget : public QListWidget
{
    Q_OBJECT

public:
    explicit ActionCategoryListWidget(QWidget *parent = nullptr);

    void setEmptyStateText(const QString &title, const QString &description);
    void applyTheme();

signals:
    void actionDropped(const QString &actionId);

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dragMoveEvent(QDragMoveEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateEmptyStateVisibility();

    EmptyStateWidget *m_emptyState;
    QString m_emptyTitle;
    QString m_emptyDescription;
};

#endif // ACTIONCATEGORYLISTWIDGET_H
