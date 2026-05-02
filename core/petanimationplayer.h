#ifndef PETANIMATIONPLAYER_H
#define PETANIMATIONPLAYER_H

#include <QList>
#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QTimer>

#include "core/petaction.h"

class PetAnimationPlayer : public QObject
{
    Q_OBJECT

public:
    explicit PetAnimationPlayer(QObject *parent = nullptr);

    bool loadAction(const PetAction &action, const QSize &displaySize);
    void play(bool loop = false, int repeat = 1);
    void stop();
    void pause();
    void resume();

    bool isPlaying() const;
    bool isLoaded() const;

signals:
    void frameChanged(const QPixmap &pixmap);
    void finished();
    void errorOccurred(const QString &message);

private slots:
    void nextFrame();

private:
    QList<QPixmap> m_frames;
    QTimer *m_timer;
    int m_currentFrameIndex;
    int m_currentLoopCount;
    int m_targetRepeat;
    bool m_loop;
    bool m_playing;
    bool m_paused;
    int m_intervalMs;
};

#endif // PETANIMATIONPLAYER_H
