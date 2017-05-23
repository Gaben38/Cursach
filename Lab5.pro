#-------------------------------------------------
#
# Project created by QtCreator 2017-04-22T16:59:17
#
#-------------------------------------------------

QT       += core gui
QT       += sql
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
win32: RC_ICONS = ikeaicon.ico

TARGET = CMV_Cursach
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0


SOURCES += main.cpp\
        mainwindow.cpp \
    database.cpp \
    registerwindow.cpp \
    passremindwindow.cpp \
    newitemwindow.cpp

HEADERS  += mainwindow.h \
    database.h \
    registerwindow.h \
    passremindwindow.h \
    newitemwindow.h

FORMS    += mainwindow.ui \
    database.ui \
    registerwindow.ui \
    passremindwindow.ui \
    newitemwindow.ui
