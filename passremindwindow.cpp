#include "passremindwindow.h"
#include "ui_passremindwindow.h"
MainWindow *parent_window;


PassRemindWindow::PassRemindWindow(QWidget *parent, MainWindow *parent2, QString sQuestion) :
    QDialog(parent),
    ui(new Ui::PassRemindWindow)
{
    parent_window = parent2;
    ui->setupUi(this);
    ui->squestionLabel->setText(sQuestion);
}

PassRemindWindow::~PassRemindWindow()
{
    delete ui;
}

void PassRemindWindow::on_pushButton_clicked()
{
    if(parent_window->recoverPass(ui->sanswerEdit->text())) this->close();
}
