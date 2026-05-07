#include "settingswindow.h"

#include "pages/aboutpage.h"
#include "pages/actionsettingspage.h"
#include "pages/apiconfigpage.h"
#include "pages/personalizationpage.h"
#include "pages/petmanagepage.h"
#include "theme/thememanager.h"

#include <QHBoxLayout>
#include <QLinearGradient>
#include <QPainter>
#include <QVBoxLayout>

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

    QColor accentColor(p.accent);
    accentColor.setAlpha(12);
    QLinearGradient topGrad(0, 0, width(), height() * 0.4);
    topGrad.setColorAt(0.0, accentColor);
    topGrad.setColorAt(1.0, QColor(0, 0, 0, 0));
    painter.fillRect(rect(), topGrad);

    QColor shadowColor(p.border);
    shadowColor.setAlpha(8);
    QLinearGradient bottomGrad(0, height() * 0.7, 0, height());
    bottomGrad.setColorAt(0.0, QColor(0, 0, 0, 0));
    bottomGrad.setColorAt(1.0, shadowColor);
    painter.fillRect(rect(), bottomGrad);
}

SettingsWindow::SettingsWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_centralWidget(nullptr)
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

    m_centralWidget = new SoftGradientBackgroundWidget(this);

    QHBoxLayout *mainLayout = new QHBoxLayout(m_centralWidget);
    mainLayout->setContentsMargins(12, 12, 12, 12);
    mainLayout->setSpacing(12);

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
}

void SettingsWindow::applyTheme()
{
    ThemeManager &theme = ThemeManager::instance();

    m_centralWidget->update();

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
