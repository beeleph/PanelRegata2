#include "sample.h"

Sample::Sample()
{

}

void Sample::setBeginDT(){
    irradiationBeginDT = QDateTime::currentDateTimeUtc();
    onChannel = true;
}

void Sample::setEndDT(){
    irradiationEndDT = QDateTime::currentDateTimeUtc();
    onChannel = false;
}

void Sample::setSetDT(QDateTime setDT){
    irradiationSetDT = setDT;
}

bool Sample::isIrradiationDone(){
    if ( QDateTime::currentDateTimeUtc() > irradiationSetDT && onChannel){
        return true;
    }
    return false;
}

QString Sample::getName(){
    return name;
}

void Sample::setName(QString name){
    this->name = name;
}

qint64 Sample::getTimeElapsedInSec(){
    return QDateTime::currentSecsSinceEpoch() - irradiationBeginDT.toSecsSinceEpoch();
}
