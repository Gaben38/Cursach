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

int searchIndex = 1; // индекс пункта выбранного в searchCombobox

Database::Database(QWidget *parent, MainWindow *test, int _id, QString username, bool can_edit_param) : // username - логин активного юзера
    QDialog(parent),
    ui(new Ui::Database)
{
    mainw = parent; // записываем переданные параметры
    param2=test;
    can_edit=can_edit_param;
    mainw->hide(); // скрываем главное окно

    ui->setupUi(this);

    ui->searchComboBox->addItem("По id",1); // добавляем пункты в combobox
    ui->searchComboBox->addItem("По названию",2);
    ui->searchComboBox->addItem("По типу",3);
    ui->searchComboBox->addItem("По диапазону цен",4);

    QPixmap pixIkea("ikea.png"); // загружаем картинку
    ui->imgLabel->setPixmap(pixIkea);

    ui->nameLabel->setText(username); // выводим логин в nameLabel
    qDebug() << "id =" <<_id;
    if(_id==1) // если пользователь - админ, то включаем кнопки регистации и добавления нового товара, и выключаем всплывающие подсказки для них
    {
        ui->registerButton->setEnabled(true);
        ui->registerButton->setToolTip("");
        ui->newDBelemButton->setEnabled(true);
        ui->newDBelemButton->setToolTip("");
    }
    if(!loadDb("ikea_db.db"))
    {
        qDebug() << "ошибка при загрузке базы данных икеа";

        QMessageBox msgBox(this);
        msgBox.setText("Ошибка при загрузке базы данных ikea.");
        msgBox.exec();
    }

    ui->amountLeftProgressbar->setAlignment(Qt::AlignCenter);
    ui->amountLeftProgressbar->setStyleSheet(style_40_100);

    if(!(QDir(QDir::currentPath() + "/ikea_imgs").exists())) // создаем папку для картинок товаров, если ее нет
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
        qDebug() << "ikea doesnt exist"; // выводим в дебаг
        if(!ikea_db.open()) qDebug() << "ikea db open/create failed"; // создаем файл базы данных
        QSqlQuery query(ikea_db); // запрос к базе данных для создания таблицы с данными пользователей
        if(!query.exec( "CREATE TABLE ikea_table  ("
                    "id INTEGER PRIMARY KEY AUTOINCREMENT, " // id ключ
                    "name VARCHAR(255)            NOT NULL,"           // логин
                   "type VARCHAR            NOT NULL,"          // пароль
                   "price INTEGER    NOT NULL,"       // секретный вопрос для восстановления пароля
                   "number INTEGER    NOT NULL"          // ответ на секретный вопрос
                " )"
            )) qDebug() << "pass db failed" << query.lastError().text();

        QMessageBox msgBox(this);
        msgBox.setText("Отсутствует база данных товаров. Была создана пустая база данных. Для заполнения используйте форму добавления товаров (только для admin).");
        msgBox.exec();

    }
    else
    {
        if(!ikea_db.open()) qDebug() << "ikea db open/create failed"; // открываем файл базы данных
    }
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

void Database::on_searchLineEdit_returnPressed() // нажат enter в searchLineEdit
{
    startSearch(); // запускаем прверку условий для поиска
}

void Database::startSearch() // проверка условий для поиска
{
    if((ui->searchLineEdit->text().isEmpty()) && (ui->searchLineEdit->isEnabled())) // если в searchLineEdit нет текста и он включен
    {
        QMessageBox msgBox(this);
        msgBox.setText("Ошибка. Введите значание для запроса.");
        msgBox.exec(); // выводим сообщение об ошибке
    }
    else // иначе
    {
        if(ui->priceRangeFromSpinbox->isEnabled()) // если spinBox включен
        {
            if(ui->priceRangeFromSpinbox->value() > ui->priceRangeToSpinbox->value()) // и нижняя граница диапазона больше верхней
            {
                QMessageBox msgBox(this);
                msgBox.setText("Ошибка. Введен неверный диапазон.");
                msgBox.exec(); // выводим сообщение об ошибке
            }
            else // если диапазон верный
            {
                ikeaSearch(); // запускаем поиск
            }
        }
        else // если в lineEdit есть текст
        {
            ikeaSearch(); // запускаем поиск
        }
    }
}

void Database::passRegisterUserData(QString login, QString password, bool can_edit, QString SQuestion, QString SAnswer) // функция для передачи данных для регистрации нового пользователя из дочернего окна RegisterWindow в родительский MainWindow
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


void Database::on_registerButton_clicked() // вызов окна для регистации нового пользователя
{
    RegisterWindow *third_window = new RegisterWindow(this, this);
    third_window->show();
}

