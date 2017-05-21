#include "newitemwindow.h"
#include "ui_newitemwindow.h"


Database *parent_db_window;
QString imgName = "";
bool imgSelected = false;

NewItemWindow::NewItemWindow(QWidget *parent, Database *parent2) :
    QDialog(parent),
    ui(new Ui::NewItemWindow)
{
    parent_db_window = parent2;

    //connect(this, SIGNAL(finished()), parent_db_window, SLOT(parent_db_window::fourth_window_finished()));
    ui->setupUi(this);

    ui->typeComboBox->addItem("мебель",1);
    ui->typeComboBox->addItem("посуда",2);
    ui->typeComboBox->addItem("другое",3);
}

NewItemWindow::~NewItemWindow()
{
    delete ui;
}

void NewItemWindow::on_selectImgButton_clicked()
{
    QFileDialog openImgDialog(this);
    openImgDialog.setFileMode(QFileDialog::ExistingFile);
    openImgDialog.setNameFilter("Images (*.jpg *.png)");
    openImgDialog.setDirectory("C:/Users/Gaben/Desktop/");
    if(openImgDialog.exec())
    {
        imgName=openImgDialog.selectedFiles().at(0);
        imgSelected = true;

        QPixmap pixElem(imgName);
        ui->imgPreviewLabel->setPixmap(pixElem);

        qDebug() << imgName;
    }
}

void NewItemWindow::on_addItemButton_clicked()
{
    if(imgSelected)
    {
        QString item_name = ui->nameEdit->text();
        int type = ui->typeComboBox->currentIndex();
        int price = ui->priceSpinBox->value();
        int amount = ui->amountSpinBox->value();
        if(parent_db_window->newIkeaItem(item_name, type, price, amount, imgName))
        {
            QMessageBox msgBox(this);
            msgBox.setText("Товар успешно добавлен.");
            int n = msgBox.exec();
            if(n==QMessageBox::Ok)
            {
                //parent_db_window->refreshTableView();
                this->close();
            }
        }
        else
        {
            QMessageBox msgBox(this);
            msgBox.setText("Ошибка при добавлении товара.");
            msgBox.exec();
        }
    }
    else
    {
        QMessageBox msgBox1(this);
        msgBox1.setText("Необходимо выбрать изображение товара.");
        msgBox1.exec();
    }
}
