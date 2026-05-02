#ifndef DEFAULTPETPAGE_H
#define DEFAULTPETPAGE_H

#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QWidget>

class DefaultPetPage : public QWidget
{
    Q_OBJECT

public:
    explicit DefaultPetPage(QWidget *parent = nullptr);
    ~DefaultPetPage();

public slots:
    void refreshTheme();

private:
    void setupUi();
    void applyTheme();

    QLabel *m_titleLabel;
    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;
    QFrame *m_petCard;
    QLabel *m_petCardTitle;
    QListWidget *m_petResourceList;
};

#endif // DEFAULTPETPAGE_H
