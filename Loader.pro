#-------------------------------------------------
#
# Project created by QtCreator 2016-10-10T14:06:18
#
#-------------------------------------------------

QT       += core gui sql

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Loader
TEMPLATE = app


SOURCES += main.cpp\
        mainwindow.cpp

HEADERS  += mainwindow.h
FORMS    += mainwindow.ui


INCLUDEPATH += "C:\Program Files\MySQL\MySQL Server 5.7\include"
LIBS += "C:\Program Files\MySQL\MySQL Server 5.7\lib\libmysql.lib"
