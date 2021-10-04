#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    for (int i = 0; i < 24; ++i)
        relayOneOutputs[i] = 0;
    for (int i = 0; i < 16; ++i)
        relayTwoOutputs[i] = 0;
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
//        errorDialog->show();
        return;
        // show error message and exit
    }
    if (!relayTwo->connectDevice()) {
        statusBar()->showMessage(tr("Connect failed: ") + relayTwo->errorString(), 5000);
//        errorDialog->show();
        return;
        // show error message and exit
    }
    relayOneMBUnit = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 20, 4);
    relayTwoMBUnit = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 20, 4);
    connect(this, SIGNAL(readFinished(QModbusReply*, int)), this, SLOT(onReadReady(QModbusReply*, int)));
    // setup timer for readRelaysOutputs
    readOutputsTimer = new QTimer(this);
    connect(readOutputsTimer, SIGNAL(timeout()), this, SLOT(readRelaysOutputs()));
    readOutputsTimer->start(1000);
}

MainWindow::~MainWindow()
{
    readOutputsTimer->stop();
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

void MainWindow::readRelaysOutputs(){
    statusBar()->clearMessage();
    if (auto *replyOne = relayOne->sendReadRequest(*relayOneMBUnit, relayOneAdress)) {
        if (!replyOne->isFinished())
            connect(replyOne, &QModbusReply::finished, this, [this, replyOne](){
                emit readFinished(replyOne, 0);  // read fiinished connects to ReadReady()
            });
        else
            delete replyOne; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + relayOne->errorString(), 5000);
    }
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
    updateGuiOutputs();
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
                relayOneOutputs[i] = value % 2;
                value = value / 2;
            }
            value = unit.value(1);
            for (int i = 16; i < 24; ++i){
                relayOneOutputs[i] = value % 2;
                value = value / 2;
            }
        }
        else{                       // thee other one
            value = unit.value(0);
            for (int i = 0; i < 16; ++i){
                relayTwoOutputs[i] = value % 2;
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
void MainWindow::updateGuiOutputs(){
    ui->N1Button->setDown(relayOneOutputs[10]);
    ui->N2Button->setDown(relayOneOutputs[11]);
    ui->GButton->setDown(relayOneOutputs[12]);
    //this->ui->relayTwoI1->setChecked(readRelaysOutputs[0]);
    //this->ui->relayTwoI2->setChecked(readRelaysOutputs[1]);
    //this->ui->relayTwoI3->setChecked(readRelaysOutputs[2]);
}
void MainWindow::writeRelayInput(int relayId, int input, bool value){
    //mb add some safety here
    //write to outputs bitmask, calculate register value, call da function
    int registerValue = 0;
    if (relayId == 0){
        relayOneInputs[input] = value;
        if (input < 16){
            if (relayOneInputs[0])
                registerValue += 1;
            for (int i = 1; i < 16; ++i){
                if (relayOneInputs[i])
                    registerValue += i * 2;
            }
            writeRelayRegister(0,16,registerValue);
        }
        else{
            if (relayOneInputs[16])
                registerValue += 1;
            for (int i = 17; i < 24; ++i){
                if (relayOneInputs[i])
                    registerValue += (i-16) * 2;
            }
            writeRelayRegister(0, 17, registerValue);
        }
    }
    else{
        relayTwoInputs[input] = value;
        if (relayTwoInputs[0])
            registerValue += 1;
        for (int i = 1; i < 16; ++i){
            if (relayTwoInputs[i])
                registerValue += i * 2;
        }
        writeRelayRegister(1, 16, registerValue);
    }
}
//void MainWindow::on_relayTwoO1_stateChanged(int arg1)
//{
//    /*writeRelayRegister(1, 16, arg1);
//    qDebug() << " arg1 " << arg1;*/
//    writeRelayOutput(1,0,(bool)arg1);
//}


void MainWindow::on_N1Button_pressed()
{
    writeRelayInput(0, 5, 1);
}


void MainWindow::on_N1Button_released()
{
    writeRelayInput(0, 5, 0);
}


void MainWindow::on_N2Button_pressed()
{
    writeRelayInput(0, 6, 1);
}


void MainWindow::on_N2Button_released()
{
    writeRelayInput(0, 6, 0);
}


void MainWindow::on_GButton_pressed()
{
    writeRelayInput(0, 7, 1);
}


void MainWindow::on_GButton_released()
{
    writeRelayInput(0, 7, 0);
}


void MainWindow::on_dozPostButton_pressed()
{
    writeRelayInput(0, 8, 1);
}


void MainWindow::on_dozPostButton_released()
{
    writeRelayInput(0, 8, 0);
}


void MainWindow::on_startButton_pressed()
{
    writeRelayInput(0, 9, 1);
}


void MainWindow::on_startButton_released()
{
    writeRelayInput(0, 9, 0);
}


void MainWindow::on_radioButton_toggled(bool checked)
{
    writeRelayInput(1, 0, checked);
}

void MainWindow::on_radioButton_2_toggled(bool checked)
{
    writeRelayInput(1, 1, checked);
}

void MainWindow::on_radioButton_3_toggled(bool checked)
{
    writeRelayInput(1, 2, checked);
}

void MainWindow::on_radioButton_4_toggled(bool checked)
{
    writeRelayInput(1, 3, checked);
}

void MainWindow::on_radioButton_5_toggled(bool checked)
{
    writeRelayInput(1, 4, checked);
}

void MainWindow::on_radioButton_6_toggled(bool checked)
{
    writeRelayInput(1, 5, checked);
}

void MainWindow::on_radioButton_7_toggled(bool checked)
{
    writeRelayInput(1, 6, checked);
}

void MainWindow::on_radioButton_8_toggled(bool checked)
{
    writeRelayInput(1, 7, checked);
}

void MainWindow::on_probotekaButton_toggled(bool checked)
{
    writeRelayInput(1, 8, checked);
}

void MainWindow::on_proboDropButton_pressed()
{
    writeRelayInput(1, 9, 1);
}

void MainWindow::on_proboDropButton_released()
{
    writeRelayInput(1, 9, 0);
}
