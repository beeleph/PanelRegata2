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
//      то есть последовательность у оператора должна быть такая: кладем контейнер, выбираем путь, выбираем образец, выбираем экспериментатора запускаем это дерьмо. okay, shit?
//      В инструкции указать что оператор должен удостовериться что часы на ПК идут верно. Что подожать пару секунд при смене пути. И удостовериться.
//          добавить в инструкцию что если у вас не возвращается контейнер, тогда и тот по времени не вернется. Также про строгий формат записи номером файлов.
//      7 в длину четыре штуки между ними запятая.требования к ОС - минимум windows 7.
//          Также, если вернулся контейнер и вы его не сбросили со стоппера, то за 10 секунд до возврата по таймеру следующего (в случае если он есть вообще) предыдущий контейнер
//      будет сброшен автоматически, т.к. без этого возврат невозможен. Также уточнить что невозможно отправить следующий контейнер если скоро по таймеру (меньше 2 мин, помоему) должен возвращаться предыдущий
//      Соответственно нельзя присвоить такое время облучения для нового контейнера которое по факту будет очень близко по времени вовзрата к предыдущему контейнеру. Потому что одновременно может посылаться/возвращаться только один контейнер и это занимает минимум минуту.

