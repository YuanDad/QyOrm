QT += core gui sql widgets
CONFIG += c++11

TARGET = QyOrm
TEMPLATE = app

include($$PWD/QyOrm/QyOrm.pri)
SOURCES += TestMain.cpp \
    TestMainWindow.cpp
HEADERS += TestModel.h \
    TestMainWindow.h

HEADERS += QyQtBasic.h \
           QyUuid.h

FORMS += \
    TestMainWindow.ui
