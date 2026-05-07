#include "softcardwidget.h"
#include "core/appsettings.h"
#include "theme/thememanager.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <QLinearGradient>

SoftCardWidget::SoftCardWidget(QWidget *parent)
    : QFrame(parent)
    , m_borderRadius(12)
    , m_backgroundOpacity(35)
    , m_highlightOpacity(25)
    , m_shadowOpacity(20)
    , m_borderOpacity(50)
    , m_showHighlight(true)
    , m_showShadow(true)
    , m_contentWidget(nullptr)
{
    init();
}

SoftCardWidget::SoftCardWidget(const QString &title, QWidget *parent)
    : QFrame(parent)
    , m_borderRadius(12)
    , m_backgroundOpacity(35)
    , m_highlightOpacity(25)
    , m_shadowOpacity(20)
    , m_borderOpacity(50)
    , m_showHighlight(true)
    , m_showShadow(true)
    , m_title(title)
    , m_contentWidget(nullptr)
{
    init();
    ensureInternalLayout();
}

void SoftCardWidget::init()
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAutoFillBackground(false);

    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &SoftCardWidget::applyTheme);
}

void SoftCardWidget::ensureInternalLayout()
{
    if (layout()) {
        return;
    }

    QVBoxLayout *internalLayout = new QVBoxLayout(this);
    internalLayout->setContentsMargins(16, 16, 16, 16);
    internalLayout->setSpacing(8);

    if (!m_title.isEmpty()) {
        QLabel *titleLabel = new QLabel(m_title, this);
        titleLabel->setObjectName("softCardTitle");
        internalLayout->addWidget(titleLabel);
    }
}

int SoftCardWidget::borderRadius() const
{
    return m_borderRadius;
}

void SoftCardWidget::setBorderRadius(int radius)
{
    radius = qMax(0, radius);
    if (m_borderRadius != radius) {
        m_borderRadius = radius;
        update();
    }
}

int SoftCardWidget::backgroundOpacity() const
{
    return m_backgroundOpacity;
}

void SoftCardWidget::setBackgroundOpacity(int opacity)
{
    opacity = qBound(0, opacity, 255);
    if (m_backgroundOpacity != opacity) {
        m_backgroundOpacity = opacity;
        update();
    }
}

int SoftCardWidget::highlightOpacity() const
{
    return m_highlightOpacity;
}

void SoftCardWidget::setHighlightOpacity(int opacity)
{
    opacity = qBound(0, opacity, 255);
    if (m_highlightOpacity != opacity) {
        m_highlightOpacity = opacity;
        update();
    }
}

int SoftCardWidget::shadowOpacity() const
{
    return m_shadowOpacity;
}

void SoftCardWidget::setShadowOpacity(int opacity)
{
    opacity = qBound(0, opacity, 255);
    if (m_shadowOpacity != opacity) {
        m_shadowOpacity = opacity;
        update();
    }
}

int SoftCardWidget::borderOpacity() const
{
    return m_borderOpacity;
}

void SoftCardWidget::setBorderOpacity(int opacity)
{
    opacity = qBound(0, opacity, 255);
    if (m_borderOpacity != opacity) {
        m_borderOpacity = opacity;
        update();
    }
}

bool SoftCardWidget::showHighlight() const
{
    return m_showHighlight;
}

void SoftCardWidget::setShowHighlight(bool show)
{
    if (m_showHighlight != show) {
        m_showHighlight = show;
        update();
    }
}

bool SoftCardWidget::showShadow() const
{
    return m_showShadow;
}

void SoftCardWidget::setShowShadow(bool show)
{
    if (m_showShadow != show) {
        m_showShadow = show;
        update();
    }
}

void SoftCardWidget::setTitle(const QString &title)
{
    if (m_title == title) {
        return;
    }

    m_title = title;

    if (!title.isEmpty()) {
        QLabel *titleLabel = findChild<QLabel *>("softCardTitle");
        if (!titleLabel) {
            if (!layout()) {
                ensureInternalLayout();
            } else {
                titleLabel = new QLabel(title, this);
                titleLabel->setObjectName("softCardTitle");
                qobject_cast<QVBoxLayout *>(layout())->insertWidget(0, titleLabel);
            }
        } else {
            titleLabel->setText(title);
            titleLabel->show();
        }
    } else {
        QLabel *titleLabel = findChild<QLabel *>("softCardTitle");
        if (titleLabel) {
            titleLabel->clear();
            titleLabel->hide();
        }
    }
}

QString SoftCardWidget::title() const
{
    return m_title;
}

void SoftCardWidget::setContentWidget(QWidget *widget)
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

QWidget *SoftCardWidget::contentWidget() const
{
    return m_contentWidget;
}

void SoftCardWidget::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    QString styleSheet = QString(
        "SoftCardWidget {"
        "  background-color: transparent;"
        "  border: none;"
        "}"
        "QLabel#softCardTitle {"
        "  color: %1;"
        "  font-size: 14px;"
        "  font-weight: bold;"
        "  background: transparent;"
        "}"
    ).arg(p.subtitleText);

    setStyleSheet(styleSheet);
    update();
}

void SoftCardWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    constexpr int BaseGradientAlpha = 20;
    constexpr int BaseHighlightAlpha = 12;
    constexpr int DefaultStrength = 35;

    int strength = AppSettings::cardGradientStrength();
    double factor = static_cast<double>(strength) / DefaultStrength;

    int gradientAlpha = qBound(0, static_cast<int>(BaseGradientAlpha * factor), 50);
    int highlightAlpha = qBound(0, static_cast<int>(BaseHighlightAlpha * factor), 30);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    QPainterPath bgPath = createRoundedRectPath(rect(), m_borderRadius);

    if (m_showShadow) {
        QColor shadowColor(p.border);
        shadowColor.setAlpha(15);
        QPainterPath shadowPath = createRoundedRectPath(rect().adjusted(2, 2, 2, 2), m_borderRadius);
        painter.fillPath(shadowPath, shadowColor);
    }

    QColor bgColor(p.cardBackground);
    bgColor.setAlpha(240);
    painter.fillPath(bgPath, bgColor);

    if (gradientAlpha > 0) {
        QLinearGradient cardGrad(0, 0, rect().width(), rect().height());
        QColor gradStartColor(p.accentSoft);
        gradStartColor.setAlpha(gradientAlpha);
        QColor gradEndColor(p.cardBackground);
        gradEndColor.setAlpha(0);
        cardGrad.setColorAt(0.0, gradStartColor);
        cardGrad.setColorAt(0.5, gradEndColor);
        painter.fillPath(bgPath, cardGrad);
    }

    if (m_showHighlight && highlightAlpha > 0) {
        QLinearGradient highlightGrad(0, 0, 0, rect().height() * 0.4);
        QColor highlightColor(p.accent);
        highlightColor.setAlpha(highlightAlpha);
        highlightGrad.setColorAt(0.0, highlightColor);
        highlightGrad.setColorAt(1.0, QColor(255, 255, 255, 0));
        painter.fillPath(bgPath, highlightGrad);
    }

    QColor borderColor(p.border);
    borderColor.setAlpha(150);
    QPen borderPen(borderColor, 1);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(bgPath);
}

void SoftCardWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    update();
}

QPainterPath SoftCardWidget::createRoundedRectPath(const QRect &rect, int radius) const
{
    QPainterPath path;
    path.addRoundedRect(rect, radius, radius);
    return path;
}

QColor SoftCardWidget::applyOpacity(const QColor &color, int opacity) const
{
    QColor result = color;
    result.setAlpha(opacity);
    return result;
}
