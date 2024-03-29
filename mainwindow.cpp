#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->dozPostButton->setEnabled(false);
    ui->dozPostButton->setFlat(false);
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
    if (!modbusMaster->connectDevice()) {
        statusBar()->showMessage(tr("Connect failed: ") + modbusMaster->errorString(), 5000);
        errorDialog->show();
        return;
        // show error message and exit
    }
    relayOneMBUnit = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 20, 4);
    relayTwoMBUnit = new QModbusDataUnit(QModbusDataUnit::HoldingRegisters, 20, 4);
    gammaMBUnit = new QModbusDataUnit(QModbusDataUnit::InputRegisters, 0, 2);
    connect(this, SIGNAL(readFinished(QModbusReply*, int)), this, SLOT(onReadReady(QModbusReply*, int)));
    // setup timer for readRelaysOutputs
    readOutputsTimer = new QTimer(this);
    emergencyReturnTimer = new QTimer(this);
    dozeTimer = new QTimer(this);
    autoReturnTimer = new QTimer(this);
    connect(readOutputsTimer, SIGNAL(timeout()), this, SLOT(readRelaysOutputs()));
    connect(emergencyReturnTimer, SIGNAL(timeout()), this, SLOT(emergencyReturnOff()));
    connect(dozeTimer, SIGNAL(timeout()), this, SLOT(checkDoze()));
    readOutputsTimer->start(1000);
    connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(isIrradiationDoneCycle())); // fix here
    autoReturnTimer->start(1000);
    ui->buttonBox->setVisible(false);
    updateGuiOutputs();
    // connecting to DB
    if (createDbConnection()){
        if (engLang)
            say("Database connected");
        else
            say("База данных подключена");
        dbConnection = true;
    }
    else{
        if (engLang)
            say("Cannot connect to database!");
        else
            say("Невозможно подключиться к базе данных");
        QSqlDatabase deebee = QSqlDatabase::database("NAA_db");
        say(deebee.lastError().text());
        dbConnection = false;
    }
    N1Sample.setDbConnectionState(dbConnection);
    N2Sample.setDbConnectionState(dbConnection);
    GSample.setDbConnectionState(dbConnection);
    initEventFilter(ui->setDaysSpinBox);
    initEventFilter(ui->setHoursSpinBox);
    initEventFilter(ui->setMinutesSpinBox);
    initEventFilter(ui->setSecondsSpinBox);
    offPal.setColor(QPalette::Button, QColor("#ef5350"));
    onPal.setColor(QPalette::Button, QColor("#66bb6a"));
    QSqlDatabase db = QSqlDatabase::database("NAA_db");
    QSqlQuery query("SELECT Received_By FROM [NAA_DB].[dbo].[table_Received_By]", db);
    while (query.next()) {
        ui->experimenterComboBox->addItem(query.value(0).toString());
    }
}

void MainWindow::initEventFilter(QSpinBox *spinny){
    spinny->installEventFilter(this);
    QObjectList o_list = spinny->children();
    for(int i = 0; i < o_list.length(); i++)
    {
        QLineEdit *cast = qobject_cast<QLineEdit*>(o_list[i]);
        if(cast)
            cast->installEventFilter(this);
    }
}

