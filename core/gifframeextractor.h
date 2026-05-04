#ifndef GIFFRAMEEXTRACTOR_H
#define GIFFRAMEEXTRACTOR_H

#include <QString>

struct GifProbeResult
{
    bool success;
    QString errorMessage;
    int frameCount;
    int fps;
};

struct GifExtractResult
{
    bool success;
    QString errorMessage;
    int frameCount;
    int fps;
};

class GifFrameExtractor
{
public:
    static GifProbeResult probeGif(const QString &gifPath);

    static GifExtractResult extractGifToFrames(
        const QString &gifPath,
        const QString &outputDir
    );

private:
    GifFrameExtractor() = delete;
};

#endif // GIFFRAMEEXTRACTOR_H
