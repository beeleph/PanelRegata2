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
    currentSpinny->setValue(smth.toInt());
}

void VirtualKeyboard::connect(QSpinBox* spinny){
    currentSpinny = spinny;
    wasBefore = currentSpinny->value();
    this->move(currentSpinny->x()+currentSpinny->width(), currentSpinny->y()+currentSpinny->height());
}

void VirtualKeyboard::on_cancelButton_clicked()
{
    currentSpinny->setValue(wasBefore);
    this->close();
}


void VirtualKeyboard::on_okButton_clicked()
{
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

