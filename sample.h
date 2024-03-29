#ifndef SAMPLE_H
#define SAMPLE_H

#include <QObject>
#include <QDateTime>
#include <QTimer>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>

class Sample
{
public:
    Sample(int ch);
    void setBeginDT();
    void setEndDT();
    void setSetDT(qint64 durationInSec);
    void setDbConnectionState(bool state);
    bool isIrradiationDone();
    bool isIrradiationAlmostDone();
    bool isOnChannel();
    QVector<QString> getName();
    void setName(QVector<QString> name);
    qint64 getTimeElapsedInSec();
    qint64 getIrradiationDurationInSec();
    void resetInfo();
    QString getLastQueryError();
    void setExperimenterName(QString name);
    QString getExperimenterName();

private:
    int channel;
    QVector<QString> name;
    QDateTime irradiationBeginDT;
    QDateTime irradiationEndDT;
    qint64 irradiationDurationInSec = 0;
    bool onChannel = false;
    bool dbConnection = false;
    QString error, experimenterName = "";

};

#endif // SAMPLE_H
