#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QApplication>
#include <QMainWindow>
#include <QSettings>
#include <QModbusDataUnit>
#include <QtSerialBus/qtserialbusglobal.h>
#include <QSerialPort>
#include <QModbusRtuSerialMaster>
#include <errorconnectiondialog.h>
#include <QTimer>
#include <QDebug>
#include <cmath>
#include <sample.h>
#include <QSqlDatabase>
#include <samplejournal.h>
#include <qsqlerror.h>
#include <QModbusTcpClient>


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

enum IrradiationChannel {
    IRCH_N1,
    IRCH_N2,
    IRCH_G
};
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
private:
    void readIniToModbusDevice();           // reads ini and writes settings to mbdevice
    void writeRelayRegister(int relayId, int registerAdress, int value);  // write single register
    void writeRelayInput(int relayId, int input, bool value);           // write single output
    void updateGuiOutputs();
    void updateGuiSampleInfo();
    void calculateIrradiationDuration();
    bool isIrradiationTimeAppropriate();
    void say(QString text);
    bool createDbConnection();


private slots:
    void readRelaysOutputs();                                            // reads all da inputs from all relays
    void onReadReady(QModbusReply* reply, int relayId);
    void timeToAutoReturnN1();                                  // really i dont't get how to get rid off that duplicating functions IN CASE im using QTimer.
    void timeToAutoReturnN2();
    void timeToAutoReturnG();
    void checkAutoReturnN1();
    void checkAutoReturnN2();
    void checkAutoReturnG();
    void readSampleInfo(QVector<QString> sampleInfo);

    void on_N1Button_pressed();

    void on_N1Button_released();

    void on_N2Button_pressed();

    void on_N2Button_released();

    void on_GButton_pressed();

    void on_GButton_released();

    void on_dozPostButton_pressed();

    void on_dozPostButton_released();

    void on_startButton_pressed();

    void on_startButton_released();

    void on_radioButton_toggled(bool checked);

    void on_radioButton_2_toggled(bool checked);

    void on_radioButton_3_toggled(bool checked);

    void on_radioButton_4_toggled(bool checked);

    void on_radioButton_5_toggled(bool checked);

    void on_radioButton_6_toggled(bool checked);

    void on_radioButton_7_toggled(bool checked);

    void on_radioButton_8_toggled(bool checked);

    void on_probotekaButton_toggled(bool checked);

    void on_proboDropButton_pressed();

    void on_proboDropButton_released();

    void on_returnButton_pressed();

    void on_returnButton_released();

    void on_emergencyDozPostButton_pressed();

    void on_emergencyDozPostButton_released();

    void on_emergencyReturnButton_pressed();

    void emergencyReturnOff();

    void on_buttonBox_accepted();

    void on_buttonBox_rejected();

    void on_setDaysSpinBox_valueChanged(int arg1);

    void on_setHoursSpinBox_valueChanged(int arg1);

    void on_setMinutesSpinBox_valueChanged(int arg1);

    void on_setSecondsSpinBox_valueChanged(int arg1);

    void on_sampleChooseButton_clicked();

    void on_testStartButton_clicked();

    void on_testPathComboBox_currentIndexChanged(const QString &arg1);

    void on_testEndButton_clicked();

    void on_sampleResetButton_clicked();

    void on_languageButton_toggled(bool checked);

signals:
    void readFinished(QModbusReply* reply, int relayId);

private:                                        // yeah, i mean, obviously i should create entire class for "relay" just for two elements. bcs they duplicate each other. but i do not want
    Ui::MainWindow *ui;
    QSettings *connectionSettings = nullptr;
    QModbusClient *modbusMaster = nullptr;
    QModbusTcpClient *tcpModbusMaster = nullptr;
    bool relayOneInputs[24];                    // relayOneInput[0] = I1, relayOneInput[1] = I2 etc.
    bool relayTwoInputs[16];
    bool relayOneInputSensors[8];
    bool relayOneOutputs[24];
    bool relayTwoOutputs[16];
    bool dbConnection = false;
    double doze = 0;
    int relayOneAdress;
    int relayTwoAdress;
    QModbusDataUnit *relayOneMBUnit = nullptr;
    QModbusDataUnit *relayTwoMBUnit = nullptr;
    QModbusDataUnit *gammaMBUnit = nullptr;
    errorConnectionDialog *errorDialog = nullptr;
    QTimer *readOutputsTimer, *emergencyReturnTimer;
    Sample N1Sample{0}, N2Sample{1}, GSample{2};
    qint64 irradiationDurationInSec;
    qint64 irradiationElapsedInSec;
    QVector<QString> tmpSampleInfo;
    QString DBserver, DBname, DBuser, DBpwd;
    bool engLang = false;
};
#endif // MAINWINDOW_H
