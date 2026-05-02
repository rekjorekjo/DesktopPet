#ifndef ABOUTPAGE_H
#define ABOUTPAGE_H

#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

class QFrame;

class AboutPage : public QWidget
{
    Q_OBJECT

public:
    explicit AboutPage(QWidget *parent = nullptr);
    ~AboutPage();

public slots:
    void refreshTheme();

private:
    void setupUi();
    void applyTheme();

    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;

    QLabel *m_titleLabel;
    QFrame *m_infoCard;
    QLabel *m_infoCardTitle;
    QLabel *m_appNameLabel;
    QLabel *m_techStackLabel;
    QLabel *m_descriptionLabel;

    QFrame *m_versionCard;
    QLabel *m_versionCardTitle;
    QLabel *m_versionLabel;
    QLabel *m_statusLabel;

    QFrame *m_updateCard;
    QLabel *m_updateCardTitle;
    QLabel *m_updateNoteLabel;
    QPushButton *m_checkUpdateButton;
};

#endif // ABOUTPAGE_H
