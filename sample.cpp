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
                QSqlQuery query3("INSERT INTO [NAA_DB].[dbo].[table_SLI_Irradiation_Log] (Country_Code, Client_ID, Year, Sample_Set_ID, Sample_Set_Index, Sample_ID, Date_Start) VALUES ('" + name.at(1) + "','" + name.at(2) + "','" + name.at(3) + "','" + name.at(4) + "','" + name.at(5) + "','" + name.at(6) + "','" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "')" , db);
                QSqlQuery query("UPDATE [NAA_DB].[dbo].[table_SLI_Irradiation_Log] SET Time_Start='" + irradiationBeginDT.time().toString() +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "' AND Date_Start LIKE '" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "'", db);
                QSqlQuery query2("UPDATE [NAA_DB].[dbo].[table_SLI_Irradiation_Log] SET Channel=" + QString::number(channel) +" WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "' AND Date_Start LIKE '" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "'", db);

            }
            else
            if (name.at(0) == "ДЖИ"){
                QSqlQuery query3("INSERT INTO [NAA_DB].[dbo].[table_LLI_Irradiation_Log] (Country_Code, Client_ID, Year, Sample_Set_ID, Sample_Set_Index, Sample_ID, Date_Start) VALUES ('" + name.at(1) + "','" + name.at(2) + "','" + name.at(3) + "','" + name.at(4) + "','" + name.at(5) + "','" + name.at(6) + "','" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "')" , db);
                QSqlQuery query("UPDATE [NAA_DB].[dbo].[table_LLI_Irradiation_Log] SET Time_Start='" + irradiationBeginDT.time().toString() +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "' AND Date_Start LIKE '" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "'", db);
                QSqlQuery query2("UPDATE [NAA_DB].[dbo].[table_LLI_Irradiation_Log] SET Channel=" + QString::number(channel) +" WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "' AND Date_Start LIKE '" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "'", db);

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
                QSqlQuery query("UPDATE [NAA_DB].[dbo].[table_SLI_Irradiation_Log] SET Duration=" + QString::number(irradiationEndDT.toSecsSinceEpoch() - irradiationBeginDT.toSecsSinceEpoch()) +" WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "' AND Date_Start LIKE '" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "'", db);
                //QSqlQuery query2("UPDATE [NAA_DB].[dbo].[table_SLI_Irradiation_Log] SET Date_Start='" + irradiationBeginDT.date().toString("yyyy-MM-dd") +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "' AND Date_Start LIKE '" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "'", db);
                QSqlQuery query3("UPDATE [NAA_DB].[dbo].[table_SLI_Irradiation_Log] SET Irradiated_By='" + experimenterName +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "' AND Date_Start LIKE '" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "'", db);
                if (name.at(1) == "s" && name.at(2) == "s" && name.at(3) == "s"){
                    //smth related to standarts.
                    QSqlQuery queryStandart("SELECT SRM_SLI_Irradiation_Date_1, SRM_SLI_Irradiation_Date_2, SRM_SLI_Irradiation_Date_3, SRM_SLI_Irradiation_Date_4, SRM_SLI_Irradiation_Date_5, SRM_SLI_Irradiation_Date_6, SRM_SLI_Irradiation_Date_7, SRM_SLI_Irradiation_Date_8, SRM_SLI_Irradiation_Date_9, SRM_SLI_Irradiation_Date_10 FROM [NAA_DB].[dbo].[table_SRM] WHERE SRM_Set_Name LIKE '" + name.at(4) +"' AND SRM_Set_Number LIKE '" + name.at(5) + "' AND SRM_Number LIKE '" + name.at(6) + "'", db);
                    queryStandart.first();
                    for (int i = 0; i < 10; ++i){
                        if (queryStandart.value(i) == ""){
                            QSqlQuery queryStandart2("UPDATE [NAA_DB].[dbo].[table_SRM] SET SRM_SLI_Irradiation_Date_" + QString::number(i+1) + "='" + irradiationBeginDT.date().toString("yyyy-MM-dd") +"' WHERE SRM_Set_Name LIKE '" + name.at(4) +"' AND SRM_Set_Number LIKE '" + name.at(5) + "' AND SRM_Number LIKE '" + name.at(6) + "'", db);
                            break;
                        }
                        if (i == 9) {  // every column is filled
                            QSqlQuery queryStandart3("UPDATE [NAA_DB].[dbo].[table_SRM] SET SRM_SLI_Not_In_Use=1 WHERE SRM_Set_Name LIKE '" + name.at(4) +"' AND SRM_Set_Number LIKE '" + name.at(5) + "' AND SRM_Number LIKE '" + name.at(6) + "'", db);
                        }
                    }
                }
                else
                if (name.at(1) == "m" && name.at(2) == "m" && name.at(3) == "m"){
                    //smth related to monitors.
                    QSqlQuery queryMonitor("UPDATE [NAA_DB].[dbo].[table_Monitor] SET Monitor_SLI_Date_Start='" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "' WHERE Monitor_Set_Name LIKE '" + name.at(4) +"' AND Monitor_Set_Number LIKE '" + name.at(5) + "' AND Monitor_Number LIKE '" + name.at(6) + "'", db);
                }
                else{
                    //usual sample
                    QSqlQuery querySample("UPDATE [NAA_DB].[dbo].[table_Sample] SET I_SLI_Date_Start='" + irradiationBeginDT.date().toString("yyyy-MM-dd") +"' WHERE F_Country_Code LIKE '" + name.at(1) + "' AND F_Client_ID LIKE '" + name.at(2) + "' AND F_Year LIKE '" + name.at(3) + "' AND F_Sample_Set_ID LIKE '" + name.at(4) + "' AND F_Sample_Set_Index LIKE '" + name.at(5) + "' AND A_Sample_ID LIKE '" + name.at(6) + "'", db);
                }
            }
            else
            if (name.at(0) == "ДЖИ"){
                QSqlQuery query("UPDATE [NAA_DB].[dbo].[table_LLI_Irradiation_Log] SET Date_Finish='" + irradiationEndDT.date().toString("yyyy-MM-dd") +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "' AND Date_Start LIKE '" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "'", db);
                QSqlQuery query2("UPDATE [NAA_DB].[dbo].[table_LLI_Irradiation_Log] SET Time_Finish='" + irradiationEndDT.time().toString() +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "' AND Date_Start LIKE '" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "'", db);
                //QSqlQuery query3("UPDATE [NAA_DB].[dbo].[table_LLI_Irradiation_Log] SET Date_Start='" + irradiationBeginDT.date().toString("yyyy-MM-dd") +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "'", db);
                QSqlQuery query4("UPDATE [NAA_DB].[dbo].[table_LLI_Irradiation_Log] SET Irradiated_By='" + experimenterName +"' WHERE Country_Code LIKE '" + name.at(1) + "' AND Client_ID LIKE '" + name.at(2) + "' AND Year LIKE '" + name.at(3) + "' AND Sample_Set_ID LIKE '" + name.at(4) + "' AND Sample_Set_Index LIKE '" + name.at(5) + "' AND Sample_ID LIKE '" + name.at(6) + "' AND Date_Start LIKE '" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "'", db);
                if (name.at(1) == "s" && name.at(2) == "s" && name.at(3) == "s"){
                    //smth related to standarts.
                    QSqlQuery queryStandart("SELECT SRM_LLI_Irradiation_Date_1, SRM_LLI_Irradiation_Date_2, SRM_LLI_Irradiation_Date_3, SRM_LLI_Irradiation_Date_4, SRM_LLI_Irradiation_Date_5, SRM_LLI_Irradiation_Date_6, SRM_LLI_Irradiation_Date_7, SRM_LLI_Irradiation_Date_8, SRM_LLI_Irradiation_Date_9, SRM_LLI_Irradiation_Date_10 FROM [NAA_DB].[dbo].[table_SRM] WHERE SRM_Set_Name LIKE '" + name.at(4) +"' AND SRM_Set_Number LIKE '" + name.at(5) + "' AND SRM_Number LIKE '" + name.at(6) + "'", db);
                    queryStandart.first();
                    for (int i = 0; i < 10; ++i){
                        if (queryStandart.value(i) == ""){
                            QSqlQuery queryStandart2("UPDATE [NAA_DB].[dbo].[table_SRM] SET SRM_LLI_Irradiation_Date_" + QString::number(i+1) + "='" + irradiationBeginDT.date().toString("yyyy-MM-dd") +"' WHERE SRM_Set_Name LIKE '" + name.at(4) +"' AND SRM_Set_Number LIKE '" + name.at(5) + "' AND SRM_Number LIKE '" + name.at(6) + "'", db);
                            break;
                        }
                        if (i == 9) {  // every column is filled
                            QSqlQuery queryStandart3("UPDATE [NAA_DB].[dbo].[table_SRM] SET SRM_LLI_Not_In_Use=1 WHERE SRM_Set_Name LIKE '" + name.at(4) +"' AND SRM_Set_Number LIKE '" + name.at(5) + "' AND SRM_Number LIKE '" + name.at(6) + "'", db);
                        }
                    }
                }
                else
                if (name.at(1) == "m" && name.at(2) == "m" && name.at(3) == "m"){
                    //smth related to monitors.
                    QSqlQuery queryMonitor("UPDATE [NAA_DB].[dbo].[table_Monitor] SET Monitor_LLI_Date_Start='" + irradiationBeginDT.date().toString("yyyy-MM-dd") + "' WHERE Monitor_Set_Name LIKE '" + name.at(4) +"' AND Monitor_Set_Number LIKE '" + name.at(5) + "' AND Monitor_Number LIKE '" + name.at(6) + "'", db);
                }
                else{
                    //usual sample
                    QSqlQuery querySample("UPDATE [NAA_DB].[dbo].[table_Sample] SET I_LLI_Date_Start='" + irradiationBeginDT.date().toString("yyyy-MM-dd") +"' WHERE F_Country_Code LIKE '" + name.at(1) + "' AND F_Client_ID LIKE '" + name.at(2) + "' AND F_Year LIKE '" + name.at(3) + "' AND F_Sample_Set_ID LIKE '" + name.at(4) + "' AND F_Sample_Set_Index LIKE '" + name.at(5) + "' AND A_Sample_ID LIKE '" + name.at(6) + "'", db);
                }
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

void Sample::setExperimenterName(QString name){
    experimenterName = name;
}

QString Sample::getExperimenterName(){
    return experimenterName;
}
