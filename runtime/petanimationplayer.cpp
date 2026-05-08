#include "petanimationplayer.h"

#include <QDebug>

int PetAnimationPlayer::clampFps(int fps)
{
    if (fps < MinFps) {
        return MinFps;
    }
    if (fps > MaxFps) {
        return MaxFps;
    }
    return fps;
}

int PetAnimationPlayer::calculateIntervalMs(int fps, double speedMultiplier)
{
    int clampedFps = clampFps(fps);
    double effectiveFps = clampedFps * speedMultiplier;
    int interval = static_cast<int>(1000.0 / effectiveFps);
    if (interval < MinIntervalMs) {
        interval = MinIntervalMs;
    }
    return interval;
}

PetAnimationPlayer::PetAnimationPlayer(QObject *parent)
    : QObject(parent)
    , m_timer(new QTimer(this))
    , m_currentFrameIndex(0)
    , m_currentLoopCount(0)
    , m_targetRepeat(1)
    , m_loop(false)
    , m_playing(false)
    , m_paused(false)
    , m_intervalMs(calculateIntervalMs(DefaultFps, 1.0))
    , m_speedMultiplier(1.0)
    , m_baseFps(DefaultFps)
{
    connect(m_timer, &QTimer::timeout, this, &PetAnimationPlayer::nextFrame);
}

bool PetAnimationPlayer::loadAction(const PetAction &action, const QSize &displaySize)
{
    m_timer->stop();
    m_frames.clear();
    m_playing = false;
    m_paused = false;
    m_currentFrameIndex = 0;
    m_currentLoopCount = 0;

    if (action.frameFiles.isEmpty()) {
        emit errorOccurred("No frame files in action: " + action.id);
        return false;
    }

    for (const QString &filePath : action.frameFiles) {
        QPixmap original(filePath);
        if (original.isNull()) {
            qWarning() << "PetAnimationPlayer: Failed to load frame:" << filePath;
            continue;
        }

        QPixmap scaled;
        if (displaySize.isValid() && !displaySize.isEmpty()) {
            scaled = original.scaled(displaySize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        } else {
            scaled = original;
        }

        m_frames.append(scaled);
    }

    if (m_frames.isEmpty()) {
        emit errorOccurred("No valid frames loaded for action: " + action.id);
        return false;
    }

    int fps = action.fps;
    if (fps <= 0) {
        fps = DefaultFps;
    }
    m_baseFps = clampFps(fps);
    m_intervalMs = calculateIntervalMs(m_baseFps, m_speedMultiplier);

    return true;
}

void PetAnimationPlayer::applyPlaybackOptions(bool loop, int repeat)
{
    bool infinite = loop || repeat == 0;

    if (infinite) {
        m_loop = true;
        m_targetRepeat = 0;
    } else {
        m_loop = false;
        m_targetRepeat = qBound(1, repeat, 10);
    }
}

void PetAnimationPlayer::play(bool loop, int repeat)
{
    if (m_frames.isEmpty()) {
        emit errorOccurred("Cannot play: no frames loaded");
        return;
    }

    applyPlaybackOptions(loop, repeat);

    m_currentFrameIndex = 0;
    m_currentLoopCount = 0;
    m_playing = true;
    m_paused = false;

    emit frameChanged(m_frames.at(0));

    m_timer->start(m_intervalMs);
}

void PetAnimationPlayer::stop()
{
    m_timer->stop();
    m_playing = false;
    m_paused = false;
    m_currentFrameIndex = 0;
    m_currentLoopCount = 0;
}

void PetAnimationPlayer::pause()
{
    if (!m_playing) {
        return;
    }

    m_timer->stop();
    m_paused = true;
}

void PetAnimationPlayer::resume()
{
    if (!m_playing || !m_paused || m_frames.isEmpty()) {
        return;
    }

    m_paused = false;
    m_timer->start(m_intervalMs);
}

void PetAnimationPlayer::setSpeedMultiplier(double multiplier)
{
    if (multiplier < 0.1) {
        multiplier = 0.1;
    } else if (multiplier > 5.0) {
        multiplier = 5.0;
    }

    m_speedMultiplier = multiplier;
    m_intervalMs = calculateIntervalMs(m_baseFps, m_speedMultiplier);

    if (m_playing && !m_paused && m_timer->isActive()) {
        m_timer->setInterval(m_intervalMs);
    }
}

double PetAnimationPlayer::speedMultiplier() const
{
    return m_speedMultiplier;
}

void PetAnimationPlayer::updatePlaybackOptions(bool loop, int repeat)
{
    applyPlaybackOptions(loop, repeat);

    if (!m_loop && m_targetRepeat > 0 && m_currentLoopCount > m_targetRepeat) {
        m_currentLoopCount = qMax(0, m_targetRepeat - 1);
    }
}

bool PetAnimationPlayer::isPlaying() const
{
    return m_playing && !m_paused;
}

bool PetAnimationPlayer::isPaused() const
{
    return m_paused;
}

bool PetAnimationPlayer::isLoaded() const
{
    return !m_frames.isEmpty();
}

void PetAnimationPlayer::nextFrame()
{
    if (m_frames.isEmpty()) {
        m_timer->stop();
        m_playing = false;
        emit errorOccurred("Playback stopped: no frames loaded");
        return;
    }

    m_currentFrameIndex++;

    if (m_currentFrameIndex >= m_frames.size()) {
        if (m_loop) {
            m_currentFrameIndex = 0;
            emit frameChanged(m_frames.at(m_currentFrameIndex));
        } else {
            m_currentLoopCount++;

            if (m_targetRepeat > 0 && m_currentLoopCount >= m_targetRepeat) {
                m_timer->stop();
                m_playing = false;
                m_currentFrameIndex = 0;
                m_currentLoopCount = 0;
                emit finished();
            } else {
                m_currentFrameIndex = 0;
                emit frameChanged(m_frames.at(m_currentFrameIndex));
            }
        }
    } else {
        emit frameChanged(m_frames.at(m_currentFrameIndex));
    }
}
