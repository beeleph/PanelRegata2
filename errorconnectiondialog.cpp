#include "errorconnectiondialog.h"
#include "ui_errorconnectiondialog.h"

errorConnectionDialog::errorConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::errorConnectionDialog)
{
    ui->setupUi(this);
}

errorConnectionDialog::~errorConnectionDialog()
{
    delete ui;
}

void errorConnectionDialog::on_pushButton_clicked()
{
    exit(0);
}
