#ifndef SAMPLE_H
#define SAMPLE_H

#include <QObject>
#include <QDateTime>
#include <QTimer>

class Sample
{
public:
    Sample();
    void setBeginDT();
    void setEndDT();
    void setSetDT(qint64 durationInSec);
    bool isIrradiationDone();
    bool isOnChannel();
    QString getName();
    void setName(QString name);
    qint64 getTimeElapsedInSec();
    qint64 getIrradiationDurationInSec();
private:
    int id;//?
    QString name;
    QDateTime irradiationBeginDT;
    QDateTime irradiationEndDT;
    qint64 irradiationDurationInSec = 0;
    bool onChannel = false;
};

#endif // SAMPLE_H
