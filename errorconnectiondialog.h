#ifndef ERRORCONNECTIONDIALOG_H
#define ERRORCONNECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class errorConnectionDialog;
}

class errorConnectionDialog : public QDialog
{
    Q_OBJECT

public:
    explicit errorConnectionDialog(QWidget *parent = nullptr);
    ~errorConnectionDialog();

private slots:
    void on_pushButton_clicked();

private:
    Ui::errorConnectionDialog *ui;
};

#endif // ERRORCONNECTIONDIALOG_H
