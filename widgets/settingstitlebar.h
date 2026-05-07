#ifndef SETTINGSTITLEBAR_H
#define SETTINGSTITLEBAR_H

#include <QLabel>
#include <QPushButton>
#include <QWidget>

class SettingsTitleBar : public QWidget
{
    Q_OBJECT

public:
    explicit SettingsTitleBar(QWidget *parent = nullptr);
    ~SettingsTitleBar();

    void applyTheme();

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;

private:
    void setupUi();
    void connectSignals();
    void updateMaximizeButtonIcon();

private slots:
    void onMinimizeClicked();
    void onMaximizeClicked();
    void onCloseClicked();

private:
    QLabel *m_iconLabel;
    QLabel *m_titleLabel;
    QPushButton *m_minimizeButton;
    QPushButton *m_maximizeButton;
    QPushButton *m_closeButton;

    QPoint m_dragPosition;
    bool m_dragging;
};

#endif // SETTINGSTITLEBAR_H
