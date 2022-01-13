#include "samplejournal.h"
#include "ui_samplejournal.h"
#include <QSqlDatabase>

sampleJournal::sampleJournal(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::sampleJournal)
{
    ui->setupUi(this);
    QSqlDatabase db = QSqlDatabase::database("NAA_db");
    sliModel = new QSqlTableModel(nullptr, db);
    lliModel = new QSqlTableModel(nullptr, db);
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
    sampleInfo.resize(7);   // 0 - which table (sli/lli), 1 - country code, 2 - Client_ID, 3 - Year, 4 - Sample_Set_ID, 5 - Sample_ID, 6 - Date_Start,
    on_sliButton_clicked();
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
    QSqlDatabase db = QSqlDatabase::database("NAA_db");
    ui->sliButton->setChecked(true);
    ui->lliButton->setChecked(false);
    ui->comboBox->clear();
    QSqlQuery query("SELECT DISTINCT Date_Start FROM table_SLI_Irradiation_Log ORDER BY Date_Start", db);
    while (query.next()){
        ui->comboBox->addItem(query.value(0).toString());
    }
    query.last();
    ui->comboBox->setCurrentText(query.value(0).toString());
    sampleInfo[0] = "КЖИ";
}

void sampleJournal::on_lliButton_clicked()
{
    QSqlDatabase db = QSqlDatabase::database("NAA_db");
    ui->sliButton->setChecked(false);
    ui->lliButton->setChecked(true);
    ui->comboBox->clear();
    QSqlQuery query("SELECT DISTINCT Date_Start FROM table_LLI_Irradiation_Log ORDER BY Date_Start", db);
    while (query.next()){
        ui->comboBox->addItem(query.value(0).toString());
    }
    query.last();
    ui->comboBox->setCurrentText(query.value(0).toString());
    sampleInfo[0] = "ДЖИ";
}

void sampleJournal::updateTable(QString dateStart){
    if (ui->lliButton->isChecked()){
        lliModel->setFilter(QString("Date_Start like '%%1%'").arg(dateStart));
        ui->tableView->setModel(lliModel);
    }
    else{
        sliModel->setFilter(QString("Date_Start like '%%1%'").arg(dateStart));
        ui->tableView->setModel(sliModel);
    }
}

void sampleJournal::on_comboBox_currentTextChanged(const QString &arg1)
{
   updateTable(arg1);
}

void sampleJournal::on_tableView_doubleClicked(const QModelIndex &index)
{
    if (ui->lliButton->isChecked()){
        sampleInfo[1] = lliModel->record(index.row()).value("Country_Code").toString();
        sampleInfo[2] = lliModel->record(index.row()).value("Client_ID").toString();
        sampleInfo[3] = lliModel->record(index.row()).value("Year").toString();
        sampleInfo[4] = lliModel->record(index.row()).value("Sample_Set_ID").toString();
        sampleInfo[5] = lliModel->record(index.row()).value("Sample_ID").toString();
        sampleInfo[6] = lliModel->record(index.row()).value("Date_Start").toString();
    }
    else{
        sampleInfo[1] = sliModel->record(index.row()).value("Country_Code").toString();
        sampleInfo[2] = sliModel->record(index.row()).value("Client_ID").toString();
        sampleInfo[3] = sliModel->record(index.row()).value("Year").toString();
        sampleInfo[4] = sliModel->record(index.row()).value("Sample_Set_ID").toString();
        sampleInfo[5] = sliModel->record(index.row()).value("Sample_ID").toString();
        sampleInfo[6] = sliModel->record(index.row()).value("Date_Start").toString();
    }
    emit sampleChoosen(sampleInfo);
    this->close();
}
