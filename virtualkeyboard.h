#ifndef VIRTUALKEYBOARD_H
#define VIRTUALKEYBOARD_H

#include <QFrame>
#include <QSpinBox>

namespace Ui {
class VirtualKeyboard;
}

class VirtualKeyboard : public QFrame
{
    Q_OBJECT

public:
    explicit VirtualKeyboard(QWidget *parent = nullptr);
    ~VirtualKeyboard();
    void connect(QSpinBox* laino);

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

private:
    void addSmth(QString smth);

    Ui::VirtualKeyboard *ui;
    int wasBefore = 0;
    QString currentInput;
    QSpinBox* currentSpinny;
};

#endif // VIRTUALKEYBOARD_H
