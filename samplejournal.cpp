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
    if (englang){
        sampleModel->setHeaderData(0, Qt::Horizontal, tr("Код страны"));
        sampleModel->setHeaderData(1, Qt::Horizontal, tr("Клиентский номер"));
        sampleModel->setHeaderData(2, Qt::Horizontal, tr("Год"));
        sampleModel->setHeaderData(3, Qt::Horizontal, tr("Номер партии"));
        sampleModel->setHeaderData(4, Qt::Horizontal, tr("Индекс партии"));
        sampleModel->setHeaderData(5, Qt::Horizontal, tr("Номер образца"));
        sampleModel->setHeaderData(6, Qt::Horizontal, tr("Номер файла"));
    }else{
        sampleModel->setHeaderData(0, Qt::Horizontal, tr("Country code"));
        sampleModel->setHeaderData(1, Qt::Horizontal, tr("Client ID"));
        sampleModel->setHeaderData(2, Qt::Horizontal, tr("Year"));
        sampleModel->setHeaderData(3, Qt::Horizontal, tr("Sample set ID"));
        sampleModel->setHeaderData(4, Qt::Horizontal, tr("Sample set Index"));
        sampleModel->setHeaderData(5, Qt::Horizontal, tr("Sample ID"));
        sampleModel->setHeaderData(6, Qt::Horizontal, tr("File number"));
    }
    sampleInfo.resize(7);   // 0 - which table (sli/lli), 1 - country code, 2 - Client_ID, 3 - Year, 4 - Sample_Set_ID, 5 - Sample_Set_Index, 6 - Sample_ID,
    setModel = new QSqlTableModel(nullptr, db);
    setModel->setTable("table_Sample_Set");
    setModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    setModel->select();
    setModel->setFilter(QString("1=1 ORDER BY Year ASC, Sample_Set_ID ASC"));
    if (!englang){
        setModel->setHeaderData(0, Qt::Horizontal, tr("Код страны"));
        setModel->setHeaderData(1, Qt::Horizontal, tr("Код клиента"));
        setModel->setHeaderData(2, Qt::Horizontal, tr("Год"));
        setModel->setHeaderData(3, Qt::Horizontal, tr("Номер партии"));
        setModel->setHeaderData(4, Qt::Horizontal, tr("Индекс партии"));
        setModel->setHeaderData(5, Qt::Horizontal, tr("Дата получения партии"));
        setModel->setHeaderData(6, Qt::Horizontal, tr("Дата отчета о партии"));
        setModel->setHeaderData(7, Qt::Horizontal, tr("Кем получена"));
        setModel->setHeaderData(8, Qt::Horizontal, tr("Заметки 1"));
        setModel->setHeaderData(9, Qt::Horizontal, tr("Заметки 2"));
        setModel->setHeaderData(10, Qt::Horizontal, tr("Заметки 3"));
    }
    setInfo.resize(5);          // 0 - Country_Code, 1 - Client_ID, 2 - Year, 3 - Sample_Set_ID, 4 - Sample_Set_Index
    on_sliButton_clicked();
    ui->tableView->setModel(setModel);
    ui->tableView->selectRow(setModel->rowCount()-1);
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
    if (setChoosen)
        updateTableFileNum();
}

void sampleJournal::on_lliButton_clicked()
{
    sampleInfo[0] = "ДЖИ";
    ui->sliButton->setChecked(false);
    ui->lliButton->setChecked(true);
    if (setChoosen)
        updateTableFileNum();
}

void sampleJournal::updateTableFileNum(){
    sampleModel->select();
    QString tmp;
    for (int i = 0; i < sampleModel->rowCount(); ++i){
        tmp = sampleModel->data(sampleModel->index(i, 6)).toString();
        if (ui->sliButton->isChecked()){
            tmp.chop(19);
            QVariant qvTmp(tmp);
            sampleModel->setData(sampleModel->index(i,6),qvTmp);
        }
        else{
            tmp.remove(18);
            QVariant qvTmp(tmp);
            sampleModel->setData(sampleModel->index(i,6),qvTmp);
        }
    }
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
        for (int i =6; i < 321; ++i){
            ui->tableView->hideColumn(i);
        }
        ui->tableView->hideColumn(322);
        ui->label->setText("Выбрана партия: " + setInfo[0] + "-" + setInfo[1] + "-" + setInfo[2] + "-" + setInfo[3] + "-" + setInfo[4]);
        setChoosen = true;
        updateTableFileNum();
    }
}

void sampleJournal::on_chooseButton_clicked()
{
    on_tableView_doubleClicked(ui->tableView->currentIndex());
}

