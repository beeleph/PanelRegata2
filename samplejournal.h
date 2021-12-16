#ifndef SAMPLEJOURNAL_H
#define SAMPLEJOURNAL_H

#include <QWidget>
#include <QSqlQuery>
#include <QSqlTableModel>

namespace Ui {
class sampleJournal;
}

class sampleJournal : public QWidget
{
    Q_OBJECT

public:
    explicit sampleJournal(QWidget *parent = nullptr);
    ~sampleJournal();

private slots:
    void on_cancelButton_clicked();

    void on_sliButton_clicked();

    void on_lliButton_clicked();

    void on_comboBox_textActivated(const QString &arg1);

private:
    Ui::sampleJournal *ui;
    void updateTable(QString dateStart);
    QSqlTableModel *sliModel, *lliModel;
};

#endif // SAMPLEJOURNAL_H
