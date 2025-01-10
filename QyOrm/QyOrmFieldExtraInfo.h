#ifndef QYORMFIELDEXTRAINFO_H
#define QYORMFIELDEXTRAINFO_H

#include <QString>
#include <QMetaType>
#include <QVariant>
#include "QyOrmFieldDefine.h"

class QyOrmFieldExtraInfo{
public:
    inline QyOrmFieldExtraInfo(){}
    explicit inline QyOrmFieldExtraInfo(QString s): fieldTypeString(s){}

    inline QyOrmFieldExtraInfo(QString s, QyOrmFieldConstraint v1):
        fieldTypeString(s), type(v1){}
    inline QyOrmFieldExtraInfo(QString s, QString v1):
        fieldTypeString(s), fieldLabel(v1){}
    inline QyOrmFieldExtraInfo(QString s, QyOrmFieldConstraint v1, QString v2):
        fieldTypeString(s), type(v1), fieldLabel(v2){}
    inline QyOrmFieldExtraInfo(QString s, QString v1, QyOrmFieldConstraint v2):
        fieldTypeString(s), type(v2), fieldLabel(v1){}

    inline explicit QyOrmFieldExtraInfo(const QyOrmFieldExtraInfo&) = default;
    inline explicit QyOrmFieldExtraInfo(QyOrmFieldExtraInfo&&) = default;
public:
    QString fieldTypeString;//这个属性不重要,只用于生成fieldType
    QyOrmFieldConstraint type = QyOrmFieldConstraint::Plain;//*
    QString fieldLabel;
    QString fieldName;//*
    QyOrmFieldType fieldType;//*
    bool isString = false;
    bool isPassSelect = false;
    QVariant defaultValue;
};
Q_DECLARE_METATYPE(QyOrmFieldExtraInfo*);


#endif // QYORMFIELDEXTRAINFO_H
