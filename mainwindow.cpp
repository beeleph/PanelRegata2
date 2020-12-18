#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QSettings::setPath(QSettings::IniFormat, QSettings::SystemScope, ".");
    connectionSettings = new QSettings("LamelsRadius.ini", QSettings::IniFormat);
    relayOne = new QModbusRtuSerialMaster(this);
    relayTwo = new QModbusRtuSerialMaster(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::readIniToModbusDevice(QModbusClient *relay, int id){

    //this->radius[i] = settings->value("r" + QString::number(i), 0).toDouble();

}
