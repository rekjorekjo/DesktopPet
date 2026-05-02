#include "core/petwidget.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    PetWidget pet;
    pet.show();
    return a.exec();
}
