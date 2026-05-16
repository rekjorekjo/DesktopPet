#include "settingswindow.h"

#include "core/appsettings.h"
#include "pages/aboutpage.h"
#include "pages/actionsettingspage.h"
#include "pages/apiconfigpage.h"
#include "pages/logpage.h"
#include "pages/personalizationpage.h"
#include "pages/petmanagepage.h"
#include "theme/thememanager.h"
#include "widgets/settingstitlebar.h"
#include "widgets/softcardwidget.h"

#include <QHBoxLayout>
#include <QLinearGradient>
#include <QPainter>
#include <QVBoxLayout>
#include <QEvent>

SoftGradientBackgroundWidget::SoftGradientBackgroundWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("centralWidget");
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
}

void SoftGradientBackgroundWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    QColor baseColor(p.pageBackground);
    painter.fillRect(rect(), baseColor);
}

SidebarContainerWidget::SidebarContainerWidget(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("sidebarContainer");
    setAttribute(Qt::WA_StyledBackground, false);
    setAutoFillBackground(false);
}

void SidebarContainerWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    ThemeManager &theme = ThemeManager::instance();
    ThemePalette p = theme.currentPalette();

    constexpr int borderRadius = 12;

    QPainterPath path;
    path.addRoundedRect(rect(), borderRadius, borderRadius);
    painter.setClipPath(path);

    QColor bgColor(p.sidebarBackground);
    painter.fillPath(path, bgColor);

    QColor borderColor(p.border);
    borderColor.setAlpha(120);
    QPen borderPen(borderColor, 1);
    painter.setPen(borderPen);
    painter.setBrush(Qt::NoBrush);
    painter.drawPath(path);
}

SettingsWindow::SettingsWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_titleBar(nullptr)
    , m_sidebarContainer(nullptr)
    , m_sidebar(nullptr)
    , m_stackedWidget(nullptr)
    , m_petManagePage(nullptr)
    , m_actionSettingsPage(nullptr)
    , m_apiConfigPage(nullptr)
    , m_logPage(nullptr)
    , m_personalizationPage(nullptr)
    , m_aboutPage(nullptr)
{
    setupUi();
    connectSignals();
}

SettingsWindow::~SettingsWindow() {}

void SettingsWindow::setupUi()
{
    setWindowTitle("Desktop Pet Settings");
    resize(1000, 650);
    setMinimumSize(800, 520);

    setWindowFlags(Qt::FramelessWindowHint | Qt::WindowMinMaxButtonsHint);
    setAttribute(Qt::WA_TranslucentBackground, false);

    m_centralWidget = new SoftGradientBackgroundWidget(this);

    QVBoxLayout *rootLayout = new QVBoxLayout(m_centralWidget);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(0);

    m_titleBar = new SettingsTitleBar(this);
    rootLayout->addWidget(m_titleBar);

    QWidget *contentWidget = new QWidget(this);
    contentWidget->setObjectName("contentWidget");
    QHBoxLayout *contentLayout = new QHBoxLayout(contentWidget);
    contentLayout->setContentsMargins(12, 12, 12, 12);
    contentLayout->setSpacing(12);

    setupSidebar();
    setupPages();

    contentLayout->addWidget(m_sidebarContainer);
    contentLayout->addWidget(m_stackedWidget, 1);

    rootLayout->addWidget(contentWidget, 1);

    setCentralWidget(m_centralWidget);
    applyTheme();
}

void SettingsWindow::setupSidebar()
{
    m_sidebarContainer = new SidebarContainerWidget(this);
    m_sidebarContainer->setFixedWidth(200);

    QVBoxLayout *containerLayout = new QVBoxLayout(m_sidebarContainer);
    containerLayout->setContentsMargins(4, 4, 4, 4);
    containerLayout->setSpacing(0);

    m_sidebar = new QListWidget(m_sidebarContainer);
    m_sidebar->setFocusPolicy(Qt::NoFocus);
    m_sidebar->addItem(tr("宠物管理"));
    m_sidebar->addItem(tr("动作设置"));
    m_sidebar->addItem(tr("聊天设置"));
    m_sidebar->addItem(tr("日志"));
    m_sidebar->addItem(tr("个性化"));
    m_sidebar->addItem(tr("关于"));
    m_sidebar->setCurrentRow(0);
    m_sidebar->setStyleSheet("QListWidget { background-color: transparent; border: none; outline: none; }");

    containerLayout->addWidget(m_sidebar);
}