bool MainWindow::eventFilter(QObject *obj, QEvent *event)
{
    if(event->type() == QEvent::MouseButtonPress)
    {
        if (keyboard){
            keyboard->close();
        }
        keyboard = new VirtualKeyboard();
        //connect(keyboard, SIGNAL(cancel()), this, SLOT(on_buttonBox_rejected()));
        //connect(keyboard, SIGNAL(ok()), this, SLOT(on_buttonBox_accepted()));
        keyboard->show();
        keyboard->connect(qobject_cast<QLineEdit*>(obj));
    }
    return false;
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
    gammaPanelAdress = connectionSettings->value("GammaPanelAdress", 5).toInt();
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
    maximumDoze = connectionSettings->value("GammaDoze",10).toInt();
    gammaTimer = connectionSettings->value("GammaTimer",5).toInt();
    if (connectionSettings->value("language", "ru").toString() == "en")
        engLang = true;
    else
        engLang = false;
    ui->languageButton->setChecked(engLang);
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
    if (auto *replyThree = modbusMaster->sendReadRequest(*gammaMBUnit, gammaPanelAdress)) {
        if (!replyThree->isFinished())
            connect(replyThree, &QModbusReply::finished, this, [this, replyThree](){
                emit readFinished(replyThree, 2);
            });
        else
            delete replyThree; // broadcast replies return immediately
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
        else if (relayId == 1) {                       // thee second one
            value = unit.value(0);
            for (int i = 0; i < 16; ++i){
                relayTwoOutputs[i] = value % 2;
                value = value / 2;
            }
        }
        else {
            unsigned short data[2];
            data[0] = unit.value(0);
            data[1] = unit.value(1);
            memcpy(&doze, data, 4);
            if (doze > 0 & doze < 0.1)
                doze = 0.1;
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
    if (relayOneOutputs[15] && !dozeTimer->isActive() && !dozeTimerAlready){
        dozeTimer->start(gammaTimer*1000);
    }
    if (ui->dozPostButton->isEnabled() && !relayOneInputSensors[4]) // air pressure is ok
        ui->dozPostButton->setPalette(onPal);
    else
        ui->dozPostButton->setPalette(offPal);
    ui->proboDropButton->setEnabled(relayTwoOutputs[4]);
    ui->pressureOkLed->setState(relayOneInputSensors[0]);
    ui->containerLed->setState(!relayOneInputSensors[1]);
    ui->uzvClosedLed->setState(!relayOneInputSensors[2]);
    ui->probotekaLed->setState(relayOneInputSensors[3]);
    ui->uzvPressureLed->setState(!relayOneInputSensors[4]);
    ui->activeZoneLedG->setState(relayOneInputSensors[5]);
    ui->activeZoneLedN2->setState(relayOneInputSensors[6]);
    ui->activeZoneLedN1->setState(relayOneInputSensors[7]);
    ui->label_3->setText(QString::number(doze, 'f', 1));
    if (relayOneInputSensors[3])
        dozeTimerAlready = false;
    if (relayOneInputSensors[0] && !relayOneInputSensors[1] && !relayOneInputSensors[2] && relayOneInputSensors[3] && !relayOneInputSensors[4] && !relayOneOutputs[9] && ((relayOneOutputs[10] && N1Sample.getIrradiationDurationInSec()) || (relayOneOutputs[11] && N2Sample.getIrradiationDurationInSec()) || (relayOneOutputs[12] && GSample.getIrradiationDurationInSec())) && isIrradiationTimeAppropriate(true))
        ui->startButton->setPalette(onPal);
    else
        ui->startButton->setPalette(offPal);
    if (relayOneInputSensors[0] && !relayOneInputSensors[1] && !relayOneInputSensors[2] && !relayOneInputSensors[4] && relayOneOutputs[9])
        ui->returnButton->setPalette(onPal);
    else
        ui->returnButton->setPalette(offPal);
    ui->startButton->update();
    ui->returnButton->update();
    // elapsedTimeCalculation and check is irradiation started
    /*if (!relayOneInputSensors[1] && dbConnection){   // first one is the "container here inda UZV"
        ui->sampleChooseButton->setEnabled(true);
        ui->sampleResetButton->setEnabled(true);
    }
    else{
        ui->sampleChooseButton->setEnabled(false);
        ui->sampleResetButton->setEnabled(false);
    }*/
    bool disableSomeGUI = false;
    if (relayOneOutputs[10]){
        irradiationElapsedInSec = N1Sample.getTimeElapsedInSec();
        if (!N1Sample.isOnChannel() && relayOneInputSensors[7]){
            N1Sample.setBeginDT();
            if (engLang)
                say("N1 sample is begin to irradiate ");
            else
                say("Начато облучение образца N1");
        }
        disableSomeGUI = N1Sample.isOnChannel();
    }
    if (relayOneOutputs[11]){
        irradiationElapsedInSec = N2Sample.getTimeElapsedInSec();
        if (!N2Sample.isOnChannel() && relayOneInputSensors[6]){
            N2Sample.setBeginDT();
            if (engLang)
                say("N2 sample is begin to irradiate ");
            else
                say("Начато облучение образца N2");
        }
        disableSomeGUI = N2Sample.isOnChannel();
    }
    if (relayOneOutputs[12]){
        irradiationElapsedInSec = GSample.getTimeElapsedInSec();
        if (!GSample.isOnChannel() && relayOneInputSensors[5]){
            GSample.setBeginDT();
            if (engLang)
                say("G sample is begin to irradiate ");
            else
                say("Начато облучение образца G");
        }
        disableSomeGUI = GSample.isOnChannel();
    }
    ui->sampleChooseButton->setEnabled(!disableSomeGUI);
    ui->sampleResetButton->setEnabled(!disableSomeGUI);
    ui->experimenterComboBox->setEnabled(!disableSomeGUI);
    ui->getDaysSpinBox->setValue(irradiationElapsedInSec/86400);
    irradiationElapsedInSec = irradiationElapsedInSec%86400;
    ui->getHoursSpinBox->setValue(irradiationElapsedInSec/3600);
    irradiationElapsedInSec = irradiationElapsedInSec%3600;
    ui->getMinutesSpinBox->setValue(irradiationElapsedInSec/60);
    ui->getSecondsSpinBox->setValue(irradiationElapsedInSec%60);
    // is it time for auto return?  
}
void MainWindow::isIrradiationDoneCycle(){
    if (N1Sample.isIrradiationAlmostDone() || N2Sample.isIrradiationAlmostDone() || GSample.isIrradiationAlmostDone())
        if (ui->dozPostButton->isEnabled()){
            on_dozPostButton_pressed();         // autodrop previous container
            on_dozPostButton_released();
        }
    if (N1Sample.isIrradiationDone()){
        writeRelayInput(0, 5, 1);   // n1 button pressed
        autoReturnTimer->disconnect();
        connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(timeToAutoReturnN1()));
        autoReturnTimer->start(3000);
        writeRelayInput(0, 5, 0);
    }else
    if (N2Sample.isIrradiationDone()){
        writeRelayInput(0, 6, 1);   // n2 button pressed
        autoReturnTimer->disconnect();
        connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(timeToAutoReturnN2()));
        autoReturnTimer->start(3000);
        writeRelayInput(0, 6, 0);
    }else
    if (GSample.isIrradiationDone()){
        writeRelayInput(0, 7, 1);   // G button pressed
        autoReturnTimer->disconnect();
        connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(timeToAutoReturnG()));
        autoReturnTimer->start(3000);
        writeRelayInput(0, 7, 0);
    }
}
void MainWindow::timeToAutoReturnN1(){
    if (!relayOneOutputs[10]){   // not N1 path
        writeRelayInput(0, 5, 1);   // n1 button pressed
        if (engLang)
            say("Cannot set N1 path, please check the conditions. Repeating...");
        else
            say("Невозможно выставить путь N1. Повтор...");
        writeRelayInput(0, 5, 0);
    }
    else{
        autoReturnTimer->stop();
        updateGuiSampleInfo();
        writeRelayInput(0, 12, 1);  // return button
        autoReturnTimer->disconnect();
        connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(checkAutoReturnN1()));
        autoReturnTimer->start(1000);
    }
}
void MainWindow::timeToAutoReturnN2(){
    if (!relayOneOutputs[11]){
        writeRelayInput(0, 6, 1);   // n2 button pressed
        if (engLang)
            say("Cannot set N2 path, please check the conditions. Repeating...");
        else
            say("Невозможно выставить путь N2. Повтор...");
        writeRelayInput(0, 6, 0);
    }
    else{
        autoReturnTimer->stop();
        updateGuiSampleInfo();
        writeRelayInput(0, 12, 1);  // return button
        autoReturnTimer->disconnect();
        connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(checkAutoReturnN2()));
        autoReturnTimer->start(1000);
    }
}
void MainWindow::timeToAutoReturnG(){
    if (!relayOneOutputs[12]){
        writeRelayInput(0, 7, 1);   // G button pressed
        if (engLang)
            say("Cannot set G path, please check the conditions. Repeating...");
        else
            say("Невозможно выставить путь G. Повтор...");
        writeRelayInput(0, 7, 0);
    }
    else{
        autoReturnTimer->stop();
        updateGuiSampleInfo();
        writeRelayInput(0, 12, 1);  // return button
        autoReturnTimer->disconnect();
        connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(checkAutoReturnG()));
        autoReturnTimer->start(1000);
    }
}

