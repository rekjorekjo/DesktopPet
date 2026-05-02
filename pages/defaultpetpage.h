#ifndef DEFAULTPETPAGE_H
#define DEFAULTPETPAGE_H

#include <QLabel>
#include <QListWidget>
#include <QScrollArea>
#include <QTabWidget>
#include <QWidget>

class QFrame;

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
    void loadMockData();

    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;

    QLabel *m_titleLabel;
    QFrame *m_infoCard;
    QLabel *m_petNameLabel;
    QLabel *m_canvasSizeLabel;
    QLabel *m_actionCountLabel;
    QLabel *m_petDirLabel;

    QLabel *m_libraryTitleLabel;
    QListWidget *m_actionLibraryList;

    QLabel *m_configTitleLabel;
    QTabWidget *m_categoryTabs;
    QListWidget *m_dailyActionList;
    QListWidget *m_randomActionList;
    QListWidget *m_scheduledActionList;
    QListWidget *m_emotionActionList;
};

#endif // DEFAULTPETPAGE_H
