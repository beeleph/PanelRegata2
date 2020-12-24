#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, ".");
    connectionSettings = new QSettings("connectionSettings.ini", QSettings::IniFormat);
    errorDialog = new errorConnectionDialog(this);
    relayOne = new QModbusRtuSerialMaster(this);
    relayTwo = new QModbusRtuSerialMaster(this);
    connect(relayOne, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        statusBar()->showMessage(relayOne->errorString(), 5000);
    });
    connect(relayTwo, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        statusBar()->showMessage(relayTwo->errorString(), 5000);
    });
    if (!relayOne || !relayTwo) {
        statusBar()->showMessage(tr("Could not create Modbus master."), 5000);
    }
    readIniToModbusDevice(relayOne, 0);
    readIniToModbusDevice(relayTwo, 1);
    if (!relayOne->connectDevice()) {
        statusBar()->showMessage(tr("Connect failed: ") + relayOne->errorString(), 5000);
        errorDialog->show();
        // show error message and exit
    }
    if (!relayTwo->connectDevice()) {
        statusBar()->showMessage(tr("Connect failed: ") + relayTwo->errorString(), 5000);
        errorDialog->show();
        //exit(0);
        // show error message and exit
    }
}

MainWindow::~MainWindow()
{
    if (relayOne)
        relayOne->disconnectDevice();
    if (relayTwo)
        relayTwo->disconnectDevice();
    delete relayOne;
    delete relayTwo;
    delete ui;
}

void MainWindow::readIniToModbusDevice(QModbusClient *relay, int id){
    statusBar()->clearMessage();
    if (id == 0) {
        relay->setConnectionParameter(QModbusDevice::SerialPortNameParameter, connectionSettings->value("Port#1", 0));
    }
    else{
        relay->setConnectionParameter(QModbusDevice::SerialPortNameParameter, connectionSettings->value("Port#2", 0));
    }
    relay->setConnectionParameter(QModbusDevice::SerialParityParameter, connectionSettings->value("Parity", 0).toInt());
    relay->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, connectionSettings->value("BaudRate", 0).toInt());
    relay->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, connectionSettings->value("DataBits", 0).toInt());
    relay->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, connectionSettings->value("StopBits", 0).toInt());
    relay->setTimeout(connectionSettings->value("Timeout", 0).toInt());
    relay->setNumberOfRetries(connectionSettings->value("NumberOfRetries", 0).toInt());
}
