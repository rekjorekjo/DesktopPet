#include "traymanager.h"

#include "theme/thememanager.h"

#include <QAction>
#include <QApplication>
#include <QColor>
#include <QFont>
#include <QIcon>
#include <QMenu>
#include <QPainter>
#include <QPixmap>
#include <QRect>
#include <QSystemTrayIcon>

TrayManager::TrayManager(QObject *parent)
    : QObject(parent)
    , m_trayIcon(nullptr)
    , m_menu(nullptr)
{
    m_trayIcon = new QSystemTrayIcon(this);
    m_trayIcon->setToolTip(tr("Desktop Pet"));

    QIcon icon(":/icons/app_icon.png");
    if (icon.isNull()) {
        icon = QApplication::windowIcon();
    }
    if (icon.isNull()) {
        icon = QIcon::fromTheme("application-x-executable");
    }
    if (icon.isNull()) {
        QPixmap pixmap(32, 32);
        pixmap.fill(Qt::transparent);
        QPainter painter(&pixmap);
        painter.setRenderHint(QPainter::Antialiasing);
        painter.setBrush(QColor(100, 150, 200));
        painter.setPen(Qt::NoPen);
        painter.drawEllipse(0, 0, 32, 32);
        painter.setPen(Qt::white);
        QFont font = painter.font();
        font.setPointSize(14);
        font.setBold(true);
        painter.setFont(font);
        painter.drawText(QRect(0, 0, 32, 32), Qt::AlignCenter, tr("宠"));
        icon = QIcon(pixmap);
    }
    m_trayIcon->setIcon(icon);

    createMenu();

    connect(m_trayIcon, &QSystemTrayIcon::activated,
            this, &TrayManager::onTrayActivated);
    connect(&ThemeManager::instance(), &ThemeManager::themeChanged,
            this, &TrayManager::applyTheme);
}

TrayManager::~TrayManager()
{
    if (m_trayIcon) {
        m_trayIcon->hide();
        m_trayIcon->setContextMenu(nullptr);
    }
    delete m_menu;
    m_menu = nullptr;
}

void TrayManager::show()
{
    if (m_trayIcon) {
        m_trayIcon->show();
    }
}

void TrayManager::createMenu()
{
    m_menu = new QMenu();

    ThemeManager &theme = ThemeManager::instance();
    m_menu->setStyleSheet(theme.menuStyleSheet());

    QAction *showAction = m_menu->addAction(tr("显示桌宠"));
    QAction *hideAction = m_menu->addAction(tr("隐藏桌宠"));
    m_menu->addSeparator();
    QAction *startAction = m_menu->addAction(tr("开始"));
    QAction *pauseAction = m_menu->addAction(tr("暂停"));
    m_menu->addSeparator();

    QMenu *emotionMenu = m_menu->addMenu(tr("测试情绪"));
    emotionMenu->setStyleSheet(theme.menuStyleSheet());

    QAction *happyAction = emotionMenu->addAction("happy");
    QAction *sadAction = emotionMenu->addAction("sad");
    QAction *angryAction = emotionMenu->addAction("angry");
    QAction *confusedAction = emotionMenu->addAction("confused");
    QAction *comfortAction = emotionMenu->addAction("comfort");

    connect(happyAction, &QAction::triggered, this, [this]() { emit emotionRequested("happy"); });
    connect(sadAction, &QAction::triggered, this, [this]() { emit emotionRequested("sad"); });
    connect(angryAction, &QAction::triggered, this, [this]() { emit emotionRequested("angry"); });
    connect(confusedAction, &QAction::triggered, this, [this]() { emit emotionRequested("confused"); });
    connect(comfortAction, &QAction::triggered, this, [this]() { emit emotionRequested("comfort"); });

    m_menu->addSeparator();
    QAction *settingsAction = m_menu->addAction(tr("打开设置"));
    m_menu->addSeparator();
    QAction *quitAction = m_menu->addAction(tr("退出"));

    connect(showAction, &QAction::triggered, this, &TrayManager::showPetRequested);
    connect(hideAction, &QAction::triggered, this, &TrayManager::hidePetRequested);
    connect(startAction, &QAction::triggered, this, &TrayManager::startPetRequested);
    connect(pauseAction, &QAction::triggered, this, &TrayManager::pausePetRequested);
    connect(settingsAction, &QAction::triggered, this, &TrayManager::openSettingsRequested);
    connect(quitAction, &QAction::triggered, this, &TrayManager::quitRequested);

    m_trayIcon->setContextMenu(m_menu);
}

void TrayManager::onTrayActivated()
{
}

void TrayManager::applyTheme()
{
    if (!m_menu) {
        return;
    }

    QString style = ThemeManager::instance().menuStyleSheet();
    m_menu->setStyleSheet(style);

    for (QAction *action : m_menu->actions()) {
        if (QMenu *subMenu = action->menu()) {
            subMenu->setStyleSheet(style);
        }
    }
}
