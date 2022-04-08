#include "samplejournal.h"
#include "ui_samplejournal.h"
#include <QSqlDatabase>

sampleJournal::sampleJournal(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::sampleJournal)
{
    ui->setupUi(this);
    QSqlDatabase db = QSqlDatabase::database("NAA_db");
    model = new QSqlTableModel(nullptr, db);
    model->setTable("Sample");
    model->setEditStrategy(QSqlTableModel::OnManualSubmit);
    model->select();
    model->setHeaderData(0, Qt::Horizontal, tr("Код страны"));
    model->setHeaderData(1, Qt::Horizontal, tr("Код клиента"));
    model->setHeaderData(2, Qt::Horizontal, tr("Год"));
    sampleInfo.resize(7);   // 0 - which table (sli/lli), 1 - country code, 2 - Client_ID, 3 - Year, 4 - Sample_Set_ID, 5 - Sample_ID, 6 - Date_Start,
    on_sliButton_clicked();
    ui->comboBox->clear();
    QSqlQuery query("SELECT DISTINCT P_Date_Sample_Preparation FROM Sample ORDER BY P_Date_Sample_Preparation", db);
    while (query.next()){
        ui->comboBox->addItem(query.value(0).toString());
    }
    query.last();
    ui->comboBox->setCurrentText(query.value(0).toString());
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
    sampleInfo[0] = "КЖИ";
}

void sampleJournal::on_lliButton_clicked()
{
    sampleInfo[0] = "ДЖИ";
}

void sampleJournal::updateTable(QString dateStart){
    model->setFilter(QString("P_Date_Sample_Preparation like '%%1%'").arg(dateStart));
    ui->tableView->setModel(model);
}

void sampleJournal::on_comboBox_currentTextChanged(const QString &arg1)
{
   updateTable(arg1);
}

void sampleJournal::on_tableView_doubleClicked(const QModelIndex &index)
{
    sampleInfo[1] = model->record(index.row()).value("Country_Code").toString();
    sampleInfo[2] = model->record(index.row()).value("Client_ID").toString();
    sampleInfo[3] = model->record(index.row()).value("Year").toString();
    sampleInfo[4] = model->record(index.row()).value("Sample_Set_ID").toString();
    sampleInfo[5] = model->record(index.row()).value("Sample_ID").toString();
    sampleInfo[6] = model->record(index.row()).value("Date_Start").toString();
    emit sampleChoosen(sampleInfo);
    this->close();
}

void sampleJournal::on_chooseButton_clicked()
{
    on_tableView_doubleClicked(ui->tableView->currentIndex());
}

