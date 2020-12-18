#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

/* ToDo:
    1. Ini file func
    2. Modbus func
    2.1 Check modbus with device asap
    3. Warning window if can't connect

    To commit:
    + some modbus includes, two modbus clients, settings init in the constructor,
*/
