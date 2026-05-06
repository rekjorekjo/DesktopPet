#include "core/appsettings.h"
#include "core/petpaths.h"
#include "runtime/petwidget.h"
#include "runtime/traymanager.h"
#include "ui/settingswindow.h"
#include "services/actionimportservice.h"

#include <QApplication>
#include <QIcon>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);

    qRegisterMetaType<ActionImportResult>("ActionImportResult");

    QIcon appIcon(":/icons/app_icon.ico");
    QApplication::setWindowIcon(appIcon);

    PetPaths::ensureDefaultStructure();

    PetWidget pet;
    SettingsWindow settings;
    TrayManager tray;

    QObject::connect(&pet, &PetWidget::openSettingsRequested, [&settings]() {
        if (!settings.isVisible()) {
            settings.show();
        }
        settings.raise();
        settings.activateWindow();
    });

    QObject::connect(&pet, &PetWidget::hidePetRequested, &pet, &PetWidget::hide);

    QObject::connect(&pet, &PetWidget::quitRequested, &app, &QApplication::quit);

    QObject::connect(&settings, &SettingsWindow::startPetRequested,
                     &pet, &PetWidget::startPet);

    QObject::connect(&settings, &SettingsWindow::pausePetRequested,
                     &pet, &PetWidget::pausePet);

    QObject::connect(&settings, &SettingsWindow::applyPetConfigRequested,
                     &pet, &PetWidget::reloadPet);

    QObject::connect(&settings, &SettingsWindow::petOpacityChanged,
                     &pet, &PetWidget::setPetOpacity);

    QObject::connect(&tray, &TrayManager::showPetRequested, &pet, &PetWidget::show);

    QObject::connect(&tray, &TrayManager::hidePetRequested, &pet, &PetWidget::hide);

    QObject::connect(&tray, &TrayManager::startPetRequested, &pet, &PetWidget::startPet);

    QObject::connect(&tray, &TrayManager::pausePetRequested, &pet, &PetWidget::pausePet);

    QObject::connect(&tray, &TrayManager::openSettingsRequested, [&settings]() {
        if (!settings.isVisible()) {
            settings.show();
        }
        settings.raise();
        settings.activateWindow();
    });

    QObject::connect(&tray, &TrayManager::quitRequested, &app, &QApplication::quit);

    QObject::connect(&tray, &TrayManager::emotionRequested,
                     &pet, &PetWidget::playEmotion);

    pet.show();

    if (AppSettings::openSettingsOnLaunch()) {
        settings.show();
    }

    tray.show();

    return app.exec();
}
