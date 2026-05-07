#include "settingswindow.h"

#include "core/appsettings.h"
#include "pages/aboutpage.h"
#include "pages/actionsettingspage.h"
#include "pages/apiconfigpage.h"
#include "pages/personalizationpage.h"
#include "pages/petmanagepage.h"
#include "theme/thememanager.h"
#include "widgets/settingstitlebar.h"

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

SettingsWindow::SettingsWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_titleBar(nullptr)
    , m_sidebar(nullptr)
    , m_stackedWidget(nullptr)
    , m_petManagePage(nullptr)
    , m_actionSettingsPage(nullptr)
    , m_apiConfigPage(nullptr)
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

    contentLayout->addWidget(m_sidebar);
    contentLayout->addWidget(m_stackedWidget, 1);

    rootLayout->addWidget(contentWidget, 1);

    setCentralWidget(m_centralWidget);
    applyTheme();
}

void SettingsWindow::setupSidebar()
{
    m_sidebar = new QListWidget(this);
    m_sidebar->setFixedWidth(200);
    m_sidebar->setFocusPolicy(Qt::NoFocus);
    m_sidebar->addItem(tr("宠物管理"));
    m_sidebar->addItem(tr("动作设置"));
    m_sidebar->addItem(tr("API配置"));
    m_sidebar->addItem(tr("个性化"));
    m_sidebar->addItem(tr("关于"));
    m_sidebar->setCurrentRow(0);
    m_sidebar->setContentsMargins(8, 8, 8, 8);
}

void SettingsWindow::setupPages()
{
    m_stackedWidget = new QStackedWidget(this);

    m_petManagePage = new PetManagePage(this);
    m_actionSettingsPage = new ActionSettingsPage(this);
    m_apiConfigPage = new ApiConfigPage(this);
    m_personalizationPage = new PersonalizationPage(this);
    m_aboutPage = new AboutPage(this);

    m_stackedWidget->addWidget(m_petManagePage);
    m_stackedWidget->addWidget(m_actionSettingsPage);
    m_stackedWidget->addWidget(m_apiConfigPage);
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
            this, &SettingsWindow::applyPetConfigRequested);

    connect(m_petManagePage, &PetManagePage::applyConfigRequested, this, [this]() {
        if (m_actionSettingsPage) {
            m_actionSettingsPage->reloadData();
        }
        emit applyPetConfigRequested();
    });

    connect(m_personalizationPage, &PersonalizationPage::petOpacityChanged,
            this, &SettingsWindow::petOpacityChanged);

    connect(m_personalizationPage, &PersonalizationPage::cardGradientStrengthChanged,
            this, [this](int) {
                if (m_petManagePage) {
                    m_petManagePage->refreshTheme();
                }
                if (m_actionSettingsPage) {
                    m_actionSettingsPage->refreshTheme();
                }
                if (m_apiConfigPage) {
                    m_apiConfigPage->refreshTheme();
                }
                if (m_personalizationPage) {
                    m_personalizationPage->refreshTheme();
                }
                if (m_aboutPage) {
                    m_aboutPage->refreshTheme();
                }
            });
}

void SettingsWindow::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();

    m_centralWidget->update();

    if (m_titleBar) {
        m_titleBar->applyTheme();
    }

    m_sidebar->setStyleSheet(theme.softSidebarStyleSheet(8, 35));

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
