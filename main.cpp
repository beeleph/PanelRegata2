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
    2. Modbus func
    2.1 Check modbus with device asap
    подумай как закрыть иник если это нужно вообще
    не забудь реализовать присвоение адреса к модбас девайсу через функцию чтения/записи.
    не забудь добавить таймер с вызовом опроса

    To commit:
    + bool relayInputs for two relays + initialization in the constructor
    + qmodbusdataunit
    + readRelaysInputs
    + onReadReady
*/
