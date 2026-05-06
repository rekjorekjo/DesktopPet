#include "glasscardwidget.h"
#include "theme/thememanager.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <QLinearGradient>

GlassCardWidget::GlassCardWidget(QWidget *parent)
    : QFrame(parent)
    , m_borderRadius(12)
    , m_backgroundOpacity(50)
    , m_highlightOpacity(45)
    , m_shadowOpacity(35)
    , m_borderOpacity(90)
    , m_showHighlight(true)
    , m_showShadow(true)
    , m_contentWidget(nullptr)
{
    init();
}

GlassCardWidget::GlassCardWidget(const QString &title, QWidget *parent)
    : QFrame(parent)
    , m_borderRadius(12)
    , m_backgroundOpacity(50)
    , m_highlightOpacity(45)
    , m_shadowOpacity(35)
    , m_borderOpacity(90)
    , m_showHighlight(true)
    , m_showShadow(true)
    , m_title(title)
    , m_contentWidget(nullptr)
{
    init();
    ensureInternalLayout();
}

void GlassCardWidget::init()
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);

    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &GlassCardWidget::applyTheme);
}

void GlassCardWidget::ensureInternalLayout()
{
    if (layout()) {
        return;
    }

    QVBoxLayout *internalLayout = new QVBoxLayout(this);
    internalLayout->setContentsMargins(16, 16, 16, 16);
    internalLayout->setSpacing(8);

    if (!m_title.isEmpty()) {
        QLabel *titleLabel = new QLabel(m_title, this);
        titleLabel->setObjectName("glassCardTitle");
        internalLayout->addWidget(titleLabel);
    }
}

int GlassCardWidget::borderRadius() const
{
    return m_borderRadius;
}

void GlassCardWidget::setBorderRadius(int radius)
{
    if (m_borderRadius != radius) {
        m_borderRadius = radius;
        update();
    }
}

int GlassCardWidget::backgroundOpacity() const
{
    return m_backgroundOpacity;
}

void GlassCardWidget::setBackgroundOpacity(int opacity)
{
    if (m_backgroundOpacity != opacity) {
        m_backgroundOpacity = opacity;
        update();
    }
}

int GlassCardWidget::highlightOpacity() const
{
    return m_highlightOpacity;
}

void GlassCardWidget::setHighlightOpacity(int opacity)
{
    if (m_highlightOpacity != opacity) {
        m_highlightOpacity = opacity;
        update();
    }
}

int GlassCardWidget::shadowOpacity() const
{
    return m_shadowOpacity;
}

void GlassCardWidget::setShadowOpacity(int opacity)
{
    if (m_shadowOpacity != opacity) {
        m_shadowOpacity = opacity;
        update();
    }
}

int GlassCardWidget::borderOpacity() const
{
    return m_borderOpacity;
}

void GlassCardWidget::setBorderOpacity(int opacity)
{
    if (m_borderOpacity != opacity) {
        m_borderOpacity = opacity;
        update();
    }
}

bool GlassCardWidget::showHighlight() const
{
    return m_showHighlight;
}

void GlassCardWidget::setShowHighlight(bool show)
{
    if (m_showHighlight != show) {
        m_showHighlight = show;
        update();
    }
}

bool GlassCardWidget::showShadow() const
{
    return m_showShadow;
}

void GlassCardWidget::setShowShadow(bool show)
{
    if (m_showShadow != show) {
        m_showShadow = show;
        update();
    }
}

void GlassCardWidget::setTitle(const QString &title)
{
    if (m_title != title) {
        m_title = title;
        QLabel *titleLabel = findChild<QLabel *>("glassCardTitle");
        if (titleLabel) {
            titleLabel->setText(title);
        }
    }
}

QString GlassCardWidget::title() const
{
    return m_title;
}

void GlassCardWidget::setContentWidget(QWidget *widget)
{
    if (m_contentWidget) {
        m_contentWidget->deleteLater();
    }

    ensureInternalLayout();

    m_contentWidget = widget;
    if (m_contentWidget) {
        layout()->addWidget(m_contentWidget);
    }
}

QWidget *GlassCardWidget::contentWidget() const
{
    return m_contentWidget;
}

void GlassCardWidget::applyTheme()
{
    ThemePalette p = ThemeManager::instance().currentPalette();

    QString styleSheet = QString(
        "GlassCardWidget {"
        "  background-color: transparent;"
        "  border: none;"
        "}"
        "QLabel#glassCardTitle {"
        "  color: %1;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  background: transparent;"
        "}"
    ).arg(p.glassTextPrimary);

    setStyleSheet(styleSheet);
    update();
}

void GlassCardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    ThemePalette p = ThemeManager::instance().currentPalette();

    int shadowOffset = 3;
    QRect contentRect = rect().adjusted(shadowOffset, shadowOffset, -shadowOffset, -shadowOffset);

    if (m_showShadow) {
        QRect shadowRect = rect().adjusted(2, 2, -2, -2);
        QPainterPath shadowPath = createRoundedRectPath(shadowRect, m_borderRadius);
        QColor shadowColor(p.glassShadow);
        shadowColor.setAlpha(m_shadowOpacity);
        painter.fillPath(shadowPath, shadowColor);
    }

    QPainterPath bgPath = createRoundedRectPath(contentRect, m_borderRadius);

    QColor bgColor(p.glassBackground);
    bgColor.setAlpha(m_backgroundOpacity);
    painter.fillPath(bgPath, bgColor);

    if (m_showHighlight) {
        painter.save();
        painter.setClipPath(bgPath);

        QLinearGradient highlightGradient(
            contentRect.topLeft(),
            contentRect.topLeft() + QPointF(0, contentRect.height() * 0.5)
        );

        QColor hlTopColor(p.glassHighlight);
        hlTopColor.setAlpha(m_highlightOpacity);
        QColor hlBottomColor(p.glassHighlight);
        hlBottomColor.setAlpha(0);

        highlightGradient.setColorAt(0.0, hlTopColor);
        highlightGradient.setColorAt(1.0, hlBottomColor);

        painter.fillPath(bgPath, highlightGradient);

        QRect topHighlightRect = contentRect.adjusted(2, 1, -2, -contentRect.height() + 3);
        QPainterPath topHighlightPath = createRoundedRectPath(topHighlightRect, m_borderRadius - 1);
        QColor topLineColor(p.glassHighlight);
        topLineColor.setAlpha(m_highlightOpacity * 2 / 3);
        QPen topLinePen(topLineColor, 1.5);
        painter.setPen(topLinePen);
        painter.drawPath(topHighlightPath);

        painter.restore();
    }

    QColor borderColor(p.glassBorder);
    borderColor.setAlpha(m_borderOpacity);
    QPen borderPen(borderColor, 1);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(bgPath);

    if (m_showHighlight) {
        QColor topBorderColor(p.glassHighlight);
        topBorderColor.setAlpha(m_borderOpacity * 2 / 3);
        QPen topBorderPen(topBorderColor, 1);

        QRectF cr = contentRect;
        qreal r = m_borderRadius;

        painter.setPen(topBorderPen);
        painter.setBrush(Qt::NoBrush);
        painter.drawLine(QPointF(cr.left() + r, cr.top()), QPointF(cr.right() - r, cr.top()));
    }
}

void GlassCardWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    update();
}

QPainterPath GlassCardWidget::createRoundedRectPath(const QRect &rect, int radius) const
{
    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);
    return path;
}

QColor GlassCardWidget::applyOpacity(const QColor &color, int opacity) const
{
    QColor result = color;
    result.setAlpha(opacity);
    return result;
}
