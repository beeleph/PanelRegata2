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
 *  соответственно накидать тестовых образцов чтобы посмотреть как это работает.
    Или заниматься функционалом работы с архивом
    Надо не забыть сбрасывать всю инфу с Sample при чтении нового образца.
    Надо бы сразу выбирать путь нужный при чтении образца с БД чтобы изменяемое время применилось к правильному Sample?
        !Возможно обновление инфы об образце не будет успевать. тк считывание происходит сразу при нажатии кнопки а смена пути нет.
    это можно подлечить попробовав вызывать updateguichotottam при слоте toggled на кнопках.
        Потом правильно настроить выставление времени начала и конца облучения с учетом таймеров и прочего чтобы получалось +- точно
*/
