#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    for (int i = 0; i < 24; ++i){
        relayOneOutputs[i] = 0;
        relayOneInputs[i] = 0;
    }
    for (int i = 0; i < 16; ++i){
        relayTwoOutputs[i] = 0;
        relayTwoInputs[i] = 0;
    }
    for (int i = 0; i < 8; ++i){
        relayOneInputSensors[i] = 0;
    }
    tmpSampleInfo.resize(7);
    for (int i = 0; i < 7; ++i){
        tmpSampleInfo[i] = "-";
    }
    errorDialog = new errorConnectionDialog(this);
    modbusMaster = new QModbusRtuSerialMaster(this);
    connect(modbusMaster, &QModbusClient::errorOccurred, [this](QModbusDevice::Error) {
        statusBar()->showMessage(modbusMaster->errorString(), 5000);
    });
    if (!modbusMaster) {
        statusBar()->showMessage(tr("Could not create Modbus master."), 5000);
    }
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, ".");
    connectionSettings = new QSettings("connectionSettings.ini", QSettings::IniFormat);
    readIniToModbusDevice();
    relayOneMBUnit = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 20, 4);
    relayTwoMBUnit = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 20, 4);
    connect(this, SIGNAL(readFinished(QModbusReply*, int)), this, SLOT(onReadReady(QModbusReply*, int)));
    // setup timer for readRelaysOutputs
    readOutputsTimer = new QTimer(this);
    connect(readOutputsTimer, SIGNAL(timeout()), this, SLOT(readRelaysOutputs()));
    readOutputsTimer->start(1000);
    emergencyReturnTimer = new QTimer(this);
    connect(emergencyReturnTimer, SIGNAL(timeout()), this, SLOT(emergencyReturnOff()));
    ui->buttonBox->setVisible(false);
    updateGuiOutputs();
    // connecting to DB
    if (createDbConnection()){
        say("Database connected");
        dbConnection = true;
    }
    else{
        say("Cannot connect to database!");
        QSqlDatabase deebee = QSqlDatabase::database("NAA_db");
        say(deebee.lastError().text());
        dbConnection = false;
    }
}

MainWindow::~MainWindow()
{
    readOutputsTimer->stop();
    if (modbusMaster)
        modbusMaster->disconnectDevice();
    delete modbusMaster;
    delete ui;
}

void MainWindow::readIniToModbusDevice(){
    statusBar()->clearMessage();
    modbusMaster->setConnectionParameter(QModbusDevice::SerialPortNameParameter, connectionSettings->value("Port1", 0));
    relayOneAdress = connectionSettings->value("Adress1", 0).toInt();
    relayTwoAdress = connectionSettings->value("Adress2", 0).toInt();
    modbusMaster->setConnectionParameter(QModbusDevice::SerialParityParameter, connectionSettings->value("Parity", 0).toInt());
    modbusMaster->setConnectionParameter(QModbusDevice::SerialBaudRateParameter, connectionSettings->value("BaudRate", 0).toInt());
    modbusMaster->setConnectionParameter(QModbusDevice::SerialDataBitsParameter, connectionSettings->value("DataBits", 0).toInt());
    modbusMaster->setConnectionParameter(QModbusDevice::SerialStopBitsParameter, connectionSettings->value("StopBits", 0).toInt());
    modbusMaster->setTimeout(connectionSettings->value("Timeout", 0).toInt());
    modbusMaster->setNumberOfRetries(connectionSettings->value("NumberOfRetries", 0).toInt());
    DBserver = connectionSettings->value("Server", 0).toString();
    DBname = connectionSettings->value("DatabaseName", 0).toString();
    DBuser = connectionSettings->value("User", 0).toString();
    DBpwd = connectionSettings->value("Password", 0).toString();
}

