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

    pet.show();

    return app.exec();
}
