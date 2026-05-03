#include "traymanager.h"

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

    QIcon icon = QApplication::windowIcon();
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

    m_menu->setStyleSheet(
        "QMenu {"
        "  padding: 8px;"
        "  min-width: 180px;"
        "}"
        "QMenu::item {"
        "  padding: 8px 28px 8px 18px;"
        "  min-height: 28px;"
        "}"
        "QMenu::separator {"
        "  height: 1px;"
        "  margin: 6px 8px;"
        "}"
    );

    QAction *showAction = m_menu->addAction(tr("显示桌宠"));
    QAction *hideAction = m_menu->addAction(tr("隐藏桌宠"));
    m_menu->addSeparator();
    QAction *startAction = m_menu->addAction(tr("开始"));
    QAction *pauseAction = m_menu->addAction(tr("暂停"));
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
