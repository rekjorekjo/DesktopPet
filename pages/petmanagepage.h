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
    void reloadPetInfo();

signals:
    void startPetRequested();
    void pausePetRequested();
    void applyConfigRequested();

private:
    void setupUi();
    void applyTheme();
    void loadPetInfo();
    void updateInfoDisplay();
    void setRunningStatus(bool running);
    void connectSignals();
    void updateButtonStates();

    int usablePetActionCount() const;
    int globalActionResourceCount() const;

private slots:
    void onCreatePet();

private:
    QLabel *m_titleLabel;
    QLabel *m_currentPetLabel;

    QFrame *m_infoCard;
    QLabel *m_petNameLabel;
    QLabel *m_petIdLabel;
    QLabel *m_petDirLabel;
    QLabel *m_canvasSizeLabel;
    QLabel *m_displaySizeLabel;
    QLabel *m_petActionCountLabel;
    QLabel *m_globalActionCountLabel;
    QLabel *m_statusLabel;

    QPushButton *m_createPetButton;
    QPushButton *m_startButton;
    QPushButton *m_pauseButton;
    QPushButton *m_reloadButton;

    PetBasicInfo m_petInfo;
    QList<PetAction> m_actions;
    PetPlaylist m_playlist;
    bool m_loadSuccess;
};

#endif // PETMANAGEPAGE_H
