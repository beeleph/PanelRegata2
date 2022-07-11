#ifndef SAMPLEJOURNAL_H
#define SAMPLEJOURNAL_H

#include <QWidget>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QSqlRecord>

namespace Ui {
class sampleJournal;
}

class sampleJournal : public QWidget
{
    Q_OBJECT

public:
    explicit sampleJournal(bool englang, QWidget *parent = nullptr);
    ~sampleJournal();

private slots:
    void on_cancelButton_clicked();

    void on_sliButton_clicked();

    void on_lliButton_clicked();

    void on_comboBox_currentTextChanged(const QString &arg1);

    void on_tableView_doubleClicked(const QModelIndex &index);

    void on_chooseButton_clicked();

signals:
    void sampleChoosen(QVector<QString> smth);
private:
    Ui::sampleJournal *ui;
    void updateTable(QString dateStart);
    QSqlTableModel *sampleModel, *setModel;
    QVector<QString> sampleInfo, setInfo;
    bool setChoosen = false;
};

#endif // SAMPLEJOURNAL_H
