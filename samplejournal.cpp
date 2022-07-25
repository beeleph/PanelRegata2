#include "samplejournal.h"
#include "ui_samplejournal.h"
#include <QSqlDatabase>

sampleJournal::sampleJournal(bool language, QWidget *parent) :
    QWidget(parent),
    ui(new Ui::sampleJournal)
{
    ui->setupUi(this);
    englang = language;
    if (englang){
        ui->sliButton->setText("SLI");
        ui->lliButton->setText("LLI");
        ui->label->setText("Choose sample set");
        ui->chooseButton->setText("Ok");
        ui->cancelButton->setText("Cancel");
        ui->sampleTypeComboBox->setItemText(0, "Samples");
        ui->sampleTypeComboBox->setItemText(1, "Standarts");
        ui->sampleTypeComboBox->setItemText(2, "Monitors");
    }else{
        ui->sliButton->setText("КЖИ");
        ui->lliButton->setText("ДЖИ");
        ui->label->setText("Выберите партию");
        ui->chooseButton->setText("Выбор");
        ui->cancelButton->setText("Отмена");
        ui->sampleTypeComboBox->setItemText(0, "Образцы");
        ui->sampleTypeComboBox->setItemText(1, "Стандарты");
        ui->sampleTypeComboBox->setItemText(2, "Мониторы");
    }
    QSqlDatabase db = QSqlDatabase::database("NAA_db");
    sampleModel = new QSqlTableModel(nullptr, db);
    sampleModel->setTable("table_Sample");
    sampleModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    sampleModel->select();
    if (!englang){
        sampleModel->setHeaderData(0, Qt::Horizontal, tr("Код страны"));
        sampleModel->setHeaderData(1, Qt::Horizontal, tr("Клиентский\n номер"));
        sampleModel->setHeaderData(2, Qt::Horizontal, tr("Год"));
        sampleModel->setHeaderData(3, Qt::Horizontal, tr("Номер партии"));
        sampleModel->setHeaderData(4, Qt::Horizontal, tr("Индекс партии"));
        sampleModel->setHeaderData(5, Qt::Horizontal, tr("Номер образца"));
        sampleModel->setHeaderData(321, Qt::Horizontal, tr("Номер файла"));
    }else{
        sampleModel->setHeaderData(0, Qt::Horizontal, tr("Country code"));
        sampleModel->setHeaderData(1, Qt::Horizontal, tr("Client ID"));
        sampleModel->setHeaderData(2, Qt::Horizontal, tr("Year"));
        sampleModel->setHeaderData(3, Qt::Horizontal, tr("Sample set ID"));
        sampleModel->setHeaderData(4, Qt::Horizontal, tr("Sample set Index"));
        sampleModel->setHeaderData(5, Qt::Horizontal, tr("Sample ID"));
        sampleModel->setHeaderData(321, Qt::Horizontal, tr("File number"));
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
        setModel->setHeaderData(5, Qt::Horizontal, tr("Дата получения\n партии"));
        setModel->setHeaderData(6, Qt::Horizontal, tr("Дата отчета \nо партии"));
        setModel->setHeaderData(7, Qt::Horizontal, tr("Кем получена"));
        setModel->setHeaderData(8, Qt::Horizontal, tr("Заметки 1"));
        setModel->setHeaderData(9, Qt::Horizontal, tr("Заметки 2"));
        setModel->setHeaderData(10, Qt::Horizontal, tr("Заметки 3"));
    }else{
        setModel->setHeaderData(0, Qt::Horizontal, tr("Country code"));
        setModel->setHeaderData(1, Qt::Horizontal, tr("Client ID"));
        setModel->setHeaderData(2, Qt::Horizontal, tr("Year"));
        setModel->setHeaderData(3, Qt::Horizontal, tr("Sample set ID"));
        setModel->setHeaderData(4, Qt::Horizontal, tr("Sample set\n index"));
        setModel->setHeaderData(5, Qt::Horizontal, tr("Sample set\n receipt date"));
        setModel->setHeaderData(6, Qt::Horizontal, tr("Sample set\n report date"));
        setModel->setHeaderData(7, Qt::Horizontal, tr("Recieved by"));
        setModel->setHeaderData(8, Qt::Horizontal, tr("Notes 1"));
        setModel->setHeaderData(9, Qt::Horizontal, tr("Notes 2"));
        setModel->setHeaderData(10, Qt::Horizontal, tr("Notes 3"));
    }
    setInfo.resize(5);          // 0 - Country_Code, 1 - Client_ID, 2 - Year, 3 - Sample_Set_ID, 4 - Sample_Set_Index
    standartModel = new QSqlTableModel(nullptr, db);
    standartModel->setTable("table_SRM");
    standartModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    standartModel->select();
    if (!englang){
        standartModel->setHeaderData(0, Qt::Horizontal, tr("Индекс партии"));    // start from here.
        standartModel->setHeaderData(1, Qt::Horizontal, tr("Номер партии"));
        standartModel->setHeaderData(2, Qt::Horizontal, tr("Вес партии"));
        standartModel->setHeaderData(3, Qt::Horizontal, tr("Номер \nстандарта"));
        standartModel->setHeaderData(4, Qt::Horizontal, tr("Вес КЖИ"));
        standartModel->setHeaderData(5, Qt::Horizontal, tr("Дата \nоблучения КЖИ"));
        standartModel->setHeaderData(16, Qt::Horizontal, tr("Вес ДЖИ"));
        standartModel->setHeaderData(17, Qt::Horizontal, tr("Дата \nоблучения ДЖИ"));
    }else{
        standartModel->setHeaderData(0, Qt::Horizontal, tr("Set name"));    // start from here.
        standartModel->setHeaderData(1, Qt::Horizontal, tr("Set number"));
        standartModel->setHeaderData(2, Qt::Horizontal, tr("Set weight"));
        standartModel->setHeaderData(3, Qt::Horizontal, tr("SRM number"));
        standartModel->setHeaderData(4, Qt::Horizontal, tr("SLI weight"));
        standartModel->setHeaderData(5, Qt::Horizontal, tr("SLI\n irradiation date"));
        standartModel->setHeaderData(16, Qt::Horizontal, tr("LLI weight"));
        standartModel->setHeaderData(17, Qt::Horizontal, tr("LLI\n irradiation date"));
    }
    standartSetModel = new QSqlTableModel(nullptr, db);
    standartSetModel->setTable("table_SRM_Set");
    standartSetModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    standartSetModel->select();
    if (!englang){
        standartSetModel->setHeaderData(0, Qt::Horizontal, tr("Индекс партии"));    // start from here.
        standartSetModel->setHeaderData(1, Qt::Horizontal, tr("Номер партии"));
        standartSetModel->setHeaderData(2, Qt::Horizontal, tr("Тип стандарта"));
        standartSetModel->setHeaderData(3, Qt::Horizontal, tr("Вес партии"));
        standartSetModel->setHeaderData(4, Qt::Horizontal, tr("Дата \nприобретения"));
    }else{
        standartSetModel->setHeaderData(0, Qt::Horizontal, tr("Set name"));    // start from here.
        standartSetModel->setHeaderData(1, Qt::Horizontal, tr("Set number"));
        standartSetModel->setHeaderData(2, Qt::Horizontal, tr("Set type"));
        standartSetModel->setHeaderData(3, Qt::Horizontal, tr("Set weight"));
        standartSetModel->setHeaderData(4, Qt::Horizontal, tr("Purchasing date"));
    }
    monitorModel = new QSqlTableModel(nullptr, db);
    monitorModel->setTable("table_Monitor");
    monitorModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    monitorModel->select();
    if (!englang){
        monitorModel->setHeaderData(0, Qt::Horizontal, tr("Индекс партии"));    // start from here.
        monitorModel->setHeaderData(1, Qt::Horizontal, tr("Номер партии"));
        monitorModel->setHeaderData(2, Qt::Horizontal, tr("Вес партии"));
        monitorModel->setHeaderData(3, Qt::Horizontal, tr("Номер \nмонитора"));
        monitorModel->setHeaderData(4, Qt::Horizontal, tr("Вес КЖИ"));
        monitorModel->setHeaderData(5, Qt::Horizontal, tr("Дата \nоблучения КЖИ"));
        monitorModel->setHeaderData(7, Qt::Horizontal, tr("Вес ДЖИ"));
        monitorModel->setHeaderData(8, Qt::Horizontal, tr("Дата \nоблучения ДЖИ"));
    }else{
        monitorModel->setHeaderData(0, Qt::Horizontal, tr("Set name"));    // start from here.
        monitorModel->setHeaderData(1, Qt::Horizontal, tr("Set number"));
        monitorModel->setHeaderData(2, Qt::Horizontal, tr("Set weight"));
        monitorModel->setHeaderData(3, Qt::Horizontal, tr("Monitor number"));
        monitorModel->setHeaderData(4, Qt::Horizontal, tr("SLI weight"));
        monitorModel->setHeaderData(5, Qt::Horizontal, tr("SLI\n irradiation date"));
        monitorModel->setHeaderData(7, Qt::Horizontal, tr("LLI weight"));
        monitorModel->setHeaderData(8, Qt::Horizontal, tr("LLI\n irradiation date"));
    }
    monitorSetModel = new QSqlTableModel(nullptr, db);
    monitorSetModel->setTable("table_Monitor_Set");
    monitorSetModel->setEditStrategy(QSqlTableModel::OnManualSubmit);
    monitorSetModel->select();
    if (!englang){
        monitorSetModel->setHeaderData(0, Qt::Horizontal, tr("Индекс партии"));    // start from here.
        monitorSetModel->setHeaderData(1, Qt::Horizontal, tr("Номер партии"));
        monitorSetModel->setHeaderData(2, Qt::Horizontal, tr("Тип монитора"));
        monitorSetModel->setHeaderData(3, Qt::Horizontal, tr("Вес партии"));
        monitorSetModel->setHeaderData(4, Qt::Horizontal, tr("Дата \nприобретения"));
    }else{
        monitorSetModel->setHeaderData(0, Qt::Horizontal, tr("Set name"));    // start from here.
        monitorSetModel->setHeaderData(1, Qt::Horizontal, tr("Set number"));
        monitorSetModel->setHeaderData(2, Qt::Horizontal, tr("Monitor type"));
        monitorSetModel->setHeaderData(3, Qt::Horizontal, tr("Set weight"));
        monitorSetModel->setHeaderData(4, Qt::Horizontal, tr("Purchasing date"));
    }
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
    if (setChoosen && ui->sampleTypeComboBox->currentIndex() == 0)
        updateTableFileNum();
}

