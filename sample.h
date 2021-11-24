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
    void setBeginDT();  // add onchannel true
    void setEndDT();    // add onchannel false
    void setSetDT(QDateTime setDT);
    bool isIrradiationDone();
    QString getName();
    void setName(QString name);
    qint64 getTimeElapsedInSec();
private:
    int id;//?
    QString name;
    QDateTime irradiationBeginDT;
    QDateTime irradiationEndDT;
    QDateTime irradiationSetDT;
    IrradiationChannel irch;//?
    bool onChannel = false;


};

#endif // SAMPLE_H
