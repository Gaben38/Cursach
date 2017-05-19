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
    ~Database();

private slots:
    void on_Database_finished();

    void on_changePassButton_clicked();

    void on_searchLineEdit_returnPressed();

    void on_registerButton_clicked();

    void on_exitButton_clicked();

    void on_resetsearchButton_clicked();

private:
    Ui::Database *ui;
};

#endif // DATABASE_H
