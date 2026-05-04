#ifndef GIFFRAMEEXTRACTOR_H
#define GIFFRAMEEXTRACTOR_H

#include <QString>

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
    static GifExtractResult extractGifToFrames(
        const QString &gifPath,
        const QString &outputDir
    );

private:
    GifFrameExtractor() = delete;
};

#endif // GIFFRAMEEXTRACTOR_H
