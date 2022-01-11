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
    bool isIrradiationDone();
    bool isOnChannel();
    QVector<QString> getName();
    void setName(QVector<QString> name);
    qint64 getTimeElapsedInSec();
    qint64 getIrradiationDurationInSec();
    void resetInfo();
private:
    int channel;
    QVector<QString> name;
    QDateTime irradiationBeginDT;
    QDateTime irradiationEndDT;
    qint64 irradiationDurationInSec = 0;
    bool onChannel = false;
};

#endif // SAMPLE_H