void sampleJournal::on_lliButton_clicked()
{
    sampleInfo[0] = "ДЖИ";
    ui->sliButton->setChecked(false);
    ui->lliButton->setChecked(true);
    if (setChoosen && ui->sampleTypeComboBox->currentIndex() == 0)
        updateTableFileNum();
}

void sampleJournal::updateTableFileNum(){
    sampleModel->select();
    QString tmp;
    for (int i = 0; i < sampleModel->rowCount(); ++i){
        tmp = sampleModel->data(sampleModel->index(i, 321)).toString();
        if (ui->sliButton->isChecked()){
            tmp.chop(19);
            QVariant qvTmp(tmp);
            sampleModel->setData(sampleModel->index(i,321),qvTmp);
        }
        else{
            tmp.remove(0,18);
            QVariant qvTmp(tmp);
            sampleModel->setData(sampleModel->index(i,321),qvTmp);
        }
    }
}

void sampleJournal::on_tableView_doubleClicked(const QModelIndex &index)
{
    if (setChoosen){
        switch (ui->sampleTypeComboBox->currentIndex()){
            case 0:
                sampleInfo[1] = sampleModel->record(index.row()).value("F_Country_Code").toString();
                sampleInfo[2] = sampleModel->record(index.row()).value("F_Client_ID").toString();
                sampleInfo[3] = sampleModel->record(index.row()).value("F_Year").toString();
                sampleInfo[4] = sampleModel->record(index.row()).value("F_Sample_Set_ID").toString();
                sampleInfo[5] = sampleModel->record(index.row()).value("F_Sample_Set_Index").toString();
                sampleInfo[6] = sampleModel->record(index.row()).value("A_Sample_ID").toString();
            break;
            case 1:
                sampleInfo[1] = "s";
                sampleInfo[2] = "s";
                sampleInfo[3] = "s";
                sampleInfo[4] = standartModel->record(index.row()).value("SRM_Set_Name").toString();
                sampleInfo[5] = standartModel->record(index.row()).value("SRM_Set_Number").toString();
                sampleInfo[6] = standartModel->record(index.row()).value("SRM_Number").toString();
            break;
            case 2:
                sampleInfo[1] = "m";
                sampleInfo[2] = "m";
                sampleInfo[3] = "m";
                sampleInfo[4] = monitorModel->record(index.row()).value("Monitor_Set_Name").toString();
                sampleInfo[5] = monitorModel->record(index.row()).value("Monitor_Set_Number").toString();
                sampleInfo[6] = monitorModel->record(index.row()).value("Monitor_Number").toString();
            break;
        }
        emit sampleChoosen(sampleInfo);
        this->close();
    }
    else{
        switch (ui->sampleTypeComboBox->currentIndex()){
            case 0:
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
                updateTableFileNum();
            break;
            case 1:
                setInfo[0] = "s";
                setInfo[1] = "s";
                setInfo[2] = "s";
                setInfo[3] = standartSetModel->record(index.row()).value("SRM_Set_Name").toString();
                setInfo[4] = standartSetModel->record(index.row()).value("SRM_Set_Number").toString();
                standartModel->setFilter(QString("SRM_Set_Name like '%%1%' AND SRM_Set_Number like '%%2%'").arg(setInfo[3], setInfo[4]));
                ui->tableView->setModel(standartModel);
                for (int i =6; i < 16; ++i){
                    ui->tableView->hideColumn(i);
                }
                for (int i =18; i < 29; ++i){
                    ui->tableView->hideColumn(i);
                }

            break;
            case 2:
                setInfo[0] = "m";
                setInfo[1] = "m";
                setInfo[2] = "m";
                setInfo[3] = monitorSetModel->record(index.row()).value("Monitor_Set_Name").toString();
                setInfo[4] = monitorSetModel->record(index.row()).value("Monitor_Set_Number").toString();
                monitorModel->setFilter(QString("Monitor_Set_Name like '%%1%' AND Monitor_Set_Number like '%%2%'").arg(setInfo[3], setInfo[4]));
                ui->tableView->setModel(monitorModel);
                ui->tableView->hideColumn(6);
                ui->tableView->hideColumn(9);
            break;
        }
        ui->sampleTypeComboBox->setEnabled(0);
        if (englang)
            ui->label->setText("Sample set choosen: " + setInfo[0] + "-" + setInfo[1] + "-" + setInfo[2] + "-" + setInfo[3] + "-" + setInfo[4]);
        else
            ui->label->setText("Выбрана партия: " + setInfo[0] + "-" + setInfo[1] + "-" + setInfo[2] + "-" + setInfo[3] + "-" + setInfo[4]);
        setChoosen = true;
    }
}

void sampleJournal::on_chooseButton_clicked()
{
    on_tableView_doubleClicked(ui->tableView->currentIndex());
}


void sampleJournal::on_sampleTypeComboBox_currentIndexChanged(int index)
{
    switch(index){
        case 0:
            ui->tableView->setModel(setModel);
        break;
        case 1:
            ui->tableView->setModel(standartSetModel);
        break;
        case 2:
            ui->tableView->setModel(monitorSetModel);
        break;
    }
}
