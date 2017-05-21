#include "database.h"
#include "ui_database.h"
#include "registerwindow.h"
#include "newitemwindow.h"

QWidget *mainw; // виджет главного окна
MainWindow *param2; // главное окно
bool can_edit; // право редактирования активного юзера
QSqlDatabase ikea_db; // рабочая база данных
QSqlTableModel *model; // табличная модель для базы данных

QString style_40_100      = "QProgressBar::chunk{ background-color: green; }"; // зеленый цвет прогрессбара для количества товара 40-100
QString style_20_40     = "QProgressBar::chunk{ background-color: orange; }"; // оранжевый для 20-40
QString style_0_20    = "QProgressBar::chunk{ background-color: red; }"; // красный для 0-20

Database::Database(QWidget *parent, MainWindow *test, int _id, QString username, bool can_edit_param) : // username - логин активного юзера
    QDialog(parent),
    ui(new Ui::Database)
{
    mainw = parent; // записываем переданные параметры
    param2=test;
    can_edit=can_edit_param;
    mainw->hide(); // скрываем главное окно

    ui->setupUi(this);

    ui->searchComboBox->addItem("По id",1);
    ui->searchComboBox->addItem("По названию",2);

    QPixmap pixIkea("ikea.png");
    ui->imgLabel->setPixmap(pixIkea);

    ui->nameLabel->setText(username); // выводим логин в nameLabel
    qDebug() << "id =" <<_id;
    if(_id==1) ui->registerButton->setEnabled(true);
    if(_id==1) ui->newDBelemButton->setEnabled(true);
    if(!loadDb("ikea_db.db"))
    {
        qDebug() << "ошибка при загрузке базы данных икеа";

        QMessageBox msgBox(this);
        msgBox.setText("Ошибка при загрузке базы данных ikea.");
        msgBox.exec();
    }

    ui->amountLeftProgressbar->setAlignment(Qt::AlignCenter);
    ui->amountLeftProgressbar->setStyleSheet(style_40_100);

    if(!(QDir(QDir::currentPath() + "/ikea_imgs").exists()))
    {
        if(QDir().mkdir(QDir::currentPath() + "/ikea_imgs")) qDebug() << "creating directory";
        else qDebug() << "error creating directory";
    }
    qDebug() << QDir::currentPath() ;
}

Database::~Database()
{
    delete ui;
}


bool Database::loadDb(QString dbname) // метод загрузки рабочей базы данных
{
    ikea_db = QSqlDatabase::addDatabase("QSQLITE","ikea_connection"); // создаем соединение драйвером sqlite с названием ikea_connection
    ikea_db.setDatabaseName(dbname); // база данных с именем dbname

    if(!QFile(dbname).exists()) // если файл базы данных не существует
    {
        qDebug() << "ikea doesnt exist"; // выводим в дебаг и возвращаем 0

        return false;
    }
    else
    {
       if(!ikea_db.open()) qDebug() << "open ikea db failed"; // открываем базу данных
       model = new QSqlTableModel(this,ikea_db); // создаем табличную модель для отображения базы данных
       model->setTable("ikea_table"); // выбираем таблицу ikea_table и выводим ее содержимое в модель
       model->select();
       ui->tableView->setModel(model); // ставим модель для tableView и обновляем его
       ui->tableView->show();
       QRect viewRect = ui->tableView->rect();
       int column_width = viewRect.width() * 0.2;// Делаем все столбцы примерно равными
       ui->tableView->setColumnWidth(0, column_width/2);
       for(int i=1; i<5;i++) ui->tableView->setColumnWidth( i, column_width );
       if(!can_edit) ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers); // если нет права для редактирования, то меняем параметр tableView
       return true;
    }
}
void Database::on_Database_finished() // эта функция вызывается при закрытии вторго окна
{
    ui->tableView->setModel(0); // убираем модель с tableView
    delete model; // удаляем модель
    ikea_db.close(); // закрываем бд
    ikea_db = QSqlDatabase(); // записываем туда дефолтную базу данных
    ikea_db.removeDatabase("ikea_connection"); // убираем соединение ikea_connection
    mainw->show(); // делаем главное окно видимым
}

