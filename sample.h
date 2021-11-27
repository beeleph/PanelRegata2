#ifndef SAMPLE_H
#define SAMPLE_H

#include <QObject>
#include <QDateTime>
#include <QTimer>

class Sample
{
public:
    Sample();
    enum IrradiationChannel {
        IRCH_N1,
        IRCH_N2,
        IRCH_G
    };
    void setBeginDT();
    void setEndDT();
    void setSetDT(qint64 durationInSec);
    bool isIrradiationDone();
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
    IrradiationChannel irch;//?
    bool onChannel = false;
};

#endif // SAMPLE_H
