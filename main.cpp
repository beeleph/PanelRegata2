/* Igor Zhironkin isjironn@gmail.com, jironkin@jinr.ru, FLNP JINR Dubna 2022 */

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    MainWindow w;
    w.showMaximized();
    return a.exec();
}

/* ToDo:
*/
//      Инфа для инструкций и прочего:
//      то есть последовательность у оператора должна быть такая: кладем контейнер, выбираем путь, выбираем образец, запускаем это дерьмо. okay, shit?
//      В инструкции указать что оператор должен удостовериться что часы на ПК идут верно. Что подожать пару секунд при смене пути. И удостовериться.
