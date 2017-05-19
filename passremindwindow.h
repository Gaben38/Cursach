#ifndef PASSREMINDWINDOW_H
#define PASSREMINDWINDOW_H

#include <QDialog>
#include "mainwindow.h"

namespace Ui {
class PassRemindWindow;
}

class PassRemindWindow : public QDialog
{
    Q_OBJECT

public:
    explicit PassRemindWindow(QWidget *parent = 0, MainWindow *parent2 = 0, QString sQuestion = "error, no secret question");
    ~PassRemindWindow();

private slots:
    void on_pushButton_clicked();

private:
    Ui::PassRemindWindow *ui;
};

#endif // PASSREMINDWINDOW_H
