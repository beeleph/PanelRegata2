#include "virtualkeyboard.h"
#include "ui_virtualkeyboard.h"

VirtualKeyboard::VirtualKeyboard(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::VirtualKeyboard)
{
    ui->setupUi(this);
}

VirtualKeyboard::~VirtualKeyboard()
{
    delete ui;
}

void VirtualKeyboard::addSmth(QString smth){
    currentInput += smth;
    currentLE->setText(currentInput);
}

void VirtualKeyboard::connect(QLineEdit* laino){
    currentLE = laino;
    wasBefore = currentLE->text();
    this->move(currentLE->x()+currentLE->width(), currentLE->y()+currentLE->height());
}

void VirtualKeyboard::on_cancelButton_clicked()
{
    currentLE->setText(wasBefore);
    emit cancel();
    this->close();
}


void VirtualKeyboard::on_okButton_clicked()
{
    emit ok();
    this->close();
}


void VirtualKeyboard::on_oneButton_clicked()
{
    addSmth("1");
}


void VirtualKeyboard::on_twoButton_clicked()
{
    addSmth("2");
}


void VirtualKeyboard::on_threeButton_clicked()
{
    addSmth("3");
}


void VirtualKeyboard::on_fourButton_clicked()
{
    addSmth("4");
}


void VirtualKeyboard::on_fiveButton_clicked()
{
    addSmth("5");
}


void VirtualKeyboard::on_sixButton_clicked()
{
    addSmth("6");
}


void VirtualKeyboard::on_sevenButton_clicked()
{
    addSmth("7");
}


void VirtualKeyboard::on_eightButton_clicked()
{
    addSmth("8");
}


void VirtualKeyboard::on_nineButton_clicked()
{
    addSmth("9");
}


void VirtualKeyboard::on_zeroButton_clicked()
{
    addSmth("0");
}