void SettingsWindow::setupPages()
{
    m_stackedWidget = new QStackedWidget(this);

    m_petManagePage = new PetManagePage(this);
    m_actionSettingsPage = new ActionSettingsPage(this);
    m_apiConfigPage = new ApiConfigPage(this);
    m_logPage = new LogPage(this);
    m_personalizationPage = new PersonalizationPage(this);
    m_aboutPage = new AboutPage(this);

    m_stackedWidget->addWidget(m_petManagePage);
    m_stackedWidget->addWidget(m_actionSettingsPage);
    m_stackedWidget->addWidget(m_apiConfigPage);
    m_stackedWidget->addWidget(m_logPage);
    m_stackedWidget->addWidget(m_personalizationPage);
    m_stackedWidget->addWidget(m_aboutPage);
}

void SettingsWindow::connectSignals()
{
    connect(m_sidebar,
            &QListWidget::currentRowChanged,
            m_stackedWidget,
            &QStackedWidget::setCurrentIndex);

    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &SettingsWindow::applyTheme);

    connect(m_petManagePage, &PetManagePage::startPetRequested,
            this, &SettingsWindow::startPetRequested);

    connect(m_petManagePage, &PetManagePage::pausePetRequested,
            this, &SettingsWindow::pausePetRequested);

    connect(m_actionSettingsPage, &ActionSettingsPage::applyConfigRequested,
            this, [this]() {
        if (m_petManagePage) {
            m_petManagePage->reloadPetInfo();
        }
        emit actionConfigApplied();
    });

    connect(m_petManagePage, &PetManagePage::applyConfigRequested, this, [this]() {
        if (m_actionSettingsPage) {
            m_actionSettingsPage->reloadData();
        }
        emit applyPetConfigRequested();
    });

    connect(m_personalizationPage, &PersonalizationPage::petOpacityChanged,
            this, &SettingsWindow::petOpacityChanged);

    connect(m_personalizationPage, &PersonalizationPage::baseMoveSpeedChanged,
            this, &SettingsWindow::baseMoveSpeedChanged);

    connect(m_personalizationPage, &PersonalizationPage::cardGradientStrengthChanged,
            this, [this](int) {
                const QList<SoftCardWidget *> cards = findChildren<SoftCardWidget *>();
                for (SoftCardWidget *card : cards) {
                    card->update();
                }
            });

    connect(m_personalizationPage, &PersonalizationPage::randomCardGradientChanged,
            this, [this](bool) {
                const QList<SoftCardWidget *> cards = findChildren<SoftCardWidget *>();
                for (SoftCardWidget *card : cards) {
                    card->randomizeGradient();
                }
            });
}

// 宠物管理页的运行状态必须来自 PetWidget 的真实启动结果，
// 不能在点击"开始"时乐观设置为运行中。
// 这样可以避免播放失败时 UI 仍显示"运行中"。
void SettingsWindow::onPetStarted()
{
    if (m_petManagePage) {
        m_petManagePage->onPetStarted();
    }
}

void SettingsWindow::onPetPaused()
{
    if (m_petManagePage) {
        m_petManagePage->onPetPaused();
    }
}

void SettingsWindow::onPetStartFailed(const QString &message)
{
    if (m_petManagePage) {
        m_petManagePage->onPetStartFailed(message);
    }
}

void SettingsWindow::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();

    m_centralWidget->update();

    if (m_titleBar) {
        m_titleBar->applyTheme();
    }

    m_sidebarContainer->update();

    ThemePalette p = theme.currentPalette();
    QString sidebarItemStyle = QString(
        "QListWidget { background-color: transparent; border: none; outline: none; }"
        "QListWidget::item { padding: 12px 16px; border: none; color: %1; border-radius: 8px; margin: 2px 4px; }"
        "QListWidget::item:selected { background-color: %2; color: %3; }"
        "QListWidget::item:hover { background-color: %4; color: %5; }"
    ).arg(p.sidebarText, p.sidebarSelectedBg, p.sidebarSelectedText, p.sidebarHoverBg, p.sidebarHoverText);
    m_sidebar->setStyleSheet(sidebarItemStyle);

    m_stackedWidget->setStyleSheet("QStackedWidget { background-color: transparent; }");

    if (m_petManagePage) {
        m_petManagePage->refreshTheme();
    }
    if (m_actionSettingsPage) {
        m_actionSettingsPage->refreshTheme();
    }
    if (m_apiConfigPage) {
        m_apiConfigPage->refreshTheme();
    }
    if (m_logPage) {
        m_logPage->refreshTheme();
    }
    if (m_personalizationPage) {
        m_personalizationPage->refreshTheme();
    }
    if (m_aboutPage) {
        m_aboutPage->refreshTheme();
    }
}

void SettingsWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        if (m_titleBar) {
            m_titleBar->applyTheme();
        }
    }
    QMainWindow::changeEvent(event);
}
