#include "glasscardwidget.h"
#include "theme/thememanager.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>

GlassCardWidget::GlassCardWidget(QWidget *parent)
    : QFrame(parent)
    , m_borderRadius(12)
    , m_backgroundOpacity(40)
    , m_showHighlight(true)
    , m_showShadow(true)
    , m_contentWidget(nullptr)
{
    init();
}

GlassCardWidget::GlassCardWidget(const QString &title, QWidget *parent)
    : QFrame(parent)
    , m_borderRadius(12)
    , m_backgroundOpacity(40)
    , m_showHighlight(true)
    , m_showShadow(true)
    , m_title(title)
    , m_contentWidget(nullptr)
{
    init();
}

void GlassCardWidget::init()
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(16, 16, 16, 16);
    mainLayout->setSpacing(8);

    if (!m_title.isEmpty()) {
        QLabel *titleLabel = new QLabel(m_title, this);
        titleLabel->setObjectName("glassCardTitle");
        mainLayout->addWidget(titleLabel);
    }

    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &GlassCardWidget::applyTheme);
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

    QRect contentRect = rect().adjusted(4, 4, -4, -4);

    if (m_showShadow) {
        QPainterPath shadowPath = createRoundedRectPath(rect(), m_borderRadius);
        QColor shadowColor = QColor(p.glassShadow);
        shadowColor.setAlpha(30);
        painter.fillPath(shadowPath, shadowColor);
    }

    QPainterPath bgPath = createRoundedRectPath(contentRect, m_borderRadius);
    QColor bgColor = QColor(p.glassBackground);
    bgColor.setAlpha(m_backgroundOpacity);
    painter.fillPath(bgPath, bgColor);

    if (m_showHighlight) {
        QRect highlightRect = contentRect.adjusted(0, 0, 0, -contentRect.height() + 2);
        QPainterPath highlightPath = createRoundedRectPath(highlightRect, m_borderRadius);
        QColor highlightColor = QColor(p.glassHighlight);
        highlightColor.setAlpha(20);
        painter.setClipPath(bgPath);
        painter.fillPath(highlightPath, highlightColor);
        painter.setClipping(false);
    }

    QPen borderPen(QColor(p.glassBorder));
    borderPen.setWidth(1);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(bgPath);
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
