#include "sample.h"

Sample::Sample(int ch)
{
    channel = ch;
    irradiationBeginDT.setSecsSinceEpoch(0);
    irradiationEndDT.setSecsSinceEpoch(0);
    name.resize(3);
    qDebug() << "sample channel = " + QString::number(ch);
}

void Sample::setBeginDT(){
    irradiationBeginDT = QDateTime::currentDateTimeUtc();
    onChannel = true;
    QSqlDatabase db = QSqlDatabase::database("NAA_db");
    if (db.isOpen()){
        if (name.at(0) == "КЖИ"){
            QSqlQuery query("UPDATE [Regata-2].[dbo].[table_SLI_Irradiation_Log] SET Time_Start='" + irradiationBeginDT.time().toString() +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Sample_ID LIKE '" + name.at(2) + "'", db);
            QSqlQuery query2("UPDATE [Regata-2].[dbo].[table_SLI_Irradiation_Log] SET Channel=" + QString::number(channel) +" WHERE Country_Code LIKE '" + name.at(1) + "' AND Sample_ID LIKE '" + name.at(2) + "'", db);
        }
        else
        if (name.at(0) == "ДЖИ"){
            QSqlQuery query("UPDATE [Regata-2].[dbo].[table_LLI_Irradiation_Log] SET Time_Start='" + irradiationBeginDT.time().toString() +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Sample_ID LIKE '" + name.at(2) + "'", db);
            QSqlQuery query2("UPDATE [Regata-2].[dbo].[table_LLI_Irradiation_Log] SET Channel=" + QString::number(channel) +" WHERE Country_Code LIKE '" + name.at(1) + "' AND Sample_ID LIKE '" + name.at(2) + "'", db);
        }
    }
    else{
        //say("Cannot write startTime and channel info. Database is not connected");
    }
}

void Sample::setEndDT(){
    irradiationEndDT = QDateTime::currentDateTimeUtc();
    onChannel = false;
    QSqlDatabase db = QSqlDatabase::database("NAA_db");
    if (db.isOpen()){
        if (name.at(0) == "КЖИ"){
            QSqlQuery query("UPDATE [Regata-2].[dbo].[table_SLI_Irradiation_Log] SET Duration=" + QString::number(irradiationEndDT.toSecsSinceEpoch() - irradiationBeginDT.toSecsSinceEpoch()) +" WHERE Country_Code LIKE '" + name.at(1) + "' AND Sample_ID LIKE '" + name.at(2) + "'", db);
        }
        else
        if (name.at(0) == "ДЖИ"){
            QSqlQuery query("UPDATE [Regata-2].[dbo].[table_LLI_Irradiation_Log] SET Date_Finish='" + irradiationEndDT.date().toString("yyyy-MM-dd") +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Sample_ID LIKE '" + name.at(2) + "'", db);
            QSqlQuery query2("UPDATE [Regata-2].[dbo].[table_LLI_Irradiation_Log] SET Time_Finish='" + irradiationEndDT.time().toString() +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Sample_ID LIKE '" + name.at(2) + "'", db);
        }
    }
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

QVector<QString> Sample::getName(){
    return name;
}

void Sample::setName(QVector<QString> name){
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

void Sample::resetInfo(){
    for (int i = 0; i < 3; ++i){
        name[i] = "-";
    }
    irradiationBeginDT.setSecsSinceEpoch(0);
    irradiationEndDT.setSecsSinceEpoch(0);
    irradiationDurationInSec = 0;
}
