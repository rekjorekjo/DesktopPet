#ifndef SOFTCARDWIDGET_H
#define SOFTCARDWIDGET_H

#include <QFrame>
#include <QColor>
#include <QPainterPath>
#include <QRectF>

class QEnterEvent;
class QEvent;
class QPropertyAnimation;

class SoftCardWidget : public QFrame
{
    Q_OBJECT
    Q_PROPERTY(int borderRadius READ borderRadius WRITE setBorderRadius)
    Q_PROPERTY(int backgroundOpacity READ backgroundOpacity WRITE setBackgroundOpacity)
    Q_PROPERTY(int highlightOpacity READ highlightOpacity WRITE setHighlightOpacity)
    Q_PROPERTY(int shadowOpacity READ shadowOpacity WRITE setShadowOpacity)
    Q_PROPERTY(int borderOpacity READ borderOpacity WRITE setBorderOpacity)
    Q_PROPERTY(bool showHighlight READ showHighlight WRITE setShowHighlight)
    Q_PROPERTY(bool showShadow READ showShadow WRITE setShowShadow)
    Q_PROPERTY(qreal hoverProgress READ hoverProgress WRITE setHoverProgress)
    Q_PROPERTY(bool hoverAnimationEnabled READ hoverAnimationEnabled WRITE setHoverAnimationEnabled)

public:
    explicit SoftCardWidget(QWidget *parent = nullptr);
    explicit SoftCardWidget(const QString &title, QWidget *parent = nullptr);

    int borderRadius() const;
    void setBorderRadius(int radius);

    int backgroundOpacity() const;
    void setBackgroundOpacity(int opacity);

    int highlightOpacity() const;
    void setHighlightOpacity(int opacity);

    int shadowOpacity() const;
    void setShadowOpacity(int opacity);

    int borderOpacity() const;
    void setBorderOpacity(int opacity);

    bool showHighlight() const;
    void setShowHighlight(bool show);

    bool showShadow() const;
    void setShowShadow(bool show);

    qreal hoverProgress() const;
    void setHoverProgress(qreal progress);

    bool hoverAnimationEnabled() const;
    void setHoverAnimationEnabled(bool enabled);

    void setTitle(const QString &title);
    QString title() const;

    void setContentWidget(QWidget *widget);
    QWidget *contentWidget() const;

public slots:
    void applyTheme();
    void randomizeGradient();

protected:
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void enterEvent(QEnterEvent *event) override;
    void leaveEvent(QEvent *event) override;

private:
    void init();
    void ensureInternalLayout();
    void animateHover(qreal endValue, bool entering);
    QPainterPath createRoundedRectPath(const QRectF &rect, int radius) const;
    QColor applyOpacity(const QColor &color, int opacity) const;
    void randomizeGradientSeed();

private:
    int m_borderRadius;
    int m_backgroundOpacity;
    int m_highlightOpacity;
    int m_shadowOpacity;
    int m_borderOpacity;
    bool m_showHighlight;
    bool m_showShadow;
    qreal m_hoverProgress;
    bool m_hoverAnimationEnabled;
    QPropertyAnimation *m_hoverAnimation;
    QString m_title;
    QWidget *m_contentWidget;

    qreal m_gradientStartX;
    qreal m_gradientStartY;
    qreal m_gradientEndX;
    qreal m_gradientEndY;
    qreal m_highlightOffset;
};

#endif // SOFTCARDWIDGET_H
