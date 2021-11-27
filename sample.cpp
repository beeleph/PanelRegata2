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

void Sample::setSetDT(qint64 durationInSec){
    irradiationDurationInSec = durationInSec;
}

bool Sample::isIrradiationDone(){
    if (onChannel)
        if ( QDateTime::currentDateTimeUtc().toSecsSinceEpoch() > irradiationBeginDT.toSecsSinceEpoch() + irradiationDurationInSec ){
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

qint64 Sample::getIrradiationDurationInSec(){
    return irradiationDurationInSec;
}
