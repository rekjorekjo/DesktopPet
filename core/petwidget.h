#ifndef PETWIDGET_H
#define PETWIDGET_H

#include <QLabel>
#include <QPoint>
#include <QWidget>

#include "core/petaction.h"
#include "core/petanimationplayer.h"
#include "core/petconfigmanager.h"
#include "core/petplaylist.h"

class QContextMenuEvent;

enum class PetPlayMode
{
    Idle,
    Random
};

class PetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PetWidget(QWidget *parent = nullptr);
    ~PetWidget();

    bool loadPet(const QString &petDirPath);
    void playIdleAction();

protected:
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;

signals:
    void openSettingsRequested();
    void quitRequested();

private slots:
    void onFrameChanged(const QPixmap &pixmap);
    void onActionFinished();
    void onErrorOccurred(const QString &message);
    void triggerRandomAction();

private:
    void setupUi();
    PetAction findActionById(const QString &actionId) const;
    bool playAction(const PetAction &action, const PetActionRef &ref);
    bool playActionByRef(const PetActionRef &ref);

    QLabel *m_displayLabel;
    PetAnimationPlayer *m_player;
    QTimer *m_randomTimer;

    PetBasicInfo m_petInfo;
    QList<PetAction> m_actions;
    PetPlaylist m_playlist;

    PetAction m_currentAction;
    PetActionRef m_currentActionRef;
    QString m_currentActionId;
    PetPlayMode m_currentMode;

    QPoint m_dragPosition;
};

#endif // PETWIDGET_H
