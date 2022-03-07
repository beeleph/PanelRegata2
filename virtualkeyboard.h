#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H

#include <QFrame>
#include <QLineEdit>

namespace Ui {
class VirtualKeyboard;
}

class VirtualKeyboard : public QFrame
{
    Q_OBJECT

public:
    explicit VirtualKeyboard(QWidget *parent = nullptr);
    ~VirtualKeyboard();
    void connect(QLineEdit* laino);

signals:
    void ok();
    void cancel();

private slots:
    void on_cancelButton_clicked();

    void on_okButton_clicked();

    void on_oneButton_clicked();

    void on_twoButton_clicked();

    void on_threeButton_clicked();

    void on_fourButton_clicked();

    void on_fiveButton_clicked();

    void on_sixButton_clicked();

    void on_sevenButton_clicked();

    void on_eightButton_clicked();

    void on_nineButton_clicked();

    void on_zeroButton_clicked();

private:
    void addSmth(QString smth);

    Ui::VirtualKeyboard *ui;
    QString wasBefore = 0;
    QString currentInput;
    QLineEdit* currentLE;
};

#endif // VIRTUALKEYBOARD_H
