#include <QApplication>
#include "resizewindow.h"
#include "appversion_generated.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("DesktopPet Action Resizer");
    app.setApplicationVersion(APP_VERSION_TAG);

    ResizeWindow window;
    window.show();

    return app.exec();
}
