#include "sample.h"

Sample::Sample()
{
    irradiationBeginDT.setSecsSinceEpoch(0);
    irradiationEndDT.setSecsSinceEpoch(0);
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
    if (onChannel)
        return QDateTime::currentSecsSinceEpoch() - irradiationBeginDT.toSecsSinceEpoch();
    else
        return irradiationEndDT.toSecsSinceEpoch() - irradiationBeginDT.toSecsSinceEpoch();
}

qint64 Sample::getIrradiationDurationInSec(){
    return irradiationDurationInSec;
}

bool Sample::isOnChannel(){
    if (onChannel)
        return true;
    else
        return false;
}
