#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "database.h"
#include "passremindwindow.h"
#include <QSqlDatabase>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QClipboard>



QSqlDatabase db; // база данных юзеров
QString active_username; // логин активного юзера
int active_id; // id активного юзера
bool active_can_edit; // наличие права редактировать рабочую базу данных у активного юзера

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    QIcon icon("ikeaicon.ico");
    this->setWindowIcon(icon);

    db = QSqlDatabase::addDatabase("QSQLITE"); // дефолтное соединение драйвером sqlite
    db.setDatabaseName("pass_db.db");  // к базе данных pass_db.db

    if(!QFile("pass_db.db").exists()) // если база  не существует
    {
        if(!db.open()) qDebug() << "pass db create failed"; // создаем файл базы данных
        QSqlQuery query(db); // запрос к базе данных для создания таблицы с данными пользователей
        if(!query.exec( "CREATE TABLE passtable  ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, " // id ключ
                    "login VARCHAR(255)            NOT NULL,"           // логин
                   "password VARCHAR            NOT NULL,"          // пароль
                   "can_edit BOOLEAN         NOT NULL,"             // право на редактирование рабочей базы данных
                   "s_question VARCHAR(255)    NOT NULL,"       // секретный вопрос для восстановления пароля
                   "s_answer VARCHAR(255)    NOT NULL"          // ответ на секретный вопрос
                " )"
            )) qDebug() << "pass db failed" << query.lastError().text();

        registerNewUser("admin","qwerty", true,"Favorite color", "blue"); // регистрируем двух пользователей по умолчанию
        registerNewUser("user","123", false,"Favorite pet", "cat");
    }
    else
    {
       if(!db.open()) qDebug() << "pass db open failed"; // если база существует то просто открываем ее
    }

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    bool login_found=false; // найден ли введный логин в бд
    bool pass_valid=false; // правильный ли пароль для найденного логина

    QString login = ui->loginEdit->text();      // берем введенные логин и пароль
    QString password = ui->passEdit->text();

    QSqlQuery query(db); // запрос для поиска записи в бд с введеным логином
    query.prepare("SELECT id, login, password, can_edit FROM passtable WHERE login = (:login)"); // запрос вернет id, логин, пароль и право на редактирование из записи с нужным логином, если такая есть
    query.bindValue(":login",login); // добавляем параметр к запросу
    if(!query.exec()) qDebug() << "search failed" << query.lastError().text(); // выполняем

    while (query.next()) // перебираем результаты запроса
    {
       login_found=true; // если сюда попали, значит есть хотя бы одна запись с таким логином
       if(password==query.value(2).toString()) // берем пароль из результата и сравниваем с введенным
       {
           pass_valid=true; // если они равны, то пароль верен
           active_id = query.value(0).toInt();
           active_username=query.value(1).toString(); // записываем id, логин и право на редактирование
           active_can_edit=query.value(3).toBool();
       }
      qDebug() << query.value(0).toInt() <<" "<< query.value(1).toString() << query.value(2).toString() << query.value(3).toBool(); // все подряд в дебаг лог
    }

    if(pass_valid) // если пароль совпал
    {
        Database *secondWindow = new Database(this, this, active_id, active_username, active_can_edit); // то открываем второе окно для работы с рабочей базой данных
        secondWindow->show();
        active_can_edit=false;
    }

    if((login_found)&&(!pass_valid)) // если такой логин был, но пароль неверен, то выводим сообщение об этом
    {
        QMessageBox msgBox;
        msgBox.setText("Неверный пароль.");
        msgBox.exec();
    }

    if(!login_found) // если логин не найден, то выводим сообщение об этом
    {
        QMessageBox msgBox;
        msgBox.setText("Несуществующее имя пользователя.");
        msgBox.exec();
    }

}

bool MainWindow::changePass(QString password) // метод смены пароль в базе данных пользователей
{
    QSqlQuery query(db); // запрос для базы данных
    query.prepare("UPDATE passtable SET password = (:password) WHERE login = (:login)"); // обновить запись с логином активного юзера на пароль из параметра
    query.bindValue(":password", password); // прицепляем параметры к запросу
    query.bindValue(":login", active_username);
    if(!query.exec()) {
        qDebug() << "smena pass failed" << query.lastError().text(); // если запрос неудался, то выводим в дебаг
        return false;
    }
    else return true; // если удался, то все хорошо
}

