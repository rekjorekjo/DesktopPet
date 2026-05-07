#include "softcardwidget.h"
#include "theme/thememanager.h"

#include <QPainter>
#include <QVBoxLayout>
#include <QLabel>
#include <QLinearGradient>
#include <QPropertyAnimation>
#include <QEasingCurve>

SoftCardWidget::SoftCardWidget(QWidget *parent)
    : QFrame(parent)
    , m_borderRadius(12)
    , m_backgroundOpacity(35)
    , m_highlightOpacity(25)
    , m_shadowOpacity(20)
    , m_borderOpacity(50)
    , m_showHighlight(true)
    , m_showShadow(true)
    , m_hoverProgress(0.0)
    , m_hoverAnimationEnabled(true)
    , m_hoverAnimation(nullptr)
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
    , m_hoverProgress(0.0)
    , m_hoverAnimationEnabled(true)
    , m_hoverAnimation(nullptr)
    , m_title(title)
    , m_contentWidget(nullptr)
{
    init();
    ensureInternalLayout();
}

void SoftCardWidget::init()
{
    setAttribute(Qt::WA_StyledBackground, true);
    setAttribute(Qt::WA_Hover, true);
    setAutoFillBackground(false);

    m_hoverAnimation = new QPropertyAnimation(this, "hoverProgress", this);
    m_hoverAnimation->setDuration(170);
    QEasingCurve enterCurve(QEasingCurve::OutBack);
    enterCurve.setOvershoot(0.45);
    m_hoverAnimation->setEasingCurve(enterCurve);

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

qreal SoftCardWidget::hoverProgress() const
{
    return m_hoverProgress;
}

void SoftCardWidget::setHoverProgress(qreal progress)
{
    progress = qBound<qreal>(0.0, progress, 1.0);
    if (!qFuzzyCompare(m_hoverProgress, progress)) {
        m_hoverProgress = progress;
        update();
    }
}

bool SoftCardWidget::hoverAnimationEnabled() const
{
    return m_hoverAnimationEnabled;
}

void SoftCardWidget::setHoverAnimationEnabled(bool enabled)
{
    if (m_hoverAnimationEnabled == enabled) {
        return;
    }

    m_hoverAnimationEnabled = enabled;
    if (!m_hoverAnimationEnabled) {
        if (m_hoverAnimation) {
            m_hoverAnimation->stop();
        }
        setHoverProgress(0.0);
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

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);
    painter.setRenderHint(QPainter::SmoothPixmapTransform, true);

    ThemeManager &theme = ThemeManager::instance();
    SoftCardGradientColors colors = theme.softCardGradientColors(false, m_hoverProgress);

    const qreal hoverLift = 2.0 * m_hoverProgress;
    QRectF cardRect = QRectF(rect()).adjusted(1.0, 2.5, -1.0, -3.5);
    cardRect.translate(0.0, -hoverLift);
    QPainterPath bgPath = createRoundedRectPath(cardRect, m_borderRadius);

    if (m_showShadow) {
        QColor shadowColor = colors.shadow;
        shadowColor.setAlpha(qBound(0, colors.shadowAlpha + m_shadowOpacity / 3 + qRound(10 * m_hoverProgress), 96));
        QRectF shadowRect = cardRect.adjusted(1.8,
                                              3.0 + 1.2 * m_hoverProgress,
                                              1.8,
                                              4.0 + 2.0 * m_hoverProgress);
        QPainterPath shadowPath = createRoundedRectPath(shadowRect, m_borderRadius);
        painter.fillPath(shadowPath, shadowColor);
    }

    QLinearGradient cardGrad(cardRect.topLeft(), cardRect.bottomRight());
    QColor topLeft = colors.topLeft;
    QColor topRight = colors.topRight;
    QColor bottomRight = colors.bottomRight;
    topLeft.setAlpha(colors.baseAlpha);
    topRight.setAlpha(colors.baseAlpha);
    bottomRight.setAlpha(colors.baseAlpha);
    cardGrad.setColorAt(0.0, topLeft);
    cardGrad.setColorAt(0.48, topRight);
    cardGrad.setColorAt(1.0, bottomRight);
    painter.fillPath(bgPath, cardGrad);

    if (m_showHighlight && colors.highlightAlpha > 0) {
        QLinearGradient highlightGrad(cardRect.topLeft(), QPointF(cardRect.left(), cardRect.top() + cardRect.height() * 0.42));
        QColor highlightColor = colors.highlight;
        int highlightAlpha = qMin(colors.highlightAlpha + qRound(4 * m_hoverProgress),
                                  m_highlightOpacity + qRound(14 * m_hoverProgress));
        highlightColor.setAlpha(highlightAlpha);
        highlightGrad.setColorAt(0.0, highlightColor);
        highlightGrad.setColorAt(1.0, QColor(255, 255, 255, 0));
        painter.fillPath(bgPath, highlightGrad);
    }

    QColor borderColor = colors.border;
    borderColor.setAlpha(qBound(0, colors.borderAlpha + m_borderOpacity / 5 + qRound(12 * m_hoverProgress), 235));
    QPen borderPen(borderColor, 1.0 + 0.15 * m_hoverProgress);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(bgPath);
}

void SoftCardWidget::resizeEvent(QResizeEvent *event)
{
    QFrame::resizeEvent(event);
    update();
}

void SoftCardWidget::enterEvent(QEnterEvent *event)
{
    QFrame::enterEvent(event);
    if (m_hoverAnimationEnabled) {
        animateHover(1.0, true);
    }
}

void SoftCardWidget::leaveEvent(QEvent *event)
{
    QFrame::leaveEvent(event);
    if (m_hoverAnimationEnabled) {
        animateHover(0.0, false);
    }
}

void SoftCardWidget::animateHover(qreal endValue, bool entering)
{
    if (!m_hoverAnimationEnabled) {
        setHoverProgress(0.0);
        return;
    }

    if (!m_hoverAnimation) {
        setHoverProgress(endValue);
        return;
    }

    m_hoverAnimation->stop();
    m_hoverAnimation->setDuration(entering ? 170 : 190);
    if (entering) {
        QEasingCurve curve(QEasingCurve::OutBack);
        curve.setOvershoot(0.45);
        m_hoverAnimation->setEasingCurve(curve);
    } else {
        m_hoverAnimation->setEasingCurve(QEasingCurve::OutCubic);
    }
    m_hoverAnimation->setStartValue(m_hoverProgress);
    m_hoverAnimation->setEndValue(qBound<qreal>(0.0, endValue, 1.0));
    m_hoverAnimation->start();
}

QPainterPath SoftCardWidget::createRoundedRectPath(const QRectF &rect, int radius) const
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
