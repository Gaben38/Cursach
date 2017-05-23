#ifndef NEWITEMWINDOW_H
#define NEWITEMWINDOW_H

#include <QDialog>
#include <QFileDialog>
#include <QDebug>
#include <QPixmap>
#include <QMessageBox>
#include <QStandardPaths>
#include <database.h>

namespace Ui {
class NewItemWindow;
}

class NewItemWindow : public QDialog
{
    Q_OBJECT

public:
    explicit NewItemWindow(QWidget *parent = 0, Database *parent2 = 0);
    ~NewItemWindow();

private slots:
    void on_selectImgButton_clicked();

    void on_addItemButton_clicked();

private:
    Ui::NewItemWindow *ui;
};

#endif // NEWITEMWINDOW_H
