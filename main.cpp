#include "core/petwidget.h"
#include "core/settingswindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    PetWidget pet;
    SettingsWindow settings;

    QObject::connect(&pet, &PetWidget::openSettingsRequested, [&settings]() {
        if (!settings.isVisible()) {
            settings.show();
        }
        settings.raise();
        settings.activateWindow();
    });

    QObject::connect(&pet, &PetWidget::quitRequested, &app, &QApplication::quit);

    QObject::connect(&settings, &SettingsWindow::startPetRequested,
                     &pet, &PetWidget::startPet);

    QObject::connect(&settings, &SettingsWindow::pausePetRequested,
                     &pet, &PetWidget::pausePet);

    pet.show();

    return app.exec();
}
