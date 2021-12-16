#include "samplejournal.h"
#include "ui_samplejournal.h"
#include <QSqlDatabase>

sampleJournal::sampleJournal(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::sampleJournal)
{
    ui->setupUi(this);
    sliModel = new QSqlTableModel;
    lliModel = new QSqlTableModel;
    sliModel->setTable("table_SLI_Irradiation_Log");
    sliModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    sliModel->select();
    sliModel->setHeaderData(0, Qt::Horizontal, tr("Код страны"));
    sliModel->setHeaderData(1, Qt::Horizontal, tr("Код клиента"));
    sliModel->setHeaderData(2, Qt::Horizontal, tr("Год"));
    lliModel->setTable("table_LLI_Irradiation_Log");
    lliModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    lliModel->select();
    lliModel->setHeaderData(0, Qt::Horizontal, tr("Код страны"));
    lliModel->setHeaderData(1, Qt::Horizontal, tr("Код клиента"));
    lliModel->setHeaderData(2, Qt::Horizontal, tr("Год"));
    on_sliButton_clicked();
    //QSqlDatabase db = QSqlDatabase::database();
}

sampleJournal::~sampleJournal()
{
    delete ui;
}

void sampleJournal::on_cancelButton_clicked()
{
    this->close();
}

void sampleJournal::on_sliButton_clicked()
{
    ui->sliButton->setChecked(true);
    ui->lliButton->setChecked(false);
    ui->comboBox->clear();
    QSqlQuery query("SELECT DISTINCT Date_Start FROM table_SLI_Irradiation_Log ORDER BY Date_Start");
    while (query.next()){
        ui->comboBox->addItem(query.value(0).toString());
    }
    updateTable(query.value(0).toString());
}

void sampleJournal::on_lliButton_clicked()
{
    ui->sliButton->setChecked(false);
    ui->lliButton->setChecked(true);
    ui->comboBox->clear();
    QSqlQuery query("SELECT DISTINCT Date_Start FROM table_LLI_Irradiation_Log ORDER BY Date_Start");
    while (query.next()){
        ui->comboBox->addItem(query.value(0).toString());
    }
    updateTable(query.value(0).toString());
}

void sampleJournal::updateTable(QString dateStart){
    if (ui->lliButton->isChecked()){
        lliModel->setFilter("Date_Start="+dateStart);
        ui->tableView->setModel(lliModel);
    }
    else{
        sliModel->setFilter("Date_Start="+dateStart);
        ui->tableView->setModel(sliModel);
    }
}

void sampleJournal::on_comboBox_textActivated(const QString &arg1)
{
    updateTable(arg1);
}
