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

private:
    void setupUi();
    PetAction findActionById(const QString &actionId) const;
    void playAction(const PetAction &action, const PetActionRef &ref);
    void playActionByRef(const PetActionRef &ref);

    QLabel *m_displayLabel;
    PetAnimationPlayer *m_player;

    PetBasicInfo m_petInfo;
    QList<PetAction> m_actions;
    PetPlaylist m_playlist;

    PetAction m_currentAction;
    PetActionRef m_currentActionRef;
    QString m_currentActionId;

    QPoint m_dragPosition;
};

#endif // PETWIDGET_H
