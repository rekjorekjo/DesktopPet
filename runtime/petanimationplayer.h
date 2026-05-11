#ifndef PETANIMATIONPLAYER_H
#define PETANIMATIONPLAYER_H

#include <QList>
#include <QObject>
#include <QPixmap>
#include <QSize>
#include <QTimer>

#include "models/petaction.h"

// 帧动画播放器，负责动作帧的加载、播放控制和帧更新
//
// 职责：
// - 加载动作的所有帧图像到内存缓存
// - 按指定 FPS 和速度播放帧动画
// - 支持循环播放和指定次数播放
// - 支持暂停、恢复和停止
//
// 注意：
// - stop() 只停止播放，不清空帧缓存
// - clear() 会清空帧缓存和所有状态，用于完全重置
class PetAnimationPlayer : public QObject
{
    Q_OBJECT

public:
    explicit PetAnimationPlayer(QObject *parent = nullptr);

    // 加载动作的所有帧到内存
    bool loadAction(const PetAction &action, const QSize &displaySize);

    // 开始播放
    // loop: 是否循环播放
    // repeat: 非循环时的重复次数
    void play(bool loop = false, int repeat = 1);

    // 停止播放，但保留帧缓存
    void stop();
    void pause();
    void resume();

    // 清空帧缓存和所有状态
    // stop() 不等于清空帧缓存，clear() 才会完全重置
    void clear();

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

    // 帧图像缓存，存储加载的所有帧
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
