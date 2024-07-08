TEMPLATE = app
TARGET = slideshow
CONFIG += console # for debugging: cout
QT += core gui \
      multimedia
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

OBJECTS_DIR = build
MOC_DIR = build
UI_DIR = build

 HEADERS =src/mainwindow.h

 SOURCES =src/main.cpp \
     src/mainwindow.cpp

FORMS = src/forms/mainwindow.ui

RC_FILE = src/slideshow.rc

RESOURCES += \
    src/slideshow.qrc

