#ifndef PETMANAGEPAGE_H
#define PETMANAGEPAGE_H

#include <QWidget>

#include "core/petaction.h"
#include "core/petconfigmanager.h"
#include "core/petplaylist.h"

class QLabel;
class QPushButton;
class QFrame;

class PetManagePage : public QWidget
{
    Q_OBJECT

public:
    explicit PetManagePage(QWidget *parent = nullptr);
    ~PetManagePage();

public slots:
    void refreshTheme();

private:
    void setupUi();
    void applyTheme();
    void loadPetInfo();
    void updateInfoDisplay();

private:
    QLabel *m_titleLabel;

    QFrame *m_infoCard;
    QLabel *m_petNameLabel;
    QLabel *m_petDirLabel;
    QLabel *m_canvasSizeLabel;
    QLabel *m_displaySizeLabel;
    QLabel *m_actionCountLabel;
    QLabel *m_statusLabel;

    QPushButton *m_startButton;
    QPushButton *m_pauseButton;
    QPushButton *m_reloadButton;

    PetBasicInfo m_petInfo;
    QList<PetAction> m_actions;
    PetPlaylist m_playlist;
};

#endif // PETMANAGEPAGE_H
