#ifndef QYORMFIELD_P_H
#define QYORMFIELD_P_H

#include <QString>
#include <QVariant>
#include <QList>
#include "QyOrmFieldDefine.h"
#include "QyOrmFieldExtraInfo.h"

class QyOrmDateTimeFieldPrivate{
public:
    bool isLike = false;
    QString likeString;
    QString likeFormat;
};

class QyOrmTable;
class QyOrmFieldPrivate
{
public:
    QStringList executers;
    bool isAndExecuter = true;

public:
    QyOrmTable* table = nullptr;
    QString operatorString = "=";
    bool isNull = true;
    bool isScanChanged = false;
    bool isDesc = false;
    int orderToCastNumber = 0;
    bool isPassSelect = false;
    QString replaceSource;//记录replace的两个参数
    QString replaceTarget;

    QString joinTableName, joinReplaceName, joinOnFieldName;//join on

public:
    QyOrmFieldExtraInfo* extraInfo = nullptr;
    inline QString fieldName(){return (extraInfo ? extraInfo->fieldName : QString());}
    inline QString fieldLabel(){return (extraInfo ? extraInfo->fieldLabel : QString());}
    inline QyOrmFieldType fieldType(){return (extraInfo ? extraInfo->fieldType : QyOrmFieldType::String);}
    inline bool autoPrimaryKey(){return (extraInfo && extraInfo->type == QyOrmFieldConstraint::AutoPrimaryKey);}
    inline bool primaryKey(){return (extraInfo && extraInfo->type == QyOrmFieldConstraint::PrimaryKey);}
    inline bool unique(){return (extraInfo && (extraInfo->type == QyOrmFieldConstraint::Unique ||
                                                 extraInfo->type == QyOrmFieldConstraint::NotNullUnique));}
    inline bool notNull(){return (extraInfo && (extraInfo->type == QyOrmFieldConstraint::NotNull ||
                                   extraInfo->type == QyOrmFieldConstraint::NotNullUnique));}
    inline bool isString(){return (extraInfo && extraInfo->isString);}

public:
    inline ~QyOrmFieldPrivate(){
        if(dateTimePrivated) delete dateTimePrivated;
    }
    QyOrmDateTimeFieldPrivate* dateTimePrivated = nullptr;
};

#endif // QYORMFIELD_P_H
