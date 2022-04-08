#include "sample.h"
#include "QSqlError"

const int irradiationCorrectionSec = 8; // in seconds

Sample::Sample(int ch)
{
    channel = ch;
    irradiationBeginDT.setSecsSinceEpoch(0);
    irradiationEndDT.setSecsSinceEpoch(0);
    name.resize(7);
}

void Sample::setBeginDT(){
    irradiationBeginDT = QDateTime::currentDateTime();
    onChannel = true;
    if (dbConnection){
        QSqlDatabase db = QSqlDatabase::database("NAA_db");
        if (db.isOpen()){
            if (name.at(0) == "КЖИ"){
                QSqlQuery checkIfExist("SELECT Sample_ID FROM [NAA_DB].[dbo].[table_SLI_Irradiation_Log] WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "'", db);
                if (!checkIfExist.next()){
                    QSqlQuery query("INSERT INTO [NAA_DB].[dbo].[table_SLI_Irradiation_Log] (Country_Code, Client_ID, Year, Sample_Set_ID, Sample_Set_Index, Sample_ID, Date_Start) VALUES ('" + name.at(1) + "','" + name.at(2) + "','" + name.at(3) + "','" + name.at(4) + "','" + name.at(5) + "','" + name.at(6) + "','" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "')" , db);
                }
                QSqlQuery query("UPDATE [NAA_DB].[dbo].[table_SLI_Irradiation_Log] SET Time_Start='" + irradiationBeginDT.time().toString() +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "'", db);
                QSqlQuery query2("UPDATE [NAA_DB].[dbo].[table_SLI_Irradiation_Log] SET Channel=" + QString::number(channel) +" WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "'", db);
            }
            else
            if (name.at(0) == "ДЖИ"){
                QSqlQuery checkIfExist("SELECT Sample_ID FROM [NAA_DB].[dbo].[table_LLI_Irradiation_Log] WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "'", db);
                if (!checkIfExist.next()){
                    QSqlQuery query("INSERT INTO [NAA_DB].[dbo].[table_LLI_Irradiation_Log] (Country_Code, Client_ID, Year, Sample_Set_ID, Sample_Set_Index, Sample_ID, Date_Start) VALUES ('" + name.at(1) + "','" + name.at(2) + "','" + name.at(3) + "','" + name.at(4) + "','" + name.at(5) + "','" + name.at(6) + "','" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "')" , db);
                }
                QSqlQuery query("UPDATE [NAA_DB].[dbo].[table_LLI_Irradiation_Log] SET Time_Start='" + irradiationBeginDT.time().toString() +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "'", db);
                //error = query.lastError().text();
                QSqlQuery query2("UPDATE [NAA_DB].[dbo].[table_LLI_Irradiation_Log] SET Channel=" + QString::number(channel) +" WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "'", db);
            }
        }
        else{
            //say("Cannot write startTime and channel info. Database is not connected");
        }
    }
}

void Sample::setEndDT(){
    irradiationEndDT = QDateTime::currentDateTime();
    onChannel = false;
    if (dbConnection){
        QSqlDatabase db = QSqlDatabase::database("NAA_db"); // [dbo]. paste
        if (db.isOpen()){
            if (name.at(0) == "КЖИ"){
                QSqlQuery query("UPDATE [NAA_DB].[dbo].[table_SLI_Irradiation_Log] SET Duration=" + QString::number(irradiationEndDT.toSecsSinceEpoch() - irradiationBeginDT.toSecsSinceEpoch()) +" WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "'", db);
                QSqlQuery query2("UPDATE [NAA_DB].[dbo].[table_SLI_Irradiation_Log] SET Date_Start='" + irradiationBeginDT.date().toString("yyyy-MM-dd") +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "'", db);
            }
            else
            if (name.at(0) == "ДЖИ"){
                QSqlQuery query("UPDATE [NAA_DB].[dbo].[table_LLI_Irradiation_Log] SET Date_Finish='" + irradiationEndDT.date().toString("yyyy-MM-dd") +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "'", db);
                QSqlQuery query2("UPDATE [NAA_DB].[dbo].[table_LLI_Irradiation_Log] SET Time_Finish='" + irradiationEndDT.time().toString() +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "'", db);
                QSqlQuery query3("UPDATE [NAA_DB].[dbo].[table_LLI_Irradiation_Log] SET Date_Start='" + irradiationBeginDT.date().toString("yyyy-MM-dd") +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "'", db);
            }
        }
    }
}

void Sample::setSetDT(qint64 durationInSec){
    irradiationDurationInSec = durationInSec;
}

bool Sample::isIrradiationDone(){
    if (onChannel)
        if ( QDateTime::currentDateTime().toSecsSinceEpoch() > irradiationBeginDT.toSecsSinceEpoch() + irradiationDurationInSec - irradiationCorrectionSec ){
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

void Sample::setDbConnectionState(bool state){
    dbConnection = state;
}

QString Sample::getLastQueryError(){
    return error;
}
