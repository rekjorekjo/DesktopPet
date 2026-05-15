#include <QApplication>
#include "resizewindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("DesktopPet Action Resizer");
    app.setApplicationVersion("1.0.0");

    ResizeWindow window;
    window.show();

    return app.exec();
}
