#ifndef SOFTDIALOGTITLEBAR_H
#define SOFTDIALOGTITLEBAR_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>

class SoftDialogTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit SoftDialogTitleBar(QWidget *parent = nullptr);
    explicit SoftDialogTitleBar(const QString &title, QWidget *parent = nullptr);
    ~SoftDialogTitleBar();

    void setTitle(const QString &title);
    QString title() const;

    void applyTheme();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;

private:
    void setupUi();
    void connectSignals();

private slots:
    void onCloseClicked();

private:
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QPushButton *m_closeButton;

    QPoint m_dragPosition;
    bool m_dragging;
    QString m_title;
};

#endif // SOFTDIALOGTITLEBAR_H
