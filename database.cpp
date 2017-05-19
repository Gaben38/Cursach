#include "database.h"
#include "ui_database.h"
#include "registerwindow.h"

QWidget *mainw; // виджет главного окна
MainWindow *param2; // главное окно
bool can_edit; // право редактирования активного юзера
QSqlDatabase ikea_db; // рабочая база данных
 QSqlTableModel *model; // табличная модель для базы данных

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
    if(!loadDb("ikea_db.db"))
    {
        qDebug() << "ошибка при загрузке базы данных икеа";

        QMessageBox msgBox(this);
        msgBox.setText("Ошибка при загрузке базы данных ikea.");
        msgBox.exec();
    }

    ui->amountLeftProgressbar->setAlignment(Qt::AlignCenter);
    QString danger = "QProgressBar::chunk {background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0,stop: 0 #FF0350,stop: 0.4999 #FF0020,stop: 0.5 #FF0019,stop: 1 #FF0000 );border-bottom-right-radius: 5px;border-bottom-left-radius: 5px;border: .px solid black;}";
    QString safe= "QProgressBar::chunk {background: QLinearGradient( x1: 0, y1: 0, x2: 1, y2: 0,stop: 0 #78d,stop: 0.4999 #46a,stop: 0.5 #45a,stop: 1 #238 );border-bottom-right-radius: 7px;border-bottom-left-radius: 7px;border: 1px solid black;}";
    ui->amountLeftProgressbar->setStyleSheet(safe);
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
    ui->tableView->setModel(model); // ставим модель для tableView и обновляем его
    ui->tableView->show();
}
