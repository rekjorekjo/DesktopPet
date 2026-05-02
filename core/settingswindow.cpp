#include "settingswindow.h"

#include "pages/aboutpage.h"
#include "pages/apiconfigpage.h"
#include "pages/defaultpetpage.h"
#include "pages/displaysettingspage.h"
#include "theme/thememanager.h"

#include <QHBoxLayout>
#include <QVBoxLayout>

SettingsWindow::SettingsWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
    , m_sidebar(nullptr)
    , m_stackedWidget(nullptr)
    , m_defaultPetPage(nullptr)
    , m_apiConfigPage(nullptr)
    , m_displaySettingsPage(nullptr)
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

    m_centralWidget = new QWidget(this);
    m_centralWidget->setObjectName("centralWidget");

    QHBoxLayout *mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);

    setupSidebar();
    setupPages();

    mainLayout->addWidget(m_sidebar);
    mainLayout->addWidget(m_stackedWidget, 1);

    setCentralWidget(m_centralWidget);
    applyTheme();
}

void SettingsWindow::setupSidebar()
{
    m_sidebar = new QListWidget(this);
    m_sidebar->setFixedWidth(200);
    m_sidebar->setFocusPolicy(Qt::NoFocus);
    m_sidebar->addItem(tr("默认宠物"));
    m_sidebar->addItem(tr("API配置"));
    m_sidebar->addItem(tr("显示设置"));
    m_sidebar->addItem(tr("关于"));
    m_sidebar->setCurrentRow(0);
}

void SettingsWindow::setupPages()
{
    m_stackedWidget = new QStackedWidget(this);

    m_defaultPetPage = new DefaultPetPage(this);
    m_apiConfigPage = new ApiConfigPage(this);
    m_displaySettingsPage = new DisplaySettingsPage(this);
    m_aboutPage = new AboutPage(this);

    m_stackedWidget->addWidget(m_defaultPetPage);
    m_stackedWidget->addWidget(m_apiConfigPage);
    m_stackedWidget->addWidget(m_displaySettingsPage);
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
}

void SettingsWindow::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();

    m_centralWidget->setStyleSheet(QString("QWidget#centralWidget { background-color: %1; }")
                                       .arg(theme.pageBackgroundColor()));

    m_sidebar->setStyleSheet(theme.sidebarStyleSheet());

    if (m_defaultPetPage) {
        m_defaultPetPage->refreshTheme();
    }
    if (m_apiConfigPage) {
        m_apiConfigPage->refreshTheme();
    }
    if (m_displaySettingsPage) {
        m_displaySettingsPage->refreshTheme();
    }
    if (m_aboutPage) {
        m_aboutPage->refreshTheme();
    }
}
