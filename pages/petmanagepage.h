#ifndef PETMANAGEPAGE_H
#define PETMANAGEPAGE_H

#include <QWidget>

#include "core/petconfigmanager.h"
#include "models/petaction.h"
#include "models/petplaylist.h"

class QLabel;
class QPushButton;
class QFrame;
class QListWidget;
class QListWidgetItem;

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
    void updatePreviewForPet(const QString &petId);
    void setRunningStatus(bool running);
    void connectSignals();
    void updateButtonStates();
    void refreshPetList();
    QString petDisplayName(const QString &petId) const;
    QString firstEnabledPetId() const;

    int usablePetActionCount() const;
    int globalActionResourceCount() const;

private slots:
    void onCreatePet();
    void onImportPet();
    void onPetListItemClicked(QListWidgetItem *item);
    void onPetListContextMenu(const QPoint &pos);
    void onSwitchToPet();
    void onEditPet();
    void onDisablePet();
    void onDeletePet();

private:
    QLabel *m_titleLabel;
    QLabel *m_currentPetLabel;
    QLabel *m_petListTitleLabel;

    QListWidget *m_petListWidget;

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
    QPushButton *m_importPetButton;
    QPushButton *m_startButton;
    QPushButton *m_pauseButton;
    QPushButton *m_reloadButton;

    PetBasicInfo m_petInfo;
    QList<PetAction> m_actions;
    PetPlaylist m_playlist;
    bool m_loadSuccess;
};

#endif // PETMANAGEPAGE_H
