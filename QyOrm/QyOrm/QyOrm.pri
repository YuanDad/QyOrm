#if(true){
if(false){
CONFIG += SubMain

#SOURCES += $$PWD/AutoGenerate.cpp   #自动连接数据库生成model代码
#SOURCES += $$PWD/TestMain.cpp
#HEADERS +=$$PWD/QyOrm.h

SOURCES += $$PWD/AutoGenerateSeparate.cpp   #自动连接数据库生成model代码,每个表分开
}

#if(true){
if(false){
QT += gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets
}

contains(QT, gui):{
HEADERS += \
    $$PWD/FormModel.h \
    $$PWD/FormWidget.h

SOURCES += \
    $$PWD/FormModel.cpp \
    $$PWD/FormWidget.cpp
}

QT += sql

HEADERS += \
    $$PWD/Table.h \
    $$PWD/Field.h

SOURCES += \
    $$PWD/Table.cpp \
    $$PWD/Field.cpp

#Field的 operator= 和 const&构造 是为了给moc记录来给Table的initialize函数用的
