QT += core gui widgets

CONFIG += c++11

TARGET = QyOrm
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS

include($$PWD/QyOrm/QyOrm.pri)

HEADERS += \
    test_qyorm.h

contains(CONFIG,SubMain):{
    message(SubMain)
}else{
    message(Main)
    SOURCES += Test_QyOrm_Main.cpp
}
