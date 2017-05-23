#include "newitemwindow.h"
#include "ui_newitemwindow.h"


Database *parent_db_window; // родительское окно
QString imgName = ""; // путь к картинке
bool imgSelected = false; // выбрана ли картинка

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

void NewItemWindow::on_selectImgButton_clicked() // кнопка для выбора картинки
{
    QFileDialog openImgDialog(this); // диалоговое окно выбора файла
    openImgDialog.setFileMode(QFileDialog::ExistingFile); // можно выбрать только существующие файлы
    openImgDialog.setNameFilter("Images (*.jpg *.png)"); // расширения jpg или png
    openImgDialog.setDirectory(QStandardPaths::displayName(QStandardPaths::DesktopLocation)); // в начале работы диалоговго окна открыт рабочий стол
    if(openImgDialog.exec()) // вызываем диалоговое окно
    {
        imgName=openImgDialog.selectedFiles().at(0); // достаем путь к выбранному файл
        imgSelected = true; // ставим флаг

        QPixmap pixElem(imgName);
        ui->imgPreviewLabel->setPixmap(pixElem); // выводим картинку в превью

        qDebug() << imgName;
    }
}

void NewItemWindow::on_addItemButton_clicked() // кнопка завершения работы и добавления товара с введенными данными
{
    if(imgSelected) // если была выбрана картинка
    {
        QString item_name = ui->nameEdit->text();
        int type = ui->typeComboBox->currentIndex(); // записываем введенные имя, тип, цену и количество
        int price = ui->priceSpinBox->value();
        int amount = ui->amountSpinBox->value();
        if(parent_db_window->newIkeaItem(item_name, type, price, amount, imgName)) // и вызываем функция из родительского окна с этими параметрами, если успешно добавлено
        {
            QMessageBox msgBox(this);
            msgBox.setText("Товар успешно добавлен."); // то выводим сообщение
            int n = msgBox.exec();
            if(n==QMessageBox::Ok)
            {
                //parent_db_window->refreshTableView();
                this->close(); //  и закрываем окно после нажатия ок
            }
        }
        else
        {
            QMessageBox msgBox(this);
            msgBox.setText("Ошибка при добавлении товара."); // то выводим сообщение об ошибке
            msgBox.exec();
        }
    }
    else // если не была выбрана
    {
        QMessageBox msgBox1(this);
        msgBox1.setText("Необходимо выбрать изображение товара."); // то выводим сообщение
        msgBox1.exec();
    }
}
