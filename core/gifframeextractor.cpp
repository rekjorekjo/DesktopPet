#include "gifframeextractor.h"

#include <QDir>
#include <QImage>
#include <QMovie>
#include <QObject>

GifExtractResult GifFrameExtractor::extractGifToFrames(
    const QString &gifPath,
    const QString &outputDir)
{
    GifExtractResult result;
    result.success = false;
    result.errorMessage = QString();
    result.frameCount = 0;
    result.fps = 12;

    QMovie movie(gifPath);
    if (!movie.isValid()) {
        result.errorMessage = QObject::tr("无法读取 GIF 文件。");
        return result;
    }

    QDir outputDirObj(outputDir);
    if (!outputDirObj.exists()) {
        if (!outputDirObj.mkpath(".")) {
            result.errorMessage = QObject::tr("无法创建输出目录。");
            return result;
        }
    }

    QList<int> delays;
    int frameIndex = 0;
    const int maxFrames = 2000;

    if (!movie.jumpToFrame(0)) {
        result.errorMessage = QObject::tr("GIF 文件没有可导出的帧。");
        return result;
    }

    do {
        QImage frame = movie.currentImage();
        if (frame.isNull()) {
            continue;
        }

        QString framePath = outputDir + QString("/%1.png").arg(frameIndex);
        if (!frame.save(framePath, "PNG")) {
            result.errorMessage = QObject::tr("无法保存帧：%1").arg(frameIndex);
            return result;
        }

        int delay = movie.nextFrameDelay();
        if (delay <= 0) {
            delay = 100;
        }
        delays.append(delay);

        ++frameIndex;

        if (frameIndex >= maxFrames) {
            break;
        }
    } while (movie.jumpToNextFrame());

    if (frameIndex == 0) {
        result.errorMessage = QObject::tr("GIF 文件没有可导出的帧。");
        return result;
    }

    result.frameCount = frameIndex;

    if (!delays.isEmpty()) {
        double totalDelay = 0;
        for (int d : delays) {
            totalDelay += d;
        }
        double avgDelay = totalDelay / delays.size();

        if (avgDelay > 0) {
            int calculatedFps = qRound(1000.0 / avgDelay);
            if (calculatedFps < 1) {
                calculatedFps = 1;
            } else if (calculatedFps > 60) {
                calculatedFps = 60;
            }
            result.fps = calculatedFps;
        }
    }

    result.success = true;
    return result;
}
