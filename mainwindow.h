#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QModbusDataUnit>
#include <QtSerialBus/qtserialbusglobal.h>
#include <QSerialPort>
#include <QModbusRtuSerialMaster>
#include <errorconnectiondialog.h>
#include <QTimer>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void readIniToModbusDevice(QModbusClient *relay, int id);           // reads ini and writes settings to mbdevice
    void writeRelayRegister(int relayId, int registerAdress, int value);  // write single register
    void writeRelayInput(int relayId, int input, bool value);           // write single output
    void updateGuiOutputs();

private slots:
    void readRelaysOutputs();                                            // reads all da inputs from all relays
    void onReadReady(QModbusReply* reply, int relayId);

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

signals:
    void readFinished(QModbusReply* reply, int relayId);

private:                                        // yeah, i mean, obviously i should create entire class for "relay" just for two elements. bcs they duplicate each other. but i do not want
    Ui::MainWindow *ui;
    QSettings *connectionSettings = nullptr;
    QModbusClient *relayOne = nullptr;
    QModbusClient *relayTwo = nullptr;
    bool relayOneInputs[24];                    // relayOneInput[0] = I1, relayOneInput[1] = I2 etc.
    bool relayTwoInputs[16];
    bool relayOneOutputs[24];
    bool relayTwoOutputs[16];
    int relayOneAdress;
    int relayTwoAdress;
    QModbusDataUnit *relayOneMBUnit = nullptr;
    QModbusDataUnit *relayTwoMBUnit = nullptr;
    errorConnectionDialog *errorDialog = nullptr;
    QTimer *readOutputsTimer;
};
#endif // MAINWINDOW_H
