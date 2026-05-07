#include "settingstitlebar.h"

#include "theme/thememanager.h"
#include "utils/imageutils.h"

#include <QHBoxLayout>
#include <QLinearGradient>
#include <QMouseEvent>
#include <QPainter>
#include <QPixmap>

SettingsTitleBar::SettingsTitleBar(QWidget *parent)
    : QWidget(parent)
    , m_iconLabel(nullptr)
    , m_titleLabel(nullptr)
    , m_minimizeButton(nullptr)
    , m_maximizeButton(nullptr)
    , m_closeButton(nullptr)
    , m_dragging(false)
{
    setupUi();
    connectSignals();
    applyTheme();
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged, this, &SettingsTitleBar::applyTheme);
}

SettingsTitleBar::~SettingsTitleBar()
{
}

void SettingsTitleBar::setupUi()
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

    m_titleLabel = new QLabel(tr("Desktop Pet Settings"), this);
    m_titleLabel->setObjectName("titleLabel");
    layout->addWidget(m_titleLabel);
    layout->addStretch();

    m_minimizeButton = new QPushButton(this);
    m_minimizeButton->setFixedSize(46, 32);
    m_minimizeButton->setText("─");
    m_minimizeButton->setObjectName("titleBarButton");
    layout->addWidget(m_minimizeButton);

    m_maximizeButton = new QPushButton(this);
    m_maximizeButton->setFixedSize(46, 32);
    m_maximizeButton->setObjectName("titleBarButton");
    updateMaximizeButtonIcon();
    layout->addWidget(m_maximizeButton);

    m_closeButton = new QPushButton(this);
    m_closeButton->setFixedSize(46, 32);
    m_closeButton->setText("✕");
    m_closeButton->setObjectName("titleBarCloseButton");
    layout->addWidget(m_closeButton);
}

void SettingsTitleBar::connectSignals()
{
    connect(m_minimizeButton, &QPushButton::clicked, this, &SettingsTitleBar::onMinimizeClicked);
    connect(m_maximizeButton, &QPushButton::clicked, this, &SettingsTitleBar::onMaximizeClicked);
    connect(m_closeButton, &QPushButton::clicked, this, &SettingsTitleBar::onCloseClicked);
}

void SettingsTitleBar::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    QColor hoverColor(p.textSecondary);

    QString buttonStyle = QString(
        "QPushButton#titleBarButton {"
        "  background-color: transparent;"
        "  border: none;"
        "  border-radius: 4px;"
        "  color: %1;"
        "  font-size: 12px;"
        "}"
        "QPushButton#titleBarButton:hover {"
        "  background-color: rgba(%2, %3, %4, 30);"
        "}"
        "QPushButton#titleBarButton:pressed {"
        "  background-color: rgba(%2, %3, %4, 50);"
        "}"
    ).arg(p.textSecondary)
     .arg(hoverColor.red())
     .arg(hoverColor.green())
     .arg(hoverColor.blue());

    m_minimizeButton->setStyleSheet(buttonStyle);
    m_maximizeButton->setStyleSheet(buttonStyle);

    QString closeButtonStyle = QString(
        "QPushButton#titleBarCloseButton {"
        "  background-color: transparent;"
        "  border: none;"
        "  border-radius: 4px;"
        "  color: %1;"
        "  font-size: 12px;"
        "}"
        "QPushButton#titleBarCloseButton:hover {"
        "  background-color: rgba(232, 17, 35, 200);"
        "  color: white;"
        "}"
        "QPushButton#titleBarCloseButton:pressed {"
        "  background-color: rgba(232, 17, 35, 255);"
        "  color: white;"
        "}"
    ).arg(p.textSecondary);

    m_closeButton->setStyleSheet(closeButtonStyle);

    m_titleLabel->setStyleSheet(QString(
        "QLabel#titleLabel {"
        "  color: %1;"
        "  font-size: 13px;"
        "  font-weight: bold;"
        "  background: transparent;"
        "  border: none;"
        "}"
    ).arg(p.textPrimary));

    update();
}

void SettingsTitleBar::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    QColor bgColor(p.sidebarBackground);
    painter.fillRect(rect(), bgColor);

    QColor accentColor(p.accent);
    accentColor.setAlpha(15);
    QLinearGradient topGrad(0, 0, width(), height());
    topGrad.setColorAt(0.0, accentColor);
    topGrad.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.fillRect(rect(), topGrad);

    QColor separatorColor(p.border);
    separatorColor.setAlpha(100);
    painter.setPen(QPen(separatorColor, 1));
    painter.drawLine(0, height() - 1, width(), height() - 1);
}

void SettingsTitleBar::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = true;
        m_dragPosition = event->globalPos() - window()->frameGeometry().topLeft();
        event->accept();
    }
}

void SettingsTitleBar::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        if (window()->isMaximized()) {
            window()->showNormal();
            QPoint newTopLeft = event->globalPos() - QPoint(width() / 2, 10);
            window()->move(newTopLeft);
            m_dragPosition = event->globalPos() - window()->frameGeometry().topLeft();
        } else {
            window()->move(event->globalPos() - m_dragPosition);
        }
        event->accept();
    }
}

void SettingsTitleBar::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_dragging = false;
        event->accept();
    }
}

void SettingsTitleBar::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        onMaximizeClicked();
        event->accept();
    }
}

void SettingsTitleBar::onMinimizeClicked()
{
    if (window()) {
        window()->showMinimized();
    }
}

void SettingsTitleBar::onMaximizeClicked()
{
    if (window()) {
        if (window()->isMaximized()) {
            window()->showNormal();
        } else {
            window()->showMaximized();
        }
        updateMaximizeButtonIcon();
    }
}

void SettingsTitleBar::onCloseClicked()
{
    if (window()) {
        window()->close();
    }
}

void SettingsTitleBar::updateMaximizeButtonIcon()
{
    if (window() && window()->isMaximized()) {
        m_maximizeButton->setText("❐");
    } else {
        m_maximizeButton->setText("□");
    }
}