void MainWindow::checkAutoReturnN1(){
    if (N1Sample.isOnChannel()){
        if (!relayOneInputSensors[7]){     // облучается ли образец сейчас на канале н1? этот сигнал?
            writeRelayInput(0, 12, 0);  // unbutton return button
            N1Sample.setEndDT();
            autoReturnTimer->disconnect();
            connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(isIrradiationDoneCycle()));
            autoReturnTimer->start(1000);
            if (engLang)
                say("N1 sample irradiation ended");
            else
                say("Окончено облучение образца на пути N1");
        }
    }
}
void MainWindow::checkAutoReturnN2(){
    if (N2Sample.isOnChannel()){
        if (!relayOneInputSensors[6]){
            writeRelayInput(0, 12, 0);  // unbutton return button
            N2Sample.setEndDT();
            autoReturnTimer->disconnect();
            connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(isIrradiationDoneCycle()));
            autoReturnTimer->start(1000);
            if (engLang)
                say("N2 sample irradiation ended");
            else
                say("Окончено облучение образца на пути N2");
        }
    }
}
void MainWindow::checkAutoReturnG(){
    if (GSample.isOnChannel()){
        if (!relayOneInputSensors[5]){
            writeRelayInput(0, 12, 0);  // unbutton return button
            GSample.setEndDT();
            autoReturnTimer->disconnect();
            connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(isIrradiationDoneCycle()));
            autoReturnTimer->start(1000);
            if (engLang)
                say("G sample irradiation ended");
            else
                say("Окончено облучение образца на пути G");
        }
    }
}
void MainWindow::updateGuiSampleInfo(){
    irradiationDurationInSec = 0;
    if (relayOneOutputs[10]){
        tmpSampleInfo = N1Sample.getName();
        irradiationDurationInSec = N1Sample.getIrradiationDurationInSec();
        ui->experimenterComboBox->setCurrentText(N1Sample.getExperimenterName());
    }
    if (relayOneOutputs[11]){
        tmpSampleInfo = N2Sample.getName();
        irradiationDurationInSec = N2Sample.getIrradiationDurationInSec();
        ui->experimenterComboBox->setCurrentText(N2Sample.getExperimenterName());
    }
    if (relayOneOutputs[12]){
        tmpSampleInfo = GSample.getName();
        irradiationDurationInSec = GSample.getIrradiationDurationInSec();
        ui->experimenterComboBox->setCurrentText(GSample.getExperimenterName());
    }
    ui->sampleName->setText(tmpSampleInfo.at(0) + " " + tmpSampleInfo.at(1) + "-" + tmpSampleInfo.at(2) + "-" + tmpSampleInfo.at(3) + "-" + tmpSampleInfo.at(4) + "-" + tmpSampleInfo.at(5) + "-" + tmpSampleInfo.at(6));
    ui->setDaysSpinBox->setValue(irradiationDurationInSec/86400);
    irradiationDurationInSec = irradiationDurationInSec%86400; // check this one!!
    ui->setHoursSpinBox->setValue(irradiationDurationInSec/3600);
    irradiationDurationInSec = irradiationDurationInSec%3600;
    ui->setMinutesSpinBox->setValue(irradiationDurationInSec/60);
    ui->setSecondsSpinBox->setValue(irradiationDurationInSec%60);
    ui->buttonBox->setVisible(false);
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

bool MainWindow::isIrradiationTimeAppropriate(bool silent){
    calculateIrradiationDuration();
    if (irradiationDurationInSec < 70){
        if (engLang){
            if (!silent) say("Irradiation duration time should not be less than a 70 seconds");
        }
        else
            if (!silent) say("Время облучения не должно быть меньше 70 секунд");
        return false;
    }
    qint64 nowSec = QDateTime::currentDateTime().toSecsSinceEpoch();
    if (relayOneOutputs[10]){
        if (N2Sample.isOnChannel()){
            if ( (irradiationDurationInSec + nowSec) > (nowSec - N2Sample.getTimeElapsedInSec() + N2Sample.getIrradiationDurationInSec() - 120) && (irradiationDurationInSec + nowSec) < (nowSec - N2Sample.getTimeElapsedInSec() + N2Sample.getIrradiationDurationInSec() + 120) ){
                if (engLang){
                    if (!silent) say("Cannot set that irradiation time to N1 path sample. It's too close to irradiation endtime of N2 path sample, please wait a few minutes or change irradiation duration");
                }
                else
                    if (!silent) say("Невозможно установить текущее время облучения для образца на пути N1. Расчетное время возврата образца слишком близко к времени возврата образца на пути N2. Пожалуйста подождите пару минут или измените время облучения");
                return false;
            }
            // if smth gonna be returned shortly ->
            if ((nowSec - N2Sample.getTimeElapsedInSec() + N2Sample.getIrradiationDurationInSec()) < nowSec + 120){
                if (engLang){
                    if (!silent) say("Cannot send sample right now. N2 path sample going to be returned soon. Please wait a few minutes or return it now");
                }
                else
                    if (!silent) say("Невозможно отправить образец сейчас. В ближайшее время начнется процесс возврата образца с пути N2. Пожалуйста подождите пару минут");
                return false;
            }
        }
        if (GSample.isOnChannel()){
            if ( (irradiationDurationInSec + nowSec) > (nowSec - GSample.getTimeElapsedInSec() + GSample.getIrradiationDurationInSec() - 120) && (irradiationDurationInSec + nowSec) < (nowSec - GSample.getTimeElapsedInSec() + GSample.getIrradiationDurationInSec() + 120) ){
                if (engLang){
                    if (!silent) say("Cannot set that irradiation time to N1 path sample. It's too close to irradiation endtime of G path sample, please wait a few minutes or change irradiation duration");
                }
                else
                    if (!silent) say("Невозможно установить текущее время облучения для образца на пути N1. Расчетное время возврата образца слишком близко к времени возврата образца на пути G. Пожалуйста подождите пару минут или измените время облучения");
                return false;
            }
            if ((nowSec - GSample.getTimeElapsedInSec() + GSample.getIrradiationDurationInSec()) < nowSec + 120){
                if (engLang){
                    if (!silent) say("Cannot send sample right now. G path sample going to be returned soon. Please wait a few minutes or return it now");
                }
                else
                    if (!silent) say("Невозможно отправить образец сейчас. В ближайшее время начнется процесс возврата образца с пути G. Пожалуйста подождите пару минут");
                return false;
            }
        }
    }
    if (relayOneOutputs[11]){
        if (N1Sample.isOnChannel()){
            if ( (irradiationDurationInSec + nowSec) > (nowSec - N1Sample.getTimeElapsedInSec() + N1Sample.getIrradiationDurationInSec() - 120) && (irradiationDurationInSec + nowSec) < (nowSec - N1Sample.getTimeElapsedInSec() + N1Sample.getIrradiationDurationInSec() + 120) ){
                if (engLang){
                    if (!silent) say("Cannot set that irradiation time to N2 path sample. It's too close to irradiation endtime of N1 path sample, please wait a few minutes or change irradiation duration");
                }
                else
                    if (!silent) say("Невозможно установить текущее время облучения для образца на пути N2. Расчетное время возврата образца слишком близко к времени возврата образца на пути N1. Пожалуйста подождите пару минут или измените время облучения");
                return false;
            }
            if ((nowSec - N1Sample.getTimeElapsedInSec() + N1Sample.getIrradiationDurationInSec()) < nowSec + 120){
                if (engLang){
                    if (!silent) say("Cannot send sample right now. N1 path sample going to be returned soon. Please wait a few minutes or return it now");
                }
                else
                    if (!silent) say("Невозможно отправить образец сейчас. В ближайшее время начнется процесс возврата образца с пути N1. Пожалуйста подождите пару минут");
                return false;
            }
        }
        if (GSample.isOnChannel()){
            if ( (irradiationDurationInSec + nowSec) > (nowSec - GSample.getTimeElapsedInSec() + GSample.getIrradiationDurationInSec() - 120) && (irradiationDurationInSec + nowSec) < (nowSec - GSample.getTimeElapsedInSec() + GSample.getIrradiationDurationInSec() + 120) ){
                if (engLang){
                    if (!silent) say("Cannot set that irradiation time to N2 path sample. It's too close to irradiation endtime of G path sample, please wait a few minutes or change irradiation duration");
                }
                else
                    if (!silent) say("Невозможно установить текущее время облучения для образца на пути N2. Расчетное время возврата образца слишком близко к времени возврата образца на пути G. Пожалуйста подождите пару минут или измените время облучения");
                return false;
            }
            if ((nowSec - GSample.getTimeElapsedInSec() + GSample.getIrradiationDurationInSec()) < nowSec + 120){
                if (engLang){
                    if (!silent) say("Cannot send sample right now. G path sample going to be returned soon. Please wait a few minutes or return it now");
                }
                else
                    if (!silent) say("Невозможно отправить образец сейчас. В ближайшее время начнется процесс возврата образца с пути G. Пожалуйста подождите пару минут");
                return false;
            }
        }
    }
    if (relayOneOutputs[12]){
        if (N1Sample.isOnChannel()){
            if ( (irradiationDurationInSec + nowSec) > (nowSec - N1Sample.getTimeElapsedInSec() + N1Sample.getIrradiationDurationInSec() - 120) && (irradiationDurationInSec + nowSec) < (nowSec - N1Sample.getTimeElapsedInSec() + N1Sample.getIrradiationDurationInSec() + 120) ){
                if (engLang){
                    if (!silent) say("Cannot set that irradiation time to G path sample. It's too close to irradiation endtime of N1 path sample, please wait a few minutes or change irradiation duration");
                }
                else
                    if (!silent) say("Невозможно установить текущее время облучения для образца на пути G. Расчетное время возврата образца слишком близко к времени возврата образца на пути N1. Пожалуйста подождите пару минут или измените время облучения");
                return false;
            }
            if ((nowSec - N1Sample.getTimeElapsedInSec() + N1Sample.getIrradiationDurationInSec()) < nowSec + 120){
                if (engLang){
                    if (!silent) say("Cannot send sample right now. N1 path sample going to be returned soon. Please wait a few minutes or return it now");
                }
                else
                    if (!silent) say("Невозможно отправить образец сейчас. В ближайшее время начнется процесс возврата образца с пути N1. Пожалуйста подождите пару минут");
                return false;
            }
        }
        if (N2Sample.isOnChannel()){
            if ( (irradiationDurationInSec + nowSec) > (nowSec - N2Sample.getTimeElapsedInSec() + N2Sample.getIrradiationDurationInSec() - 120) && (irradiationDurationInSec + nowSec) < (nowSec - N2Sample.getTimeElapsedInSec() + N2Sample.getIrradiationDurationInSec() + 120) ){
                if (engLang){
                    if (!silent) say("Cannot set that irradiation time to G path sample. It's too close to irradiation endtime of N2 path sample, please wait a few minutes or change irradiation duration");
                }
                else
                    if (!silent) say("Невозможно установить текущее время облучения для образца на пути G. Расчетное время возврата образца слишком близко к времени возврата образца на пути N2. Пожалуйста подождите пару минут или измените время облучения");
                return false;
            }
            if ((nowSec - N2Sample.getTimeElapsedInSec() + N2Sample.getIrradiationDurationInSec()) < nowSec + 120){
                if (engLang){
                    if (!silent) say("Cannot send sample right now. N2 path sample going to be returned soon. Please wait a few minutes or return it now");
                }
                else
                    if (!silent) say("Невозможно отправить образец сейчас. В ближайшее время начнется процесс возврата образца с пути N2. Пожалуйста подождите пару минут");
                return false;
            }
        }
    }
    return true;
}

bool MainWindow::createDbConnection(){
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC3", "NAA_db");
    db.setDatabaseName(QString("DRIVER={SQL Server};"
                               "SERVER=" + DBserver + ";DATABASE=" + DBname + ";Persist Security Info=true;"
                               "uid=" + DBuser + ";pwd=" + DBpwd));
    return db.open();
}

void MainWindow::checkDoze(){
    if (doze > maximumDoze){
        QMessageBox msgBox;
        if (engLang)
            msgBox.setText("Gamma-activity is too high! It is recommended to use proboteka");
        else
            msgBox.setText("Высокий уровень гамма-активности. \nРекомендуется оставить образец в проботеке");
        msgBox.exec();
    }
    dozeTimerAlready = true;
    ui->dozPostButton->setEnabled(true);
    ui->dozPostButton->setFlat(true);
    dozeTimer->stop();
    //ui->dozPostButton->setPalette(onPal);
}
void MainWindow::say(QString text){
    ui->textBrowser->append(QDateTime::currentDateTime().time().toString() + " " + text);
    QScrollBar *sb = ui->textBrowser->verticalScrollBar();
    sb->setValue(sb->maximum());
}
void MainWindow::on_N1Button_pressed()
{
    writeRelayInput(0, 5, 1);
    QTimer::singleShot(3000, this, SLOT(updateGuiSampleInfo()));
}


void MainWindow::on_N1Button_released()
{
    writeRelayInput(0, 5, 0);
}


void MainWindow::on_N2Button_pressed()
{
    writeRelayInput(0, 6, 1);
    QTimer::singleShot(3000, this, SLOT(updateGuiSampleInfo()));
}


void MainWindow::on_N2Button_released()
{
    writeRelayInput(0, 6, 0);
}


void MainWindow::on_GButton_pressed()
{
    writeRelayInput(0, 7, 1);
    QTimer::singleShot(3000, this, SLOT(updateGuiSampleInfo()));
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
    ui->dozPostButton->setEnabled(false);
    ui->dozPostButton->setFlat(false);
    dozeTimer->stop();
}


void MainWindow::on_startButton_pressed()
{
    if (isIrradiationTimeAppropriate(false)){
        autoReturnTimer->stop();
        autoReturnTimer->disconnect();  // in case there was fault return and then guys reboot the automatics. in that case autoreturnTimer is fckd up
        connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(isIrradiationDoneCycle()));
        autoReturnTimer->start(1000);
        writeRelayInput(0, 9, 1);

    }
    else{
        if (engLang)
            say("Cannot send sample");
        else
            say("Невозможно отправить образец");
    }
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
    autoReturnTimer->disconnect();
    autoReturnTimer->stop();
    if (relayOneOutputs[10]){
        connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(checkAutoReturnN1()));
        autoReturnTimer->start(1000);
    }
    if (relayOneOutputs[11]){
        connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(checkAutoReturnN2()));
        autoReturnTimer->start(1000);
    }
    if (relayOneOutputs[12]){
        connect(autoReturnTimer, SIGNAL(timeout()), this, SLOT(checkAutoReturnG()));
        autoReturnTimer->start(1000);
    }
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

