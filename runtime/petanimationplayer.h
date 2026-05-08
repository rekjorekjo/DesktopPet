#ifndef PETANIMATIONPLAYER_H
#define PETANIMATIONPLAYER_H

#include <QList>
#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QTimer>

#include "models/petaction.h"

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

    void setSpeedMultiplier(double multiplier);
    double speedMultiplier() const;

    void updatePlaybackOptions(bool loop, int repeat);

    bool isPlaying() const;
    bool isPaused() const;
    bool isLoaded() const;

signals:
    void frameChanged(const QPixmap &pixmap);
    void finished();
    void errorOccurred(const QString &message);

private slots:
    void nextFrame();

private:
    static constexpr int DefaultFps = 12;
    static constexpr int MinFps = 1;
    static constexpr int MaxFps = 60;
    static constexpr int MinIntervalMs = 16;

    static int clampFps(int fps);
    static int calculateIntervalMs(int fps, double speedMultiplier);
    void applyPlaybackOptions(bool loop, int repeat);

    QList<QPixmap> m_frames;
    QTimer *m_timer;
    int m_currentFrameIndex;
    int m_currentLoopCount;
    int m_targetRepeat;
    bool m_loop;
    bool m_playing;
    bool m_paused;
    int m_intervalMs;
    double m_speedMultiplier;
    int m_baseFps;
};

#endif // PETANIMATIONPLAYER_H