void MainWindow::readRelaysOutputs(){
    statusBar()->clearMessage();
    if (auto *replyOne = modbusMaster->sendReadRequest(*relayOneMBUnit, relayOneAdress)) {
        if (!replyOne->isFinished())
            connect(replyOne, &QModbusReply::finished, this, [this, replyOne](){
                emit readFinished(replyOne, 0);  // read fiinished connects to ReadReady()
            });
        else
            delete replyOne; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbusMaster->errorString(), 5000);
    }
    if (auto *replyTwo = modbusMaster->sendReadRequest(*relayTwoMBUnit, relayTwoAdress)) {
        if (!replyTwo->isFinished())
            connect(replyTwo, &QModbusReply::finished, this, [this, replyTwo](){
                emit readFinished(replyTwo, 1);
            });
        else
            delete replyTwo; // broadcast replies return immediately
    } else {
        statusBar()->showMessage(tr("Read error: ") + modbusMaster->errorString(), 5000);
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
            for (int i = 0; i < 8; ++i){
                relayOneInputSensors[i] = value % 2;
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
        reply = modbusMaster->sendWriteRequest(*writeUnit, relayOneAdress);
    else
        reply = modbusMaster->sendWriteRequest(*writeUnit, relayTwoAdress);
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
    irradiationElapsedInSec = 0;
    ui->N1Button->setDown(relayOneOutputs[10]);
    ui->N2Button->setDown(relayOneOutputs[11]);
    ui->GButton->setDown(relayOneOutputs[12]);
    ui->N1BarOne->setValue((relayOneOutputs[1] & relayOneOutputs[10])*100);
    ui->N2BarOne->setValue((relayOneOutputs[1] & relayOneOutputs[11])*100);
    ui->GBarOne->setValue((relayOneOutputs[1] & relayOneOutputs[12])*100);
    ui->N1BarTwo->setValue((relayOneOutputs[2] & relayOneOutputs[10])*100);
    ui->N2BarTwo->setValue((relayOneOutputs[2] & relayOneOutputs[11])*100);
    ui->GBarTwo->setValue((relayOneOutputs[2] & relayOneOutputs[12])*100);
    ui->dozPostButton->setEnabled(relayOneOutputs[15]);
    ui->proboDropButton->setEnabled(relayTwoOutputs[4]);
    ui->pressureOkLed->setState(relayOneInputSensors[0]);
    ui->containerLed->setState(!relayOneInputSensors[1]);
    ui->uzvClosedLed->setState(!relayOneInputSensors[2]);
    ui->probotekaLed->setState(relayOneInputSensors[3]);
    ui->uzvPressureLed->setState(!relayOneInputSensors[4]);
    ui->activeZoneLedG->setState(relayOneInputSensors[5]);
    ui->activeZoneLedN2->setState(relayOneInputSensors[6]);
    ui->activeZoneLedN1->setState(relayOneInputSensors[7]);
    // elapsedTimeCalculation and check is irradiation started
    if (!relayOneInputSensors[1] && dbConnection){   // first one is the "container here inda UZV"
        ui->sampleChooseButton->setEnabled(true);
        ui->sampleResetButton->setEnabled(true);
    }
    else{
        ui->sampleChooseButton->setEnabled(false);
        ui->sampleResetButton->setEnabled(false);
    }
    if (relayOneOutputs[10]){
        irradiationElapsedInSec = N1Sample.getTimeElapsedInSec();
        if (!N1Sample.isOnChannel() && relayOneInputSensors[7]){
            N1Sample.setBeginDT();
            say("N1 sample is begin to irradiate ");
        }
    }
    if (relayOneOutputs[11]){
        irradiationElapsedInSec = N2Sample.getTimeElapsedInSec();
        if (!N2Sample.isOnChannel() && relayOneInputSensors[6]){
            N2Sample.setBeginDT();
            say("N2 sample is begin to irradiate ");
        }
    }
    if (relayOneOutputs[12]){
        irradiationElapsedInSec = GSample.getTimeElapsedInSec();
        if (!GSample.isOnChannel() && relayOneInputSensors[5]){
            GSample.setBeginDT();
            say("G sample is begin to irradiate ");
        }
    }
    ui->getDaysSpinBox->setValue(irradiationElapsedInSec/86400);
    irradiationElapsedInSec = irradiationElapsedInSec%86400;
    ui->getHoursSpinBox->setValue(irradiationElapsedInSec/3600);
    irradiationElapsedInSec = irradiationElapsedInSec%3600;
    ui->getMinutesSpinBox->setValue(irradiationElapsedInSec/60);
    ui->getSecondsSpinBox->setValue(irradiationElapsedInSec%60);
    // is it time for auto return?
    if (N1Sample.isIrradiationDone()){
        writeRelayInput(0, 5, 1);   // n1 button pressed
        QTimer::singleShot(1000, this, SLOT(timeToAutoReturnN1())); // test it! timer delay especially
        writeRelayInput(0, 5, 0);
    }else
    if (N2Sample.isIrradiationDone()){
        writeRelayInput(0, 6, 1);   // n2 button pressed
        QTimer::singleShot(1000, this, SLOT(timeToAutoReturnN2()));
        writeRelayInput(0, 6, 0);
    }else
    if (GSample.isIrradiationDone()){
        writeRelayInput(0, 7, 1);   // G button pressed
        QTimer::singleShot(1000, this, SLOT(timeToAutoReturnG()));
        writeRelayInput(0, 7, 0);
    }
}
void MainWindow::timeToAutoReturnN1(){
    if (!relayOneOutputs[10])
        say("Cannot set N1 path, please check the conditions. Repeating...");
    else{
        updateGuiSampleInfo();
        writeRelayInput(0, 12, 1);  // return button
        QTimer::singleShot(3000, this, SLOT(checkAutoReturnN1()));
        writeRelayInput(0, 12, 0);  // unbutton return button
    }
}
void MainWindow::timeToAutoReturnN2(){
    if (!relayOneOutputs[11])
        say("Cannot set N2 path, please check the conditions. Repeating...");
    else{
        updateGuiSampleInfo();
        writeRelayInput(0, 12, 1);  // return button
        QTimer::singleShot(3000, this, SLOT(checkAutoReturnN2()));
        writeRelayInput(0, 12, 0);  // unbutton return button
    }
}
void MainWindow::timeToAutoReturnG(){
    if (!relayOneOutputs[12])
        say("Cannot set G path, please check the conditions. Repeating...");
    else{
        updateGuiSampleInfo();
        writeRelayInput(0, 12, 1);  // return button
        QTimer::singleShot(3000, this, SLOT(checkAutoReturnG()));
        writeRelayInput(0, 12, 0);  // unbutton return button
    }
}

void MainWindow::checkAutoReturnN1(){
    if (N1Sample.isOnChannel()){
        if (!relayOneInputSensors[7]){
            N1Sample.setEndDT();
            say("N1 sample irradiation ended");
        }
        else{
            say("Cannot return N1 sample, please check the conditions. Repeating...");
        }
    }
}
void MainWindow::checkAutoReturnN2(){
    if (N2Sample.isOnChannel()){
        if (!relayOneInputSensors[6]){
            N2Sample.setEndDT();
            say("N2 sample irradiation ended");
        }
        else{
            say("Cannot return N2 sample, please check the conditions. Repeating...");
        }
    }
}
void MainWindow::checkAutoReturnG(){
    if (GSample.isOnChannel()){
        if (!relayOneInputSensors[5]){
            GSample.setEndDT();
            say("G sample irradiation ended");
        }
        else{
            say("Cannot return G sample, please check the conditions. Repeating...");
        }
    }
}
void MainWindow::updateGuiSampleInfo(){
    irradiationDurationInSec = 0;
    if (relayOneOutputs[10]){
        tmpSampleInfo = N1Sample.getName();
        irradiationDurationInSec = N1Sample.getIrradiationDurationInSec();
    }
    if (relayOneOutputs[11]){
        tmpSampleInfo = N2Sample.getName();
        irradiationDurationInSec = N2Sample.getIrradiationDurationInSec();
    }
    if (relayOneOutputs[12]){
        tmpSampleInfo = GSample.getName();
        irradiationDurationInSec = GSample.getIrradiationDurationInSec();
    }
    ui->sampleName->setText(tmpSampleInfo.at(0) + " " + tmpSampleInfo.at(1) + "-" + tmpSampleInfo.at(1) + " №" + tmpSampleInfo.at(4) + "-" + tmpSampleInfo.at(5));
    ui->setDaysSpinBox->setValue(irradiationDurationInSec/86400);
    irradiationDurationInSec = irradiationDurationInSec%86400; // check this one!!
    ui->setHoursSpinBox->setValue(irradiationDurationInSec/3600);
    irradiationDurationInSec = irradiationDurationInSec%3600;
    ui->setMinutesSpinBox->setValue(irradiationDurationInSec/60);
    ui->setSecondsSpinBox->setValue(irradiationDurationInSec%60);
}
void MainWindow::calculateIrradiationDuration(){
    irradiationDurationInSec = 0;
    irradiationDurationInSec += ui->setDaysSpinBox->value()*86400;
    irradiationDurationInSec += ui->setHoursSpinBox->value()*3600;
    irradiationDurationInSec += ui->setMinutesSpinBox->value()*60;
    irradiationDurationInSec += ui->setSecondsSpinBox->value();
}
void MainWindow::writeRelayInput(int relayId, int input, bool value){
    //mb add some safety here
    //write to outputs bitmask, calculate register value, call da function
    int registerValue = 0;
    if (relayId == 0){
        relayOneInputs[input] = value;
        if (input < 16){
            for (int i = 0; i < 16; ++i){
                if (relayOneInputs[i])
                    registerValue += pow(2,i);
            }
            writeRelayRegister(0,16,registerValue);
        }
        else{
            for (int i = 16; i < 24; ++i){
                if (relayOneInputs[i])
                    registerValue += pow(2,(i-16));
            }
            writeRelayRegister(0, 17, registerValue);
        }
    }
    else{
        relayTwoInputs[input] = value;
        for (int i = 0; i < 16; ++i){
            if (relayTwoInputs[i]){
                registerValue += pow(2,i);
            }
        }
        writeRelayRegister(1, 16, registerValue);
    }
}

bool MainWindow::isIrradiationTimeAppropriate(){
    calculateIrradiationDuration();
    if (irradiationDurationInSec == 0){
        say("Irradiation duration time should not be null");
        return false;
    }
    qint64 nowSec = QDateTime::currentDateTime().toSecsSinceEpoch();
    if (relayOneOutputs[10]){
        if (N2Sample.isOnChannel()){
            if ( (irradiationDurationInSec + nowSec) > (nowSec - N2Sample.getTimeElapsedInSec() + N2Sample.getIrradiationDurationInSec() - 120) && (irradiationDurationInSec + nowSec) < (nowSec - N2Sample.getTimeElapsedInSec() + N2Sample.getIrradiationDurationInSec() + 120) ){
                say("Cannot set that irradiation time to N1 path sample. It's too close to irradiation endtime of N2 path sample, please wait a few minutes or change irradiation duration");
                return false;
            }
            // if smth gonna be returned shortly ->
            if ((nowSec - N2Sample.getTimeElapsedInSec() + N2Sample.getIrradiationDurationInSec()) < nowSec + 120){
                say("Cannot send sample right now. N2 path sample going to be returned soon. Please wait a few minutes or return it now");
                return false;
            }
        }
        if (GSample.isOnChannel()){
            if ( (irradiationDurationInSec + nowSec) > (nowSec - GSample.getTimeElapsedInSec() + GSample.getIrradiationDurationInSec() - 120) && (irradiationDurationInSec + nowSec) < (nowSec - GSample.getTimeElapsedInSec() + GSample.getIrradiationDurationInSec() + 120) ){
                say("Cannot set that irradiation time to N1 path sample. It's too close to irradiation endtime of G path sample, please wait a few minutes or change irradiation duration");
                return false;
            }
            if ((nowSec - GSample.getTimeElapsedInSec() + GSample.getIrradiationDurationInSec()) < nowSec + 120){
                say("Cannot send sample right now. G path sample going to be returned soon. Please wait a few minutes or return it now");
                return false;
            }
        }
    }
    if (relayOneOutputs[11]){
        if (N1Sample.isOnChannel()){
            if ( (irradiationDurationInSec + nowSec) > (nowSec - N1Sample.getTimeElapsedInSec() + N1Sample.getIrradiationDurationInSec() - 120) && (irradiationDurationInSec + nowSec) < (nowSec - N1Sample.getTimeElapsedInSec() + N1Sample.getIrradiationDurationInSec() + 120) ){
                say("Cannot set that irradiation time to N2 path sample. It's too close to irradiation endtime of N1 path sample, please wait a few minutes or change irradiation duration");
                return false;
            }
            if ((nowSec - N1Sample.getTimeElapsedInSec() + N1Sample.getIrradiationDurationInSec()) < nowSec + 120){
                say("Cannot send sample right now. N1 path sample going to be returned soon. Please wait a few minutes or return it now");
                return false;
            }
        }
        if (GSample.isOnChannel()){
            if ( (irradiationDurationInSec + nowSec) > (nowSec - GSample.getTimeElapsedInSec() + GSample.getIrradiationDurationInSec() - 120) && (irradiationDurationInSec + nowSec) < (nowSec - GSample.getTimeElapsedInSec() + GSample.getIrradiationDurationInSec() + 120) ){
                say("Cannot set that irradiation time to N2 path sample. It's too close to irradiation endtime of G path sample, please wait a few minutes or change irradiation duration");
                return false;
            }
            if ((nowSec - GSample.getTimeElapsedInSec() + GSample.getIrradiationDurationInSec()) < nowSec + 120){
                say("Cannot send sample right now. G path sample going to be returned soon. Please wait a few minutes or return it now");
                return false;
            }
        }
    }
    if (relayOneOutputs[12]){
        if (N1Sample.isOnChannel()){
            if ( (irradiationDurationInSec + nowSec) > (nowSec - N1Sample.getTimeElapsedInSec() + N1Sample.getIrradiationDurationInSec() - 120) && (irradiationDurationInSec + nowSec) < (nowSec - N1Sample.getTimeElapsedInSec() + N1Sample.getIrradiationDurationInSec() + 120) ){
                say("Cannot set that irradiation time to G path sample. It's too close to irradiation endtime of N1 path sample, please wait a few minutes or change irradiation duration");
                return false;
            }
            if ((nowSec - N1Sample.getTimeElapsedInSec() + N1Sample.getIrradiationDurationInSec()) < nowSec + 120){
                say("Cannot send sample right now. N1 path sample going to be returned soon. Please wait a few minutes or return it now");
                return false;
            }
        }
        if (N2Sample.isOnChannel()){
            if ( (irradiationDurationInSec + nowSec) > (nowSec - N2Sample.getTimeElapsedInSec() + N2Sample.getIrradiationDurationInSec() - 120) && (irradiationDurationInSec + nowSec) < (nowSec - N2Sample.getTimeElapsedInSec() + N2Sample.getIrradiationDurationInSec() + 120) ){
                say("Cannot set that irradiation time to G path sample. It's too close to irradiation endtime of N2 path sample, please wait a few minutes or change irradiation duration");
                return false;
            }
            if ((nowSec - N2Sample.getTimeElapsedInSec() + N2Sample.getIrradiationDurationInSec()) < nowSec + 120){
                say("Cannot send sample right now. N2 path sample going to be returned soon. Please wait a few minutes or return it now");
                return false;
            }
        }
    }
    return true;
}

bool MainWindow::createDbConnection(){
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC3", "NAA_db");
    //db.setHostName("DESKTOP-S3A0AFC\\CITADEL");
    db.setDatabaseName(QString("DRIVER={SQL Server};"
                               "SERVER=" + DBserver + ";DATABASE=" + DBname + ";Persist Security Info=true;"
                               "uid=" + DBuser + ";pwd=" + DBpwd));
    //db.setUserName("testUser");
    //db.setPassword("123123");
    return db.open();
}
void MainWindow::say(QString text){
    ui->textBrowser->append(QDateTime::currentDateTime().time().toString() + " " + text);
}
void MainWindow::on_N1Button_pressed()
{
    writeRelayInput(0, 5, 1);
}


void MainWindow::on_N1Button_released()
{
    writeRelayInput(0, 5, 0);
    updateGuiSampleInfo();
}


void MainWindow::on_N2Button_pressed()
{
    writeRelayInput(0, 6, 1);
}


void MainWindow::on_N2Button_released()
{
    writeRelayInput(0, 6, 0);
    updateGuiSampleInfo();
}


void MainWindow::on_GButton_pressed()
{
    writeRelayInput(0, 7, 1);
}


void MainWindow::on_GButton_released()
{
    writeRelayInput(0, 7, 0);
    updateGuiSampleInfo();
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
    if (isIrradiationTimeAppropriate())
        writeRelayInput(0, 9, 1);
    else
        say("Cannot send sample");
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

void MainWindow::on_returnButton_pressed()
{
    writeRelayInput(0, 12, 1);
    if (relayOneOutputs[10])
        QTimer::singleShot(3000, this, SLOT(checkAutoReturn(IRCH_N1)));
    if (relayOneOutputs[11])
        QTimer::singleShot(3000, this, SLOT(checkAutoReturn(IRCH_N2)));
    if (relayOneOutputs[12])
        QTimer::singleShot(3000, this, SLOT(checkAutoReturn(IRCH_G)));
}

void MainWindow::on_returnButton_released()
{
    writeRelayInput(0, 12, 0);
}

void MainWindow::on_emergencyDozPostButton_pressed()
{
    writeRelayInput(0, 11, 1);
}

void MainWindow::on_emergencyDozPostButton_released()
{
    writeRelayInput(0, 11, 0);
}

void MainWindow::on_emergencyReturnButton_pressed()
{
    writeRelayInput(0, 10, 1);
    on_returnButton_pressed();
    emergencyReturnTimer->start(1000);
    //timah
}

void MainWindow::emergencyReturnOff()
{
    writeRelayInput(0, 10, 0); // "unbutton" emergency button
    on_returnButton_released();
    emergencyReturnTimer->stop();
}

void MainWindow::on_buttonBox_accepted()
{
    if (isIrradiationTimeAppropriate()){
        if (relayOneOutputs[10]){
            N1Sample.setSetDT(irradiationDurationInSec);
        }
        if (relayOneOutputs[11]){
            N2Sample.setSetDT(irradiationDurationInSec);
        }
        if (relayOneOutputs[12]){
            GSample.setSetDT(irradiationDurationInSec);
        }
    }
    else
        updateGuiSampleInfo();
    ui->buttonBox->setVisible(false);
}

void MainWindow::on_buttonBox_rejected()
{
    updateGuiSampleInfo();
    ui->buttonBox->setVisible(false);
}

void MainWindow::on_setDaysSpinBox_valueChanged(int arg1)
{
    ui->buttonBox->setVisible(true);
}

void MainWindow::on_setHoursSpinBox_valueChanged(int arg1)
{
    ui->buttonBox->setVisible(true);
}

void MainWindow::on_setMinutesSpinBox_valueChanged(int arg1)
{
    ui->buttonBox->setVisible(true);
}

void MainWindow::on_setSecondsSpinBox_valueChanged(int arg1)
{
    ui->buttonBox->setVisible(true);
}

void MainWindow::on_sampleChooseButton_clicked()
{
    sampleJournal *journal = new sampleJournal();
    QObject::connect(journal, SIGNAL(sampleChoosen(QVector<QString>)), this, SLOT(readSampleInfo(QVector<QString>)));
    journal->show();
}

void MainWindow::readSampleInfo(QVector<QString> sampleInfo){
    if (relayOneOutputs[10]){
        if (sampleInfo != N2Sample.getName() && sampleInfo != GSample.getName()){
            N1Sample.setName(sampleInfo);
        }
        else
            say("Cannot choose that sample. It's already choosen at other path");
    }
    if (relayOneOutputs[11]){
        if (sampleInfo != N1Sample.getName() && sampleInfo != GSample.getName()){
            N2Sample.setName(sampleInfo);
        }
        else
            say("Cannot choose that sample. It's already choosen at other path");
    }
    if (relayOneOutputs[12]){
        if (sampleInfo != N1Sample.getName() && sampleInfo != N2Sample.getName()){
            GSample.setName(sampleInfo);
        }
        else
            say("Cannot choose that sample. It's already choosen at other path");
    }
    updateGuiSampleInfo();
}

void MainWindow::on_testStartButton_clicked()
{
    if (relayOneOutputs[10]){
        N1Sample.setBeginDT();
    }
    if (relayOneOutputs[11]){
        N2Sample.setBeginDT();
    }
    if (relayOneOutputs[12]){
        GSample.setBeginDT();
    }
}

void MainWindow::on_testPathComboBox_currentIndexChanged(const QString &arg1)
{
    relayOneOutputs[10] = (arg1 == "N1") ? 1 : 0;
    relayOneOutputs[11] = (arg1 == "N2") ? 1 : 0;
    relayOneOutputs[12] = (arg1 == "G") ? 1 : 0;
    updateGuiSampleInfo();
}

void MainWindow::on_testEndButton_clicked()
{
    if (relayOneOutputs[10]){
        N1Sample.setEndDT();
    }
    if (relayOneOutputs[11]){
        N2Sample.setEndDT();
    }
    if (relayOneOutputs[12]){
        GSample.setEndDT();
    }
}

void MainWindow::on_sampleResetButton_clicked()
{
    for (int i = 0; i < 7; ++i){
        tmpSampleInfo[i] = "-";
    }
    if (relayOneOutputs[10]){
        N1Sample.setName(tmpSampleInfo);
    }
    if (relayOneOutputs[11]){
        N2Sample.setName(tmpSampleInfo);
    }
    if (relayOneOutputs[12]){
        GSample.setName(tmpSampleInfo);
    }
    updateGuiSampleInfo();
}

void MainWindow::on_returnButton_2_toggled(bool checked)
{
    if (checked){
        if (!m_translator.isEmpty())
            QCoreApplication::removeTranslator(&m_translator);
    }
    else{
        m_translator.load("PanelRegata2_ru_RU.qm");
        QCoreApplication::installTranslator(&m_translator);
        QQmlEngine::contextForObject(this)->engine()->retranslate();
    }
}
