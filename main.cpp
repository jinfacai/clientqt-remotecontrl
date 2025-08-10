#include "TestUI.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    //Widget w;
    //w.show();
    TestUI t;
    t.show();

    return a.exec();
}
