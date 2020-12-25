#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSettings>
#include <QModbusDataUnit>
#include <QtSerialBus/qtserialbusglobal.h>
#include <QSerialPort>
#include <QModbusRtuSerialMaster>
#include <errorconnectiondialog.h>

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
    void readIniToModbusDevice(QModbusClient *relay, int id); // reads ini and writes settings to mbdevice
    void readRelaysInputs();

private slots:
    void onReadReady(int relayId, int registerPackId);

private:
    Ui::MainWindow *ui;
    QSettings *connectionSettings = nullptr;
    QModbusClient *relayOne = nullptr;
    QModbusClient *relayTwo = nullptr;
    bool relayOneInputs[24];
    bool relayTwoInputs[16];
    int relayOneAdress;
    int relayTwoAdress;
    QModbusDataUnit *relayOneUnitOne = nullptr;
    QModbusDataUnit *relayOneUnitTwo = nullptr;
    QModbusDataUnit *relayOneUnitThree = nullptr;
    QModbusDataUnit *relayTwoUnitOne = nullptr;
    QModbusDataUnit *relayTwoUnitTwo = nullptr;
    errorConnectionDialog *errorDialog = nullptr;
};
#endif // MAINWINDOW_H
