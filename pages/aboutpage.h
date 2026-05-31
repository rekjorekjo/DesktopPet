#ifndef ABOUTPAGE_H
#define ABOUTPAGE_H

#include <QLabel>
#include <QPushButton>
#include <QScrollArea>
#include <QWidget>

#include "widgets/softcardwidget.h"

class QFrame;
class QProgressBar;
class UpdateService;
struct UpdateInfo;

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
    void onCheckFinished(const UpdateInfo &info);
    void onCheckFailed(const QString &errorMessage);
    void onDownloadClicked();
    void onDownloadProgress(qint64 received, qint64 total);
    void onDownloadFinished(const QString &filePath);
    void onDownloadFailed(const QString &message);
    void onOpenReleasePageClicked();

private:
    void setupUi();
    void applyTheme();
    void showUpdateAvailable(const UpdateInfo &info);
    void showNoUpdate();
    void resetCheckButton();

    QScrollArea *m_scrollArea;
    QWidget *m_contentWidget;

    QLabel *m_titleLabel;
    SoftCardWidget *m_infoCard;
    QLabel *m_infoCardTitle;
    QLabel *m_iconLabel;
    QLabel *m_appNameLabel;
    QLabel *m_descriptionLabel;

    SoftCardWidget *m_versionCard;
    QLabel *m_versionCardTitle;
    QLabel *m_versionLabel;
    QLabel *m_updateStatusLabel;
    QLabel *m_latestVersionLabel;
    QLabel *m_releaseNotesLabel;
    QPushButton *m_checkUpdateButton;
    QPushButton *m_downloadButton;
    QPushButton *m_openReleasePageButton;
    QProgressBar *m_downloadProgressBar;
    QLabel *m_downloadStatusLabel;

    UpdateService *m_updateService;
    UpdateInfo *m_currentUpdateInfo;
};

#endif // ABOUTPAGE_H