void Database::on_exitButton_clicked() // кнопка выхода
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

void Database::on_resetsearchButton_clicked() // кнопка вывода исходного вида таблицы, без поиска
{
    refreshTableView();
}

void Database::on_amountLeftProgressbar_valueChanged(int value) // слот вызываемый при изменении значения в прогрессбаре, меняет цвет в зависимости от значения
{
    if(value>=40) ui->amountLeftProgressbar->setStyleSheet(style_40_100); // больше 40, цвет = зеленый
    else if(value>=20) ui->amountLeftProgressbar->setStyleSheet(style_20_40); // от 20 до 39, цвет = оранжевый
    else ui->amountLeftProgressbar->setStyleSheet(style_0_20); // меньше 19, цвет = красный
}

void Database::on_tableView_clicked(const QModelIndex &index) // нажатие на элемент tableview
{
     int i = index.sibling(index.row(),0).data().toInt(); // id из выбранной строки
     QString img_path = "Img_missing";

     qDebug() << i;
     QSqlQuery query(ikea_db);
     query.prepare("SELECT * FROM ikea_table WHERE id = (:id)"); // запрашиваем данные для данной строки(товара)
     query.bindValue(":id",i);

     if(!query.exec()) qDebug() << "search failed" << query.lastError().text();
     while(query.next())
     {
            ui->elemNameLabel->setText(query.value(1).toString()); // выводим название товара
            ui->elemTypeLabel->setText(query.value(2).toString()); // тип
            ui->elemCostLabel->setText(query.value(3).toString() + " руб."); // цену
            ui->amountLeftProgressbar->setValue(query.value(4).toInt()); // и обновляем прогрессбар количеством товара

            img_path = QDir::currentPath() + "/ikea_imgs/" + QString::number(i); // путь к картинке товара
            qDebug() << img_path;

            if(QFile(img_path+".png").exists()) img_path+=".png"; // ищем png с таким названием, если нашли то дописываем расширение картинки к пути
            else if(QFile(img_path+".jpg").exists()) img_path+=".jpg"; // то же самое для jpg
            else img_path="Img_missing"; // если ни png, ни jpg нет, то записываем в путь Img_missing

            if(img_path!="Img_missing") // если есть картинкиа таким номером
            {
                QPixmap pixElem(img_path);
                ui->elemImgLabel->setPixmap(pixElem); // загружаем ее
            }
            else ui->elemImgLabel->setText(img_path); //если нет, то выводим текст Img_missing
     }
}

int Database::getMaxIndex() // функция для нахождения максимального id в базе данных
{
    QSqlQuery query(ikea_db);
    query.prepare("SELECT  COALESCE(MAX(id),0) FROM ikea_table"); // подготавливаем простой запрос
    if(!query.exec()) // и исполняем его
    {
        qDebug() << "max index is broken" << query.lastError().text();
        return -1;
    }
    else
    {
        while(query.next())
        {
            qDebug() << query.value(0).toString(); // если запрос успешно прошел, то возвращаем значение
            return query.value(0).toInt();
        }
        return -1;

    }
}

void Database::refreshTableView() // функция обновления модели и tableView
{
    model->select(); // делаем выборку из таблицы
    ui->tableView->setModel(model); // ставим модель для tableView и обновляем его
    ui->tableView->show();
}

void Database::fourth_window_finished() // слот вызываемый при завершении работы окна для добавления нового товара
{
    refreshTableView(); // обновляем tableview
}

void Database::on_newDBelemButton_clicked() // кнопка вызова окна добавления нового товара
{
    NewItemWindow *fourth_window = new NewItemWindow(this); // создаем окно
    connect(fourth_window, SIGNAL(finished(int)),this, SLOT(fourth_window_finished())); // соединяем сигнал завершения этого окна со слотом обновления tableview
    fourth_window->show(); // показываем окно
}

