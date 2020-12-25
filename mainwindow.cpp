#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    for (int i = 0; i < 24; ++i)
        relayOneInputs[i] = 0;
    for (int i = 0; i < 16; ++i)
        relayTwoInputs[i] = 0;
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
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, ".");
    connectionSettings = new QSettings("connectionSettings.ini", QSettings::IniFormat);
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
        // show error message and exit
    }
    relayOneUnitOne = new QModbusDataUnit(QModbusDataUnit::Coils, 0, 10);  // relayOne first 10 registers.
    relayOneUnitTwo = new QModbusDataUnit(QModbusDataUnit::Coils, 10, 6);  // relayOne second 6 registers.
    relayOneUnitThree = new QModbusDataUnit(QModbusDataUnit::Coils, 16, 8);// relayOne Extension 8 registers.
    relayTwoUnitOne = new QModbusDataUnit(QModbusDataUnit::Coils, 0, 10);  // relayTwo first 10 registers.
    relayTwoUnitTwo = new QModbusDataUnit(QModbusDataUnit::Coils, 10, 6);  // relayTwo second 6 registers.
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
        relayOneAdress = connectionSettings->value("Adress#1", 0).toInt();
    }
    else{
        relay->setConnectionParameter(QModbusDevice::SerialPortNameParameter, connectionSettings->value("Port#2", 0));
        relayTwoAdress = connectionSettings->value("Adress#2", 0).toInt();
    }
    relay->setConnectionParameter(QModbusDevice::SerialParityParameter, connectionSettings->value("Parity", 0).toInt());
    relay->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, connectionSettings->value("BaudRate", 0).toInt());
    relay->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, connectionSettings->value("DataBits", 0).toInt());
    relay->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, connectionSettings->value("StopBits", 0).toInt());
    relay->setTimeout(connectionSettings->value("Timeout", 0).toInt());
    relay->setNumberOfRetries(connectionSettings->value("NumberOfRetries", 0).toInt());
}

void MainWindow::readRelaysInputs(){
    statusBar()->clearMessage();
    if (auto *reply = relayOne->sendReadRequest(*relayOneUnitOne, relayOneAdress)) {
        if (!reply->isFinished())
            connect(reply, &QModbusReply::finished, this, &MainWindow::onReadReady);
        else
            delete reply; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + relayOne->errorString(), 5000);
    }

}

void MainWindow::onReadReady(int relayId, int registerPackId){

}
