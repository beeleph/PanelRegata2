#include "samplejournal.h"
#include "ui_samplejournal.h"
#include <QSqlDatabase>

sampleJournal::sampleJournal(bool englang, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::sampleJournal)
{
    ui->setupUi(this);
    if (englang){
        ui->sliButton->setText("SLI");
        ui->lliButton->setText("LLI");
        ui->label->setText("Choose sample set");
        ui->chooseButton->setText("Ok");
        ui->cancelButton->setText("Cancel");
    }else{
        ui->sliButton->setText("КЖИ");
        ui->lliButton->setText("ДЖИ");
        ui->label->setText("Выберите партию");
        ui->chooseButton->setText("Выбор");
        ui->cancelButton->setText("Отмена");
    }
    QSqlDatabase db = QSqlDatabase::database("NAA_db");
    sampleModel = new QSqlTableModel(nullptr, db);
    sampleModel->setTable("table_Sample");
    sampleModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    sampleModel->select();
    sampleModel->setHeaderData(0, Qt::Horizontal, tr("Код страны"));
    sampleModel->setHeaderData(1, Qt::Horizontal, tr("Код клиента"));
    sampleModel->setHeaderData(2, Qt::Horizontal, tr("Год"));
    sampleInfo.resize(7);   // 0 - which table (sli/lli), 1 - country code, 2 - Client_ID, 3 - Year, 4 - Sample_Set_ID, 5 - Sample_Set_Index, 6 - Sample_ID,
    setModel = new QSqlTableModel(nullptr, db);
    setModel->setTable("table_Sample_Set");
    setModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    setModel->select();
    setModel->setHeaderData(0, Qt::Horizontal, tr("Код страны"));
    setModel->setHeaderData(1, Qt::Horizontal, tr("Код клиента"));
    setModel->setHeaderData(2, Qt::Horizontal, tr("Год"));
    setInfo.resize(5);          // 0 - Country_Code, 1 - Client_ID, 2 - Year, 3 - Sample_Set_ID, 4 - Sample_Set_Index
    on_sliButton_clicked();
    ui->tableView->setModel(setModel);
    //QSqlQuery query("SELECT DISTINCT P_Date_Sample_Preparation FROM table_Sample ORDER BY P_Date_Sample_Preparation", db);
    //select to table set
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
    ui->sliButton->setChecked(true);
    ui->lliButton->setChecked(false);
}

void sampleJournal::on_lliButton_clicked()
{
    sampleInfo[0] = "ДЖИ";
    ui->sliButton->setChecked(false);
    ui->lliButton->setChecked(true);
}

void sampleJournal::updateTable(QString dateStart){
    sampleModel->setFilter(QString("P_Date_Sample_Preparation like '%%1%'").arg(dateStart));
    ui->tableView->setModel(sampleModel);
}

void sampleJournal::on_comboBox_currentTextChanged(const QString &arg1)
{
   updateTable(arg1);
}

void sampleJournal::on_tableView_doubleClicked(const QModelIndex &index)
{
    if (setChoosen){
        sampleInfo[1] = sampleModel->record(index.row()).value("F_Country_Code").toString();
        sampleInfo[2] = sampleModel->record(index.row()).value("F_Client_ID").toString();
        sampleInfo[3] = sampleModel->record(index.row()).value("F_Year").toString();
        sampleInfo[4] = sampleModel->record(index.row()).value("F_Sample_Set_ID").toString();
        sampleInfo[5] = sampleModel->record(index.row()).value("F_Sample_Set_Index").toString();
        sampleInfo[6] = sampleModel->record(index.row()).value("A_Sample_ID").toString();
        emit sampleChoosen(sampleInfo);
        this->close();
    }
    else{
        setInfo[0] = setModel->record(index.row()).value("Country_Code").toString();
        setInfo[1] = setModel->record(index.row()).value("Client_ID").toString();
        setInfo[2] = setModel->record(index.row()).value("Year").toString();
        setInfo[3] = setModel->record(index.row()).value("Sample_Set_ID").toString();
        setInfo[4] = setModel->record(index.row()).value("Sample_Set_Index").toString();
        sampleModel->setFilter(QString("F_Country_Code like '%%1%' AND F_Client_ID like '%%2%' AND F_Year like '%%3%' AND F_Sample_Set_ID like '%%4%' AND F_Sample_Set_Index like '%%5%'").arg(setInfo[0], setInfo[1], setInfo[2], setInfo[3], setInfo[4]));
        ui->tableView->setModel(sampleModel);
        ui->label->setText("Выбрана партия: " + setInfo[0] + "-" + setInfo[1] + "-" + setInfo[2] + "-" + setInfo[3] + "-" + setInfo[4]);
        setChoosen = true;
    }
}

void sampleJournal::on_chooseButton_clicked()
{
    on_tableView_doubleClicked(ui->tableView->currentIndex());
}