void Database::on_changePassButton_clicked() // кнопка смена пароля
{
    if(ui->passLineEdit1->text()==ui->passLineEdit2->text()) // если оба поля паролей совпадают
    {
        QMessageBox* pmbx =
                            new QMessageBox("Сменить пароль?",
                            "Вы уверены, что хотите сменить пароль?",
                            QMessageBox::Warning,
                            QMessageBox::Yes,
                            QMessageBox::No,
                            QMessageBox::Cancel | QMessageBox::Escape,
                                            this);
        int n = pmbx->exec(); // выводим сообщение подтверждения с тремя кнопками да-нет-отмена
        delete pmbx;
        if (n == QMessageBox::Yes) // если смена пароля подтверджена
        {
          if(param2->changePass(ui->passLineEdit1->text())) // вызываем метод смены пароля из главного окна и если все хорошо выводим сообщение об успехе
          {
              QMessageBox msgBox(this);
              msgBox.setText("Пароль успешно изменен.");
              msgBox.exec();
          }
          else // иначе об ошибке
          {
              QMessageBox msgBox(this);
              msgBox.setText("Ошибка при изменении пароля.");
              msgBox.exec();
          }
        }
    }
    else // если введенные пароли не совпадают, то выводим сообщение об этом
    {
        QMessageBox msgBox(this);
        msgBox.setText("Введенные пароли не совпадают.");
        msgBox.exec();
    }
}

void Database::on_searchLineEdit_returnPressed()
{
    // поиск
    QSqlQueryModel *model2 = new QSqlQueryModel(this); // создаем табличную модель для отображения базы данных
    QSqlQuery query(ikea_db);
    int n = ui->searchComboBox->currentIndex();
    qDebug() << "n=" << n;
    switch (n)
    {
        case 0: query.prepare("SELECT * FROM ikea_table WHERE id = (:id)"); query.bindValue(":id",ui->searchLineEdit->text().toInt()); break;
        case 1: query.prepare("SELECT * FROM ikea_table WHERE name LIKE (:name)"); query.bindValue(":name","%"+ui->searchLineEdit->text()+"%"); break;
        default: qDebug() << "wrong combobox index"; break;
    }

    if(!query.exec()) qDebug() << "search failed" << query.lastError().text();
    model2->setQuery(query);
    ui->tableView->setModel(model2); // ставим модель для tableView и обновляем его
    ui->tableView->show();
    qDebug() << "poisk";
}

void Database::passRegisterUserData(QString login, QString password, bool can_edit, QString SQuestion, QString SAnswer)
{
    if(!param2->registerNewUser(login, password, can_edit, SQuestion, SAnswer))
    {
        QMessageBox msgBox(this);
        msgBox.setText("Ошибка при регистрации нового пользователя.");
        msgBox.exec();
    }
    else
    {
        QMessageBox msgBox(this);
        msgBox.setText("Регистрация успешна.");
        msgBox.exec();
    }
}


void Database::on_registerButton_clicked()
{
    RegisterWindow *third_window = new RegisterWindow(this, this);
    third_window->show();
}

void Database::on_exitButton_clicked()
{
    QMessageBox* pmbx =
            new QMessageBox("Выйти?",
            "Вы уверены, что хотите выйти?",
            QMessageBox::Warning,
            QMessageBox::Yes,
            QMessageBox::No,
            QMessageBox::Cancel | QMessageBox::Escape,
                            this);
    int n = pmbx->exec(); // выводим сообщение подтверждения с тремя кнопками да-нет-отмена
    delete pmbx;
    if (n == QMessageBox::Yes) // если смена пароля подтверджена
    {
        this->close();
    }
}

