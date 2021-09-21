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
        return;
        // show error message and exit
    }
    if (!relayTwo->connectDevice()) {
        statusBar()->showMessage(tr("Connect failed: ") + relayTwo->errorString(), 5000);
        errorDialog->show();
        return;
        // show error message and exit
    }
    relayOneMBUnit = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 20, 4);
    relayTwoMBUnit = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 20, 4);
    connect(this, SIGNAL(readFinished(QModbusReply*, int)), this, SLOT(onReadReady(QModbusReply*, int)));
    // setup timer for readRelaysInputs
    readInputsTimer = new QTimer(this);
    connect(readInputsTimer, SIGNAL(timeout()), this, SLOT(readRelaysInputs()));
    readInputsTimer->start(1000);
}

MainWindow::~MainWindow()
{
    readInputsTimer->stop();
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
        relay->setConnectionParameter(QModbusDevice::SerialPortNameParameter, connectionSettings->value("Port1", 0));
        relayOneAdress = connectionSettings->value("Adress1", 0).toInt();
    }
    else{
        relay->setConnectionParameter(QModbusDevice::SerialPortNameParameter, connectionSettings->value("Port2", 0));
        relayTwoAdress = connectionSettings->value("Adress2", 0).toInt();
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
    /*if (auto *replyOne = relayOne->sendReadRequest(*relayOneMBUnit, relayOneAdress)) {
        if (!replyOne->isFinished())
            connect(replyOne, &QModbusReply::finished, this, [this, replyOne](){
                emit readFinished(replyOne, 0);  // read fiinished connects to ReadReady()
            });
        else
            delete replyOne; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + relayOne->errorString(), 5000);
    }*/
    if (auto *replyTwo = relayTwo->sendReadRequest(*relayTwoMBUnit, relayTwoAdress)) {
        if (!replyTwo->isFinished())
            connect(replyTwo, &QModbusReply::finished, this, [this, replyTwo](){
                emit readFinished(replyTwo, 1);
            });
        else
            delete replyTwo; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + relayOne->errorString(), 5000);
    }
    updateGuiInputs();
    // update GUI
}

void MainWindow::onReadReady(QModbusReply* reply, int relayId){  // relayOne id = 0;
    if (!reply)
        return;
    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        int value = 0;
        if (relayId == 0){          // first relay
            value = unit.value(0);
            for (int i = 0; i < 16; ++i){
                relayOneInputs[i] = value % 2;
                value = value / 2;
            }
            value = unit.value(1);
            for (int i = 16; i < 24; ++i){
                relayOneInputs[i] = value % 2;
                value = value / 2;
            }
        }
        else{                       // thee other one
            value = unit.value(0);
            for (int i = 0; i < 16; ++i){
                relayTwoInputs[i] = value % 2;
                value = value / 2;
            }
        }
    } else if (reply->error() == QModbusDevice::ProtocolError) {
        statusBar()->showMessage(tr("Read response error: %1 (Mobus exception: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->rawResult().exceptionCode(), -1, 16), 5000);
    } else {
        statusBar()->showMessage(tr("Read response error: %1 (code: 0x%2)").
                                    arg(reply->errorString()).
                                    arg(reply->error(), -1, 16), 5000);
    }
    reply->deleteLater();
}

void MainWindow::writeRelayRegister(int relayId, int registerAdress, int value){
    statusBar()->clearMessage();

    QModbusDataUnit *writeUnit = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, registerAdress, 1);
    writeUnit->setValue(0, value);
    QModbusReply *reply;
    if (relayId == 0)
        reply = relayOne->sendWriteRequest(*writeUnit, relayOneAdress);
    else
        reply = relayTwo->sendWriteRequest(*writeUnit, relayTwoAdress);
    if (reply){
        if (!reply->isFinished()) {
            connect(reply, &QModbusReply::finished, this, [this, reply]() {
                if (reply->error() == QModbusDevice::ProtocolError) {
                    statusBar()->showMessage(tr("Write response error: %1 (Mobus exception: 0x%2)")
                        .arg(reply->errorString()).arg(reply->rawResult().exceptionCode(), -1, 16),
                        5000);
                } else if (reply->error() != QModbusDevice::NoError) {
                    statusBar()->showMessage(tr("Write response error: %1 (code: 0x%2)").
                        arg(reply->errorString()).arg(reply->error(), -1, 16), 5000);
                }
                reply->deleteLater();
            });
        } else {
            // broadcast replies return immediately
            reply->deleteLater();
        }
    } else {
        statusBar()->showMessage(tr("Write error: "), 5000);
    }
}
void MainWindow::updateGuiInputs(){
    this->ui->relayTwoI1->setChecked(relayTwoInputs[0]);
    this->ui->relayTwoI2->setChecked(relayTwoInputs[1]);
    this->ui->relayTwoI3->setChecked(relayTwoInputs[2]);
}
void MainWindow::writeRelayOutput(int relayId, int output, bool value){
    //mb add some safety here
    //write to outputs bitmask, calculate register value, call da function
    int registerValue = 0;
    if (relayId == 0){
        relayOneOutputs[output] = value;
        if (output < 16){
            if (relayOneOutputs[0])
                registerValue += 1;
            for (int i = 1; i < 16; ++i){
                if (relayOneOutputs[i])
                    registerValue += i * 2;
            }
            writeRelayRegister(0,16,registerValue);
        }
        else{
            if (relayOneOutputs[16])
                registerValue += 1;
            for (int i = 17; i < 24; ++i){
                if (relayOneOutputs[i])
                    registerValue += (i-16) * 2;
            }
            writeRelayRegister(0, 17, registerValue);
        }
    }
    else{
        relayTwoOutputs[output] = value;
        if (relayTwoOutputs[0])
            registerValue += 1;
        for (int i = 1; i < 16; ++i){
            if (relayTwoOutputs[i])
                registerValue += i * 2;
        }
        writeRelayRegister(1, 16, registerValue);
    }
}
void MainWindow::on_relayTwoO1_stateChanged(int arg1)
{
    /*writeRelayRegister(1, 16, arg1);
    qDebug() << " arg1 " << arg1;*/
    writeRelayOutput(1,0,(bool)arg1);
}

