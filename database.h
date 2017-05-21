#ifndef DATABASE_H
#define DATABASE_H

#include <QDialog>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <QSqlQuery>
#include <QSqlTableModel>
#include <QPixmap>
#include <QSqlError>
#include <QDir>
#include "mainwindow.h"

namespace Ui {
class Database;
}

class Database : public QDialog
{
    Q_OBJECT

public:
    explicit Database(QWidget *parent = 0, MainWindow *test=0, int id=0, QString username="",bool can_edit_param=false);
    bool loadDb(QString dbname = "ikea_db.db");
    void passRegisterUserData(QString login, QString password, bool can_edit, QString SQuestion, QString SAnswer);
    bool newIkeaItem(QString name, int type, int price, int number, QString imgPath);
    int getMaxIndex();
    void refreshTableView();
    ~Database();
public slots:
    void fourth_window_finished();
private slots:
    void on_Database_finished();

    void on_changePassButton_clicked();

    void on_searchLineEdit_returnPressed();

    void on_registerButton_clicked();

    void on_exitButton_clicked();

    void on_resetsearchButton_clicked();

    void on_amountLeftProgressbar_valueChanged(int value);

    void on_tableView_clicked(const QModelIndex &index);

    void on_newDBelemButton_clicked();

private:
    Ui::Database *ui;
};

#endif // DATABASE_H