void Database::on_resetsearchButton_clicked()
{
    refreshTableView();
}

void Database::on_amountLeftProgressbar_valueChanged(int value)
{
    if(value>=40) ui->amountLeftProgressbar->setStyleSheet(style_40_100);
    else if(value>=20) ui->amountLeftProgressbar->setStyleSheet(style_20_40);
    else ui->amountLeftProgressbar->setStyleSheet(style_0_20);
}

void Database::on_tableView_clicked(const QModelIndex &index)
{
     int i = index.row();
     QString img_path = "Img_missing";
     i++;
     qDebug() << i;
     QSqlQuery query(ikea_db);
     query.prepare("SELECT * FROM ikea_table WHERE id = (:id)");
     query.bindValue(":id",i);

     if(!query.exec()) qDebug() << "search failed" << query.lastError().text();
     while(query.next())
     {
            ui->elemNameLabel->setText(query.value(1).toString());
            ui->elemTypeLabel->setText(query.value(2).toString());
            ui->elemCostLabel->setText(query.value(3).toString() + " руб.");
            ui->amountLeftProgressbar->setValue(query.value(4).toInt());

            img_path = QDir::currentPath() + "/ikea_imgs/" + QString::number(i);
            qDebug() << img_path;

            if(QFile(img_path+".png").exists()) img_path+=".png";
            else if(QFile(img_path+".jpg").exists()) img_path+=".jpg";

            if(img_path!="Img_missing")
            {
                QPixmap pixElem(img_path);
                ui->elemImgLabel->setPixmap(pixElem);
            }
            else ui->elemImgLabel->setText(img_path);
     }
}

int Database::getMaxIndex()
{
    QSqlQuery query(ikea_db);
    query.prepare("SELECT MAX(id) FROM ikea_table");
    if(!query.exec())
    {
        qDebug() << "shit is broken" << query.lastError().text();
        return -1;
    }
    else
    {
        while(query.next())
        {
            qDebug() << query.value(0).toString();
            return query.value(0).toInt();
        }
        return -1;

    }
}

void Database::refreshTableView()
{
    model->select();
    ui->tableView->setModel(model); // ставим модель для tableView и обновляем его
    ui->tableView->show();
}

void Database::fourth_window_finished()
{
    refreshTableView();
}

void Database::on_newDBelemButton_clicked()
{
    NewItemWindow *fourth_window = new NewItemWindow(this);
    connect(fourth_window, SIGNAL(finished(int)),this, SLOT(fourth_window_finished()));
    fourth_window->show();
}

bool Database::newIkeaItem(QString name, int type, int price, int number, QString imgPath)
{
    QSqlQuery item_query(ikea_db);
    item_query.prepare("INSERT INTO ikea_table (name, type, price, number) "
                         "VALUES (:name, :type, :price, :number)");

    item_query.bindValue(":name", name);

    switch(type){
    case 1: item_query.bindValue(":type","мебель"); break;
    case 2: item_query.bindValue(":type","посуда"); break;
    case 3: item_query.bindValue(":type","другое"); break;
    default: item_query.bindValue(":type","другое"); break;
    }
    item_query.bindValue(":price",price);
    item_query.bindValue(":number",number);

    if(!item_query.exec())
    {
        qDebug() << "new item insert query failed: " << item_query.lastError().text();
        return false;
    }
    else
    {
        QFileInfo imgFileInfo(imgPath);
        QString fileExtension = "." + imgFileInfo.suffix();

        int maxIndex = getMaxIndex();
        if(maxIndex==-1) return false;

        QString imgDir = QDir::currentPath() + "/ikea_imgs/" + QString::number(maxIndex) + fileExtension;
        qDebug() << imgDir;
        if(!QFile::copy(imgPath, imgDir))
        {
           qDebug() << "file copy failed";
           return false;
        }
        else
        {
            return true;
        }
    }

}
