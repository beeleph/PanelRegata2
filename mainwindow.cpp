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
    relayOneUnitOne = new QModbusDataUnit(QModbusDataUnit::Coils, 0, 10);  // relayOne first 10 registers.
    relayOneUnitTwo = new QModbusDataUnit(QModbusDataUnit::Coils, 10, 6);  // relayOne second 6 registers.
    relayOneUnitThree = new QModbusDataUnit(QModbusDataUnit::Coils, 16, 8);// relayOne Extension 8 registers.
    relayTwoUnitOne = new QModbusDataUnit(QModbusDataUnit::Coils, 0, 10);  // relayTwo first 10 registers.
    relayTwoUnitTwo = new QModbusDataUnit(QModbusDataUnit::Coils, 10, 6);  // relayTwo second 6 registers.
    connect(this, SIGNAL(readFinished(QModbusReply*, int, int)), this, SLOT(onReadReady(QModbusReply*, int, int)));
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
    if (auto *replyOne = relayOne->sendReadRequest(*relayOneUnitOne, relayOneAdress)) {
        if (!replyOne->isFinished())
            connect(replyOne, &QModbusReply::finished, this, [this, replyOne](){
                emit readFinished(replyOne, 0, 0);  // read fiinished connects to ReadReady()
            });
        else
            delete replyOne; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + relayOne->errorString(), 5000);
    }
    if (auto *replyTwo = relayOne->sendReadRequest(*relayOneUnitTwo, relayOneAdress)) {
        if (!replyTwo->isFinished())
            connect(replyTwo, &QModbusReply::finished, this, [this, replyTwo](){
                emit readFinished(replyTwo, 0, 1);
            });
        else
            delete replyTwo; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + relayOne->errorString(), 5000);
    }
    if (auto *replyThree = relayOne->sendReadRequest(*relayOneUnitThree, relayOneAdress)) {
        if (!replyThree->isFinished())
            connect(replyThree, &QModbusReply::finished, this, [this, replyThree](){
                emit readFinished(replyThree, 0, 2);
            });
        else
            delete replyThree; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + relayOne->errorString(), 5000);
    }
    if (auto *replyFour = relayTwo->sendReadRequest(*relayTwoUnitOne, relayTwoAdress)) {
        if (!replyFour->isFinished())
            connect(replyFour, &QModbusReply::finished, this, [this, replyFour](){
                emit readFinished(replyFour, 1, 0);
            });
        else
            delete replyFour; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + relayOne->errorString(), 5000);
    }
    if (auto *replyFive = relayTwo->sendReadRequest(*relayTwoUnitTwo, relayTwoAdress)) {
        if (!replyFive->isFinished())
            connect(replyFive, &QModbusReply::finished, this, [this, replyFive](){
                emit readFinished(replyFive, 1, 1);
            });
        else
            delete replyFive; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + relayOne->errorString(), 5000);
    }
    // update GUI
}

void MainWindow::onReadReady(QModbusReply* reply, int relayId, int registerPackId){  // relayOne id = 0;
    if (!reply)
        return;

    if (reply->error() == QModbusDevice::NoError) {
        const QModbusDataUnit unit = reply->result();
        if (relayId == 0){
            if (registerPackId == 0)
                for (int i = 0; i < 10; ++i)
                    relayOneInputs[i] = unit.value(i);
            if (registerPackId == 1)
                for (int i = 10; i < 16; ++i)
                    relayOneInputs[i] = unit.value(i-10);
            if (registerPackId == 2)
                for (int i = 16; i < 24; ++i)
                    relayOneInputs[i] = unit.value(i-16);
        }
        else{
            if (registerPackId == 0)
                for (int i = 0; i < 10; ++i)
                    relayTwoInputs[i] = unit.value(i);
            if (registerPackId == 1)
                for (int i = 10; i < 16; ++i)
                    relayTwoInputs[i] = unit.value(i-10);
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

void MainWindow::writeRelayInput(int relayId, int registerAdress, bool value){
    statusBar()->clearMessage();

    QModbusDataUnit *writeUnit = new QModbusDataUnit(QModbusDataUnit::Coils, registerAdress, 1);
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
