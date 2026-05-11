#ifndef PETWIDGET_H
#define PETWIDGET_H

#include <QDateTime>
#include <QElapsedTimer>
#include <QHash>
#include <QLabel>
#include <QMap>
#include <QPoint>
#include <QWidget>

#include "core/petconfigmanager.h"
#include "models/petaction.h"
#include "models/petplaylist.h"
#include "runtime/petanimationplayer.h"

class QContextMenuEvent;
class QTimer;
class PetChatWidget;

enum class PetPlayMode
{
    Idle,
    Random,
    Timed,
    Emotion
};

enum class PetLoadStatus
{
    Ok,
    NoPet,
    PetConfigError,
    NoActionLibrary,
    NoPlaylistAction
};

class PetWidget : public QWidget
{
    Q_OBJECT

public:
    explicit PetWidget(QWidget *parent = nullptr);
    ~PetWidget();

    bool loadPet(const QString &petDirPath);
    bool playIdleAction();

public slots:
    void startPet();
    void pausePet();
    void reloadPet();
    void reloadPlaylistPreservePlayback();
    void reloadActionsAndPlaylistPreservePlayback();
    void setPetScaleFactor(double scale);
    void setPetOpacity(double opacity);
    void playEmotion(const QString &emotion);
    void setBaseMoveSpeed(int speed);

protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void contextMenuEvent(QContextMenuEvent *event) override;
    void moveEvent(QMoveEvent *event) override;
    void hideEvent(QHideEvent *event) override;

signals:
    void openSettingsRequested();
    void hidePetRequested();
    void quitRequested();
    void petStarted();
    void petStartFailed(const QString &message);
    void petPaused();

private slots:
    void onFrameChanged(const QPixmap &pixmap);
    void onActionFinished();
    void onErrorOccurred(const QString &message);
    void triggerRandomAction();
    void checkTimedActions();
    void updateMovement();

private:
    void setupUi();
    QSize currentDisplaySize() const;
    PetAction findActionById(const QString &actionId) const;
    void loadGlobalActionLibrary();
    bool playAction(const PetAction &action, const PetActionRef &ref);
    bool playActionByRef(const PetActionRef &ref);
    bool hasAnyActionResources() const;
    bool hasAnyUsableEnabledAction() const;
    bool hasAnyPlaylistAction() const;
    QString findFirstEnabledPetId() const;
    bool hasAnyEnabledPet() const;
    void showStatusMessage(const QString &title, const QString &subtitle);
    void clearStatusMessage();
    void prepareDisplayLabelForPixmap();
    void startMovement();
    void stopMovement();
    void updateMoveDirection();
    QRect getAvailableScreenGeometry() const;

    void toggleChatWidget();
    void showChatWidget();
    void hideChatWidget();
    void updateChatWidgetGeometry();
    bool isAutoMoving() const;
    void pauseAutoMovementForChat();
    void resumeAutoMovementAfterChat();

    QLabel *m_displayLabel;
    PetAnimationPlayer *m_player;
    QTimer *m_randomTimer;
    QTimer *m_timedCheckTimer;
    QTimer *m_moveTimer;
    QElapsedTimer m_moveElapsedTimer;
    QMap<int, QDateTime> m_lastTimedTriggerTimes;
    QHash<QString, QDate> m_clockTimedLastTriggeredDate;

    PetBasicInfo m_petInfo;
    QList<PetAction> m_actions;
    PetPlaylist m_playlist;

    PetAction m_currentAction;
    PetActionRef m_currentActionRef;
    QString m_currentActionId;
    PetPlayMode m_currentMode;
    bool m_petRunning;
    double m_petScaleFactor;
    int m_idleActionIndex;

    bool m_dragging;
    QPoint m_dragPosition;

    bool m_mousePressing;
    QPoint m_mousePressGlobalPos;
    bool m_mouseDragDetected;

    bool m_moveEnabled;
    int m_moveDirection;
    qreal m_moveVelocity;
    MoveAxis m_moveAxis;
    qreal m_moveRemainderX;
    qreal m_moveRemainderY;

    PetChatWidget *m_chatWidget;
    bool m_chatVisible;
    bool m_autoMovementPausedByChat;
};

#endif // PETWIDGET_H