bool MainWindow::registerNewUser(QString login, QString password, bool can_edit, QString sQuestion, QString sAnswer)
{

    bool duplicated_login = false;
    QSqlQuery query1(db);
    query1.prepare("SELECT id FROM passtable WHERE login = (:login)"); // запрос вернет id, логин, пароль и право на редактирование из записи с нужным логином, если такая есть
    query1.bindValue(":login",login); // добавляем параметр к запросу

    if(query1.exec())
    {
    while(query1.next()) duplicated_login = true; // ищем запись с таким логином, если есть хоть одна тогда ставим флаг на true
    if(!duplicated_login){
        QSqlQuery query(db); // запрос для создания первого пользователя admin

        query.prepare("INSERT INTO passtable (login, password, can_edit, s_question, s_answer) "
                             "VALUES (:login, :password, :can_edit, :s_question, :s_answer)");

        query.bindValue(":login", login); // прицепляем логин
        query.bindValue(":password", password); // пароль
        if(can_edit) query.bindValue(":can_edit", 1);  // право на редактирование
        else query.bindValue(":can_edit",0); // или его отсутствие
        query.bindValue(":s_question", sQuestion); // секретный вопрос
        query.bindValue(":s_answer", sAnswer); // и ответ на него
    if(!query.exec())
    {
        qDebug() << "user register query failed" <<query.lastError().text();
        return false;
    }
    else return true;
    }
    else
    {
        qDebug() << "duplicated login";
        return false;
    }
    }
    else
    {
        qDebug() << "duplicate login search failed" << query1.lastError().text();
        return false;
    }
}


bool MainWindow::recoverPass(QString sAnswer)
{
    bool found_login = false; // найден ли такой логин
    bool right_answer = false; // правильный ли ответ
    QString password;
    QSqlQuery query1(db);
    query1.prepare("SELECT password, s_answer FROM passtable WHERE login = (:login)"); // запрос вернет  пароль и ответ на секретный вопрос из записи с нужным логином, если такая есть
    query1.bindValue(":login", ui->loginEdit->text()); // добавляем параметр к запросу

    if(!query1.exec())
    {
        qDebug() << "search failed " << query1.lastError();
        return false;
    }
    else
    {
    while(query1.next()) // перебираем результат
    {
        found_login = true; // если хотя бы одна запись с таким логином, то ставим флаг на true
        if(query1.value(1).toString()==sAnswer) // если совпадает ответ
        {
            right_answer = true; // ставим флаг на true
            password = query1.value(0).toString(); // достаем пароль из запроса
        }
    }

    // начинаем проверку полученных флагов
    if(found_login && right_answer) // если логин и ответ на секретный вопрос верны
    {
        ui->passEdit->setText(password); // то записываем в поле ввода пароля восстановленный пароль
        QClipboard *clipboard = QApplication::clipboard();
        clipboard->setText(password); // и в буфер обмена

        QMessageBox msgBox(this);
        msgBox.setText("Ваш пароль успешно восстановлен : " + password + "\n" + "Так же пароль был вставлен в поле ввода пароля и в буфер обмена."); // и выводим в сообщении
        msgBox.exec();
        return true;
    }
    if(found_login && (!right_answer)) // если логин верный, но ответ нет
    {
        QMessageBox msgBox(this);
        msgBox.setText("Неверный ответ на секретный вопрос."); // выводим сообщение об этом
        msgBox.exec();
        return false;
    }
    if(!found_login) // если логин не верен
    {
        QMessageBox msgBox(this);
        msgBox.setText("Неверное имя пользователя."); // выводим сообщение об этом
        msgBox.exec();
        return false;
    }
    return false;
    }

}

void MainWindow::on_pushButton_2_clicked()  // нажатие кнопки восстановления пароля
{
    bool login_found = false; // найден ли логин
    QString sQuestion; // секретный вопрос
    if(ui->loginEdit->text().isEmpty()) // если поле ввода логина пусто
    {
        QMessageBox msgBox(this);
        msgBox.setText("Чтобы восстановить пароль, введите логин в поле ввода логина и нажмите кнопку восстановить пароль."); // то выводим сообщение с инструкцией
        msgBox.exec();
    }
    else
    {
        QSqlQuery query(db);
        query.prepare("SELECT s_question FROM passtable WHERE login = (:login)"); // запрос вернет  пароль и ответ на секретный вопрос из записи с нужным логином, если такая есть
        query.bindValue(":login", ui->loginEdit->text()); // добавляем параметр к запросу

        if(!query.exec()) qDebug() << "search failed" << query.lastError().text();
        else
        {
            while(query.next()) // если есть такой логин
            {
                login_found = true; // ставим флаг
                sQuestion = query.value(0).toString(); // записываем секретный вопрос
            }

            if(login_found) // если нашли логин
            {
                PassRemindWindow *fourth_window = new PassRemindWindow(this, this, sQuestion); // то вызываем окно восстановления пароля
                fourth_window->show();
            }
            else // если нет
            {
                QMessageBox msgBox(this);
                msgBox.setText("Неверный логин."); // выводим сообщение
                msgBox.exec();
            }
        }
    }
}
