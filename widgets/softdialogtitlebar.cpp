#include "softdialogtitlebar.h"
#include "theme/thememanager.h"
#include "utils/imageutils.h"

#include <QHBoxLayout>
#include <QMouseEvent>
#include <QPainter>

SoftDialogTitleBar::SoftDialogTitleBar(QWidget *parent)
    : QWidget(parent)
    , m_iconLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_closeButton(nullptr)
    , m_dragging(false)
{
    setupUi();
    connectSignals();
    applyTheme();
}

SoftDialogTitleBar::SoftDialogTitleBar(const QString &title, QWidget *parent)
    : QWidget(parent)
    , m_iconLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_closeButton(nullptr)
    , m_dragging(false)
    , m_title(title)
{
    setupUi();
    connectSignals();
    applyTheme();
}

SoftDialogTitleBar::~SoftDialogTitleBar() {}

void SoftDialogTitleBar::setTitle(const QString &title)
{
    m_title = title;
    if (m_titleLabel) {
        m_titleLabel->setText(title);
    }
}

QString SoftDialogTitleBar::title() const
{
    return m_title;
}

void SoftDialogTitleBar::setupUi()
{
    setFixedHeight(40);
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setContentsMargins(12, 0, 8, 0);
    layout->setSpacing(8);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(20, 20);
    QPixmap appIcon(":/icons/app_icon.png");
    if (!appIcon.isNull()) {
        QPixmap roundedIcon = ImageUtils::roundedPixmap(appIcon, 20, 4);
        m_iconLabel->setPixmap(roundedIcon);
    }
    layout->addWidget(m_iconLabel);

    m_titleLabel = new QLabel(m_title.isEmpty() ? tr("Dialog") : m_title, this);
    m_titleLabel->setObjectName("dialogTitleLabel");
    layout->addWidget(m_titleLabel);
    layout->addStretch();

    m_closeButton = new QPushButton(this);
    m_closeButton->setFixedSize(46, 32);
    m_closeButton->setText("✕");
    m_closeButton->setObjectName("dialogTitleBarCloseButton");
    layout->addWidget(m_closeButton);
}

void SoftDialogTitleBar::connectSignals()
{
    connect(m_closeButton, &QPushButton::clicked, this, &SoftDialogTitleBar::onCloseClicked);
}

void SoftDialogTitleBar::onCloseClicked()
{
    if (window()) {
        window()->close();
    }
}

void SoftDialogTitleBar::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    QColor hoverColor(p.textSecondary);

    QString closeButtonStyle = QString(
        "QPushButton#dialogTitleBarCloseButton {"
        "  background-color: transparent;"
        "  border: none;"
        "  border-radius: 4px;"
        "  color: %1;"
        "  font-size: 12px;"
        "}"
        "QPushButton#dialogTitleBarCloseButton:hover {"
        "  background-color: rgba(232, 17, 35, 200);"
        "  color: white;"
        "}"
        "QPushButton#dialogTitleBarCloseButton:pressed {"
        "  background-color: rgba(232, 17, 35, 255);"
        "  color: white;"
        "}"
    ).arg(p.textSecondary);

    m_closeButton->setStyleSheet(closeButtonStyle);

    m_titleLabel->setStyleSheet(QString(
        "QLabel#dialogTitleLabel {"
        "  color: %1;"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "  background: transparent;"
        "  border: none;"
        "}"
    ).arg(p.textPrimary));

    update();
}

void SoftDialogTitleBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    QColor bgColor(p.cardBackground);
    painter.fillRect(rect(), bgColor);

    QColor separatorColor(p.border);
    separatorColor.setAlpha(100);
    painter.setPen(separatorColor);
    painter.drawLine(0, height() - 1, width(), height() - 1);
}

void SoftDialogTitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPos() - window()->frameGeometry().topLeft();
        event->accept();
    }
}

void SoftDialogTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        window()->move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void SoftDialogTitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
}