void MainWindow::on_emergencyReturnButton_pressed() // changed to emergencyReturnToggled
{
    //writeRelayInput(0, 10, 1);
    //on_returnButton_pressed();
    //emergencyReturnTimer->start(1000);
}

void MainWindow::on_emergencyReturnButton_released()  // changed to emergencyReturnToggled
{
    //writeRelayInput(0, 10, 0);
}

void MainWindow::emergencyReturnOff()
{
    //writeRelayInput(0, 10, 0); // "unbutton" emergency button
    //on_returnButton_released();
    //emergencyReturnTimer->stop();
}

void MainWindow::on_buttonBox_accepted()
{
    if (isIrradiationTimeAppropriate(false)){
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
    sampleJournal *journal = new sampleJournal(engLang);
    QObject::connect(journal, SIGNAL(sampleChoosen(QVector<QString>)), this, SLOT(readSampleInfo(QVector<QString>)));
    journal->showMaximized();
}

void MainWindow::readSampleInfo(QVector<QString> sampleInfo){
    if (relayOneOutputs[10]){
        if (sampleInfo != N2Sample.getName() && sampleInfo != GSample.getName()){
            N1Sample.setName(sampleInfo);
        }
        else{
            if (engLang)
                say("Cannot choose that sample. It's already choosen at other path");
            else
                say("Невозможно выбрать данный образец. Он уже выбран на другом пути");
        }
    }
    if (relayOneOutputs[11]){
        if (sampleInfo != N1Sample.getName() && sampleInfo != GSample.getName()){
            N2Sample.setName(sampleInfo);
        }
        else{
            if (engLang)
                say("Cannot choose that sample. It's already choosen at other path");
            else
                say("Невозможно выбрать данный образец. Он уже выбран на другом пути");
        }
    }
    if (relayOneOutputs[12]){
        if (sampleInfo != N1Sample.getName() && sampleInfo != N2Sample.getName()){
            GSample.setName(sampleInfo);
        }
        else{
            if (engLang)
                say("Cannot choose that sample. It's already choosen at other path");
            else
                say("Невозможно выбрать данный образец. Он уже выбран на другом пути");
        }
    }
    updateGuiSampleInfo();
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

void MainWindow::on_languageButton_toggled(bool checked)
{
    if (checked){
        engLang = true;
        connectionSettings->setValue("language", "en");
        ui->dozPostButton->setText("Doz.\npost");
        ui->languageButton->setText("Русский");
        ui->pressureOkLabel->setText("Pressure ok");
        ui->startButton->setText("Send");
        ui->returnButton->setText("Return");
        ui->aboutLabel->setText("Pneumatic conveying system Regata-2");
        ui->uzvLabel->setText("Container");
        ui->containerLabel->setText("Door closed");
        ui->uzvClosedLabel->setText("Proboteka");
        ui->uzvPressureLabel->setText("Pressure ok");
        ui->proboDropButton->setText("Drop");
        ui->probotekaButton->setText("Proboteka\n ON");
        ui->sampleLabel->setText("Sample:");
        ui->irradiationTimeLabel->setText("Irradiation time set:");
        ui->irradiationFactlabel->setText("Irradiation time:");
        ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Cancel");
        ui->emergencyReturnButton->setText("Emergency\nreturn");
        ui->emergencyDozPostButton->setText("Emergency\nstopper open");
        ui->sampleChooseButton->setText("Choose");
        ui->sampleResetButton->setText("Reset");
        ui->setDaysSpinBox->setSuffix("D");
        ui->setHoursSpinBox->setSuffix("H");
        ui->setSecondsSpinBox->setSuffix("S");
        ui->getDaysSpinBox->setSuffix("D");
        ui->getHoursSpinBox->setSuffix("H");
        ui->getSecondsSpinBox->setSuffix("S");
    }
    else{
        engLang = false;
        connectionSettings->setValue("language", "ru");
        ui->dozPostButton->setText("Доз.\nпост");
        ui->languageButton->setText("English");
        ui->pressureOkLabel->setText("Давление норма");
        ui->startButton->setText("Отправка");
        ui->returnButton->setText("Возврат");
        ui->aboutLabel->setText("Пневмотранспортная установка Регата-2");
        ui->uzvLabel->setText("Контейнер");
        ui->containerLabel->setText("УЗВ закр.");
        ui->uzvClosedLabel->setText("Проботека");
        ui->uzvPressureLabel->setText("Давление в УЗВ");
        ui->proboDropButton->setText("Сброс");
        ui->probotekaButton->setText("ВКЛ\n Проботеку");
        ui->sampleLabel->setText("Образец:");
        ui->irradiationTimeLabel->setText("Время облучения:");
        ui->irradiationFactlabel->setText("Фактическое:");
        ui->buttonBox->button(QDialogButtonBox::Cancel)->setText("Отмена");
        ui->emergencyReturnButton->setText("Аварийный \nвозврат");
        ui->emergencyDozPostButton->setText("Аварийное \nоткрытие стоппера");
        ui->sampleChooseButton->setText("Выбрать");
        ui->sampleResetButton->setText("Сброс");
        ui->setDaysSpinBox->setSuffix("Д");
        ui->setHoursSpinBox->setSuffix("Ч");
        ui->setSecondsSpinBox->setSuffix("С");
        ui->getDaysSpinBox->setSuffix("Д");
        ui->getHoursSpinBox->setSuffix("Ч");
        ui->getSecondsSpinBox->setSuffix("С");
    }
}

void MainWindow::on_emergencyReturnButton_toggled(bool checked)
{
    if (checked){
        writeRelayInput(0, 10, 1);
    }
    else
        writeRelayInput(0, 10, 0);
}

void MainWindow::on_experimenterComboBox_currentTextChanged(const QString &arg1)
{
    if (relayOneOutputs[10]){
        N1Sample.setExperimenterName(arg1);
    }
    if (relayOneOutputs[11]){
        N2Sample.setExperimenterName(arg1);
    }
    if (relayOneOutputs[12]){
        GSample.setExperimenterName(arg1);
    }
}
/*
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
}*/
/*
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
void MainWindow::on_testPathComboBox_currentTextChanged(const QString &arg1)
{
    relayOneOutputs[10] = (arg1 == "N1") ? 1 : 0;
    relayOneOutputs[11] = (arg1 == "N2") ? 1 : 0;
    relayOneOutputs[12] = (arg1 == "G") ? 1 : 0;
    updateGuiSampleInfo();
}
*/

