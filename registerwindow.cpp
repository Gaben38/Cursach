#include "registerwindow.h"
#include "ui_registerwindow.h"


Database *parentWindow;

RegisterWindow::RegisterWindow(QWidget *parent, Database *parent2) :
    QDialog(parent),
    ui(new Ui::RegisterWindow)
{
    parentWindow=parent2;
    ui->setupUi(this);
}

RegisterWindow::~RegisterWindow()
{
    delete ui;
}

void RegisterWindow::on_registerButton_clicked()
{
    if(ui->passLineEdit1->text()==ui->passLineEdit2->text())
    {
        QString username = ui->loginLineEdit->text();
        QString password = ui->passLineEdit1->text();
        bool can_edit = ui->canEditBox->isChecked();
        QString SQuestion = ui->sQLineEdit->text();
        QString SAnswer = ui->sALineEdit->text();
        parentWindow->passRegisterUserData(username, password, can_edit, SQuestion, SAnswer);
        this->close();
    }
    else
    {
        QMessageBox msgBox(this);
        msgBox.setText("Введенные пароли не совпадают.");
        msgBox.exec();
    }
}
