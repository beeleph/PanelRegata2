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
    подумай как закрыть иник если это нужно вообще
    Попробовать закидать функции QAssert'ами для защиты? Прикольная идея
    Можно потихоньку накидывать GUI
    Или заниматься функционалом работы с архивом

    160621
    Поменять все датаюниты на правильные. поменять коилы на холдинги. правильные адреса и количество. убрать лишние.

    To commit:
    writeRelayInput renamed

*/
