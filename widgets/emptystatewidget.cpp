#include "emptystatewidget.h"

#include "theme/thememanager.h"

#include <QLabel>
#include <QPainter>
#include <QPainterPath>
#include <QVBoxLayout>

EmptyStateWidget::EmptyStateWidget(QWidget *parent)
    : QWidget(parent)
    , m_titleLabel(nullptr)
    , m_descriptionLabel(nullptr)
    , m_contentLabel(nullptr)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    setAttribute(Qt::WA_TransparentForMouseEvents, true);
    setupUi();
    applyTheme();
}

void EmptyStateWidget::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->setSpacing(8);
    mainLayout->addStretch();

    m_titleLabel = new QLabel(this);
    m_titleLabel->setAlignment(Qt::AlignCenter);
    m_titleLabel->setWordWrap(true);
    QFont titleFont = m_titleLabel->font();
    titleFont.setPointSize(12);
    titleFont.setBold(true);
    m_titleLabel->setFont(titleFont);

    m_descriptionLabel = new QLabel(this);
    m_descriptionLabel->setAlignment(Qt::AlignCenter);
    m_descriptionLabel->setWordWrap(true);

    m_contentLabel = new QLabel(this);
    m_contentLabel->setAlignment(Qt::AlignCenter);
    m_contentLabel->setWordWrap(true);
    QFont contentFont = m_contentLabel->font();
    contentFont.setPointSize(9);
    m_contentLabel->setFont(contentFont);

    mainLayout->addWidget(m_titleLabel, 0, Qt::AlignCenter);
    mainLayout->addWidget(m_descriptionLabel, 0, Qt::AlignCenter);
    mainLayout->addWidget(m_contentLabel, 0, Qt::AlignCenter);
    mainLayout->addStretch();
}

void EmptyStateWidget::setTitle(const QString &title)
{
    if (m_titleLabel) {
        m_titleLabel->setText(title);
        m_titleLabel->setVisible(!title.isEmpty());
    }
}

void EmptyStateWidget::setDescription(const QString &description)
{
    if (m_descriptionLabel) {
        m_descriptionLabel->setText(description);
        m_descriptionLabel->setVisible(!description.isEmpty());
    }
}

void EmptyStateWidget::setContent(const QString &content)
{
    if (m_contentLabel) {
        m_contentLabel->setText(content);
        m_contentLabel->setVisible(!content.isEmpty());
    }
}

void EmptyStateWidget::clear()
{
    if (m_titleLabel) {
        m_titleLabel->clear();
        m_titleLabel->hide();
    }
    if (m_descriptionLabel) {
        m_descriptionLabel->clear();
        m_descriptionLabel->hide();
    }
    if (m_contentLabel) {
        m_contentLabel->clear();
        m_contentLabel->hide();
    }
}

void EmptyStateWidget::applyTheme()
{
    ThemePalette p = ThemeManager::instance().currentPalette();

    QString titleStyle = QString("color: %1; border: none; background: transparent;")
                              .arg(p.textPrimary);
    QString descStyle = QString("color: %1; border: none; background: transparent;")
                             .arg(p.textSecondary);
    QString contentStyle = QString("color: %1; border: none; background: transparent;")
                                .arg(p.textSecondary);

    if (m_titleLabel) {
        m_titleLabel->setStyleSheet(titleStyle);
    }
    if (m_descriptionLabel) {
        m_descriptionLabel->setStyleSheet(descStyle);
    }
    if (m_contentLabel) {
        m_contentLabel->setStyleSheet(contentStyle);
    }
}

void EmptyStateWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    ThemePalette p = ThemeManager::instance().currentPalette();

    constexpr int borderRadius = 8;

    QPainterPath path;
    path.addRoundedRect(rect(), borderRadius, borderRadius);

    QColor bgColor(p.inputBackground);
    painter.fillPath(path, bgColor);

    QColor borderColor(p.border);
    QPen borderPen(borderColor, 1);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
}
