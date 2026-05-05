#include "gifframeextractor.h"

#include <QDir>
#include <QImage>
#include <QImageReader>
#include <QObject>

GifProbeResult GifFrameExtractor::probeGif(const QString &gifPath)
{
    GifProbeResult result;
    result.success = false;
    result.errorMessage = QString();
    result.frameCount = 0;
    result.fps = 12;

    QImageReader reader(gifPath);
    if (!reader.canRead()) {
        result.errorMessage = QObject::tr("无法读取 GIF 文件。");
        return result;
    }

    reader.setAutoTransform(true);

    QList<int> delays;
    int frameCount = 0;
    const int maxFrames = 500;

    while (reader.canRead() && frameCount < maxFrames) {
        int delay = reader.nextImageDelay();
        if (delay <= 0) {
            delay = 100;
        }

        QImage frame = reader.read();
        if (frame.isNull()) {
            break;
        }

        delays.append(delay);
        ++frameCount;
    }

    if (frameCount >= maxFrames && reader.canRead()) {
        result.errorMessage = QObject::tr("GIF 帧数过多，请使用较小的 GIF。");
        return result;
    }

    if (frameCount == 0) {
        result.errorMessage = QObject::tr("GIF 文件没有可导出的帧。");
        return result;
    }

    result.frameCount = frameCount;

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

GifExtractResult GifFrameExtractor::extractGifToFrames(
    const QString &gifPath,
    const QString &outputDir)
{
    GifExtractResult result;
    result.success = false;
    result.errorMessage = QString();
    result.frameCount = 0;
    result.fps = 12;

    QImageReader reader(gifPath);
    if (!reader.canRead()) {
        result.errorMessage = QObject::tr("无法读取 GIF 文件。");
        return result;
    }

    reader.setAutoTransform(true);

    QDir outputDirObj(outputDir);
    if (!outputDirObj.exists()) {
        if (!outputDirObj.mkpath(".")) {
            result.errorMessage = QObject::tr("无法创建输出目录。");
            return result;
        }
    }

    QList<int> delays;
    int frameIndex = 0;
    const int maxFrames = 500;

    while (reader.canRead() && frameIndex < maxFrames) {
        int delay = reader.nextImageDelay();
        if (delay <= 0) {
            delay = 100;
        }

        QImage frame = reader.read();
        if (frame.isNull()) {
            break;
        }

        QString framePath = outputDir + QString("/%1.png").arg(frameIndex, 5, 10, QChar('0'));
        if (!frame.save(framePath, "PNG")) {
            result.errorMessage = QObject::tr("无法保存帧：%1").arg(frameIndex);
            return result;
        }

        delays.append(delay);
        ++frameIndex;
    }

    if (frameIndex >= maxFrames && reader.canRead()) {
        result.errorMessage = QObject::tr("GIF 帧数过多，请使用较小的 GIF。");
        return result;
    }

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