bool Database::newIkeaItem(QString name, int type, int price, int number, QString imgPath) // функция добавления нового товара
{
    QSqlQuery item_query(ikea_db);
    item_query.prepare("INSERT INTO ikea_table (name, type, price, number) "
                         "VALUES (:name, :type, :price, :number)"); // делаем запрос для добавления записи нового товара

    item_query.bindValue(":name", name); // добавляем в него имя товара

    switch(type){ // соответствующий тип
    case 0: item_query.bindValue(":type","мебель"); break;
    case 1: item_query.bindValue(":type","посуда"); break;
    case 2: item_query.bindValue(":type","другое"); break;
    default: item_query.bindValue(":type","другое"); break;
    }
    item_query.bindValue(":price",price); // цену
    item_query.bindValue(":number",number); // количество

    if(!item_query.exec()) // вызывем
    {
        qDebug() << "new item insert query failed: " << item_query.lastError().text();
        return false;
    }
    else // если все хорошо, начинаем копировать указанную картинку в папку для картинок товаров
    {
        QFileInfo imgFileInfo(imgPath); // информация об указанном файле картинки
        QString fileExtension = "." + imgFileInfo.suffix(); // расширение этого файла

        int maxIndex = getMaxIndex(); // находим индекс последней добавленной записи
        if(maxIndex==-1) return false;

        QString imgDir = QDir::currentPath() + "/ikea_imgs/" + QString::number(maxIndex) + fileExtension; // собираем путь для копирования указанного файла картинки из рабочего каталога, папки ikea_imgs, номера записи и расширения
        qDebug() << imgDir;
        if(!QFile::copy(imgPath, imgDir)) // копируем
        {
           qDebug() << "file copy failed";
           return false;
        }
        else
        {
            return true; // если все хорошо, возвращаем true
        }
    }

}

void Database::ikeaSearch() // функция поиска по дб
{
    QSqlQueryModel *model2 = new QSqlQueryModel(this); // создаем запросную модель для отображения базы данных
    QSqlQuery query(ikea_db);
    int n = ui->searchComboBox->currentIndex(); // берем индекс выбранного пункта в combobox
    qDebug() << "n=" << n;
    switch (n)
    {
        case 0: query.prepare("SELECT * FROM ikea_table WHERE id = (:id)"); query.bindValue(":id",ui->searchLineEdit->text().toInt()); break; // если поиск по id, ищем id
        case 1: query.prepare("SELECT * FROM ikea_table WHERE name LIKE (:name)"); query.bindValue(":name","%"+ui->searchLineEdit->text()+"%"); break; // если имя, то имя
        case 2: query.prepare("SELECT * FROM ikea_table WHERE type LIKE (:type)"); query.bindValue(":type","%"+ui->searchLineEdit->text()+"%"); break; // если тип, то тип
        case 3: query.prepare("SELECT * FROM ikea_table WHERE price BETWEEN (:from) AND (:to)"); query.bindValue(":from", ui->priceRangeFromSpinbox->value()); query.bindValue(":to", ui->priceRangeToSpinbox->value()); break; // если диапазон цен, то ищем подходящие цены
        default: qDebug() << "wrong combobox index"; return;
    }

    if(!query.exec()) qDebug() << "search failed" << query.lastError().text(); // вызываем запрос
    model2->setQuery(query);
    ui->tableView->setModel(model2); // ставим модель для tableView и обновляем его
    ui->tableView->show();
    qDebug() << "poisk";
}

void Database::switchSearchElems(bool not_range) // переключение виджетов для поиска, not_range = true когда нужно просто поле для ввода поисковго запроса, false когда нужны spinbox ы для ввода диапазона цен
{
    ui->searchLineEdit->setEnabled(not_range); // включаем lineEdit если not_range true, выключаем если false
    ui->searchLineEdit->setVisible(not_range);

    ui->priceRangeFromSpinbox->setEnabled(!not_range); // для надписей "от" и "до"
    ui->priceRangeFromSpinbox->setVisible(!not_range);
    ui->priceRangeToSpinbox->setEnabled(!not_range);
    ui->priceRangeToSpinbox->setVisible(!not_range);

    ui->priceRangeFromLabel->setEnabled(!not_range); // и spinBox ов наборот
    ui->priceRangeFromLabel->setVisible(!not_range);
    ui->priceRangeToLabel->setEnabled(!not_range);
    ui->priceRangeToLabel->setVisible(!not_range);
}
void Database::on_searchComboBox_currentIndexChanged(int index) // слот изменения выбранного пункта в combobox поиска
{
    searchIndex = index; // записываем новый индекс
    qDebug() << searchIndex;
    switch(index)
    {
    case 0: ui->searchDescLabel->setText("Введите id товара:"); switchSearchElems(true); break; // меняем текст надписи, переключаем поисковые элементы
    case 1: ui->searchDescLabel->setText("Введите имя товара:"); switchSearchElems(true); break; // true = lineEdit включен, spinBox ы выключены
    case 2: ui->searchDescLabel->setText("Введите тип товара:"); switchSearchElems(true); break;
    case 3: ui->searchDescLabel->setText("Введите диапазон цен:"); switchSearchElems(false); break; // false наоборот
    default: qDebug() << "wrong combobox index";
    }
}

void Database::on_searchButton_clicked() // кнопка поиск
{
    startSearch(); // запускаем проверку условий для поиска
}
