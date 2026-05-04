#ifndef ABOUTPAGE_H
#define ABOUTPAGE_H

#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

class QFrame;
class UpdateManager;

class AboutPage : public QWidget
{
    Q_OBJECT

public:
    explicit AboutPage(QWidget *parent = nullptr);
    ~AboutPage();

public slots:
    void refreshTheme();

private slots:
    void onCheckUpdateClicked();
    void onUpdateAvailable(const QString &latestVersion, const QString &releaseUrl);
    void onNoUpdateAvailable(const QString &latestVersion);
    void onCheckFailed(const QString &errorMessage);

private:
    void setupUi();
    void applyTheme();

    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;

    QLabel *m_titleLabel;
    QFrame *m_infoCard;
    QLabel *m_infoCardTitle;
    QLabel *m_iconLabel;
    QLabel *m_appNameLabel;
    QLabel *m_descriptionLabel;

    QFrame *m_versionCard;
    QLabel *m_versionCardTitle;
    QLabel *m_versionLabel;

    QFrame *m_updateCard;
    QLabel *m_updateCardTitle;
    QLabel *m_updateStatusLabel;
    QPushButton *m_checkUpdateButton;

    UpdateManager *m_updateManager;
};

#endif // ABOUTPAGE_H
