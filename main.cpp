/* Igor Zhironkin isjironn@gmail.com, jironkin@jinr.ru, FLNP JINR Dubna 2022 */

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.show();
    return a.exec();
}

/* ToDo:
        Потом правильно настроить выставление времени начала и конца облучения с учетом таймеров и прочего чтобы получалось +- точно
        Настройки подключения к БД надо считывать с иника. с того же иника в общем с которого и остальные настройки подключения.
        узнать сверху почему канал это smallInt в БД и как это интерпретировать. пока будем считать что 0 - н1, 1 - Н2, 2 - Г
        я не уверен в правильной инициализации каналов в образцах, надо тестировать
        добавить дебаг инфы по поводу того что сейчас система делает с образцами.
        запилить кнопочку "сброс"? если надо просто покидаться образцами без записи в бд. или еще для чего. стирать инфу можно только когда образец не на облучении.
        При отсутствии подключения к БД кнопка "выбрать" становится не активной, при этом при наличии контейнера она становится активной. нужна доппроверка на ДБ при контейнере
        !!Для идентификации записи недостаточно только страны и номера образца. с прискорбием сообщаю что для однозначной идентификации конкретного образца нужно проверять ВСЕ ключевые поля. дятлы всемогущие, в чем проблема поле с айди добавить зачем такой геморрой?
рабочий запрос
UPDATE [Regata-2].[dbo].[table_SLI_Irradiation_Log] SET Channel=54 WHERE Country_Code LIKE 's' AND Sample_ID LIKE '20'
        !!irradiationBeginTime Отстает на три часа.

QObject::connect: No such slot MainWindow::timeToAutoReturn(IRCH_N1) in ..\PanelRegata2\mainwindow.cpp:235
QObject::connect:  (receiver name: 'MainWindow')

        дату окончания облучения не пишет в LLI, видимо формат не тот

*/
//то есть последовательность у оператора должна быть такая: кладем контейнер, выбираем путь, выбираем образец, запускаем это дерьмо. okay, shit?
//      !интересная фишка, в КЖИ есть поле Duration, в ДЖИ нет, но есть Date_Finish, Time_Finish
//      !Возможно обновление инфы об образце не будет успевать. тк считывание происходит сразу при нажатии кнопки а смена пути нет.
// это можно подлечить попробовав вызывать updateguichotottam при слоте toggled на кнопках.
