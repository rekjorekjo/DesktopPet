#include "imageutils.h"

#include <QDebug>
#include <QPainter>
#include <QPainterPath>

namespace ImageUtils {

QPixmap roundedPixmap(const QPixmap &source, int size, int radius)
{
    if (source.isNull()) {
        qWarning() << "ImageUtils::roundedPixmap: source pixmap is null";
        return QPixmap();
    }

    QPixmap scaled = source.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    if (scaled.width() > size || scaled.height() > size) {
        int x = (scaled.width() - size) / 2;
        int y = (scaled.height() - size) / 2;
        scaled = scaled.copy(x, y, size, size);
    }

    QPixmap result(size, size);
    result.fill(Qt::transparent);

    QPainter painter(&result);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    QPainterPath path;
    path.addRoundedRect(0, 0, size, size, radius, radius);
    painter.setClipPath(path);

    painter.drawPixmap(0, 0, scaled);

    return result;
}

}
