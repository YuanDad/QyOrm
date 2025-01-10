if(!contains(DEFINES, QY_ORM)){

DEFINES += QY_ORM
INCLUDEPATH += $$PWD

if(contains(QT, widgets)){
DEFINES += QY_ORM_WIDGETS
HEADERS += \
    $$PWD/QyOrmComboBoxDataMap.h \
    $$PWD/QyOrmFormWidget.h \
    $$PWD/QyOrmDateTimeEdit.h \
    $$PWD/QyOrmFormModel.h \
    $$PWD/QyOrmExtra.h \
    $$PWD/QyOrmPurviewTreeWidget.h
SOURCES += \
    $$PWD/QyOrmComboBoxDataMap.cpp \
    $$PWD/QyOrmFormWidget.cpp \
    $$PWD/QyOrmDateTimeEdit.cpp \
    $$PWD/QyOrmFormModel.cpp \
    $$PWD/QyOrmExtra.cpp \
    $$PWD/QyOrmPurviewTreeWidget.cpp
DISTFILES += \
    $$PWD/QyOrmFormModel
}

contains(QT, sql):{
DEFINES += QY_ORM_DATABASE
HEADERS += \
    $$PWD/QyOrmDatabase.h
SOURCES += \
    $$PWD/QyOrmDatabase.cpp
DISTFILES += \
    $$PWD/QyOrmDatabase
}

HEADERS += \
    $$PWD/QyOrmCompleteTable.h \
    $$PWD/QyOrmDynamicTable.h \
    $$PWD/QyOrmFieldDefine.h \
    $$PWD/QyOrmTableDefine.h \
    $$PWD/QyOrmField.h \
    $$PWD/QyOrmField_p.h \
    $$PWD/QyOrmTable.h \
    $$PWD/QyOrmFieldExtraInfo.h \
    $$PWD/QyOrmTable_p.h \
    $$PWD/QyOrmRemoteDatabase.h \
    $$PWD/QyOrmTableExtraInfo.h \
    $$PWD/QyOrmSelectGenerator.h \
    $$PWD/QyOrmConfigure.h

SOURCES += \
    $$PWD/QyOrmCompleteTable.cpp \
    $$PWD/QyOrmDynamicTable.cpp \
    $$PWD/QyOrmField.cpp  \
    $$PWD/QyOrmTable.cpp \
    $$PWD/QyOrmRemoteDatabase.cpp \
    $$PWD/QyOrmSelectGenerator.cpp

}
