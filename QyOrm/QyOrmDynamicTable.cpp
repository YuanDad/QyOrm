#include "QyOrmDynamicTable.h"
#include "QyOrmConfigure.h"
#ifdef QY_STD
#include "QyUuid.h"
#else
#include <QUuid>
#endif

QString QyOrmDynamicFieldInfo::sqlAddField()
{
    if(QyOrmConfigure::enabledBrace){
        return "ALTER TABLE \"" + fieldTableName + "\" ADD COLUMN \"" + fieldName + "\" " + getTypeString();
    }else{
        return "ALTER TABLE " + fieldTableName + " ADD COLUMN " + fieldName + ' ' + getTypeString();
    }
}

QString QyOrmDynamicFieldInfo::sqlDelField()
{
    if(QyOrmConfigure::enabledBrace){
        return "ALTER TABLE \"" + fieldTableName + "\" DROP COLUMN \"" + fieldName + '"';
    }else{
        return "ALTER TABLE " + fieldTableName + " DROP COLUMN " + fieldName;
    }
}

QyOrmFieldExtraInfo *QyOrmDynamicFieldInfo::createExtraInfo()
{
    QyOrmFieldExtraInfo* info = new QyOrmFieldExtraInfo;
    info->fieldName = fieldName;
    info->fieldLabel = fieldLabel;
    switch(fieldType){
    case 0: info->fieldType = QyOrmFieldType::String; break;
    case 1: info->fieldType = QyOrmFieldType::Integer; break;
    case 2: info->fieldType = QyOrmFieldType::Double; break;
    case 3: info->fieldType = QyOrmFieldType::DateTime; break;
    case 4: info->fieldType = QyOrmFieldType::Boolean; break;
    case 5: info->fieldType = QyOrmFieldType::Text; break;
    case 6: info->fieldType = QyOrmFieldType::Blob; break;
    }
    if(info->fieldType == QyOrmFieldType::String || info->fieldType == QyOrmFieldType::Text){
        info->isString = true;
    }else{
        info->isString = false;
    }
    if(fieldUnique){
        info->type = QyOrmFieldConstraint::Unique;
    }else{
        info->type = QyOrmFieldConstraint::Plain;
    }
    if(fieldNotnull){
        if(fieldUnique){
            info->type = QyOrmFieldConstraint::NotNullUnique;
        }else{
            info->type = QyOrmFieldConstraint::NotNull;
        }
    }
    return info;
}

void QyOrmDynamicFieldInfo::update(QyOrmFieldExtraInfo * info)
{
    info->fieldLabel = fieldLabel;
    switch(fieldType){
    case 0: info->fieldType = QyOrmFieldType::String; break;
    case 1: info->fieldType = QyOrmFieldType::Integer; break;
    case 2: info->fieldType = QyOrmFieldType::Double; break;
    case 3: info->fieldType = QyOrmFieldType::DateTime; break;
    case 4: info->fieldType = QyOrmFieldType::Boolean; break;
    case 5: info->fieldType = QyOrmFieldType::Text; break;
    case 6: info->fieldType = QyOrmFieldType::Blob; break;
    }
    if(info->fieldType == QyOrmFieldType::String || info->fieldType == QyOrmFieldType::Text){
        info->isString = true;
    }else{
        info->isString = false;
    }
    if(fieldUnique){
        info->type = QyOrmFieldConstraint::Unique;
    }else{
        info->type = QyOrmFieldConstraint::Plain;
    }
    if(fieldNotnull){
        if(fieldUnique){
            info->type = QyOrmFieldConstraint::NotNullUnique;
        }else{
            info->type = QyOrmFieldConstraint::NotNull;
        }
    }
}

void QyOrmDynamicFieldInfo::setDefaultFieldName()
{
#ifdef QY_STD
    fieldName = QyUuid::uuidSec(QyUuid::l16);
#else
    fieldName = QUuid::createUuid().toString();
#endif
}

QString QyOrmDynamicFieldInfo::getTypeString()
{
    switch(fieldType){
    case 0: return QyOrmTable::sqlCreateFieldType(QyOrmFieldType::String);
    case 1: return QyOrmTable::sqlCreateFieldType(QyOrmFieldType::Integer);
    case 2: return QyOrmTable::sqlCreateFieldType(QyOrmFieldType::Double);
    case 3: return QyOrmTable::sqlCreateFieldType(QyOrmFieldType::DateTime);
    case 4: return QyOrmTable::sqlCreateFieldType(QyOrmFieldType::Boolean);
    case 5: return QyOrmTable::sqlCreateFieldType(QyOrmFieldType::Text);
    case 6: return QyOrmTable::sqlCreateFieldType(QyOrmFieldType::Blob);
    default: return QyOrmTable::sqlCreateFieldType(QyOrmFieldType::String);
    }
}

QyOrmDynamicTableMaker::~QyOrmDynamicTableMaker()
{
    qDeleteAll(__privated->extraInfos);
}

bool QyOrmDynamicTableMaker::isEmpty()
{
    return __privated->extraInfos.isEmpty();
}

#include "QyOrmCompleteTable.h"
void QyOrmDynamicTableMaker::setFieldInfos(QList<QyOrmDynamicFieldInfo *> fieldInfos)
{
    qDeleteAll(__privated->extraInfos);
    __privated->extraInfos.clear();
    for(QyOrmDynamicFieldInfo* fieldInfo : fieldInfos){
        __privated->extraInfos.append(fieldInfo->createExtraInfo());
    }
}

void QyOrmDynamicTableMaker::appendFieldInfo(QyOrmDynamicFieldInfo *fieldInfo)
{
    __privated->extraInfos.append(fieldInfo->createExtraInfo());
}

void QyOrmDynamicTableMaker::completeDynamicTable(QyOrmDynamicTableTemplate * table)
{
    foreach(QyOrmFieldExtraInfo* extraInfo, __privated->extraInfos){
        QyOrmCompleteField temp(extraInfo);
        temp.make(table);
        temp.info = nullptr;
    }
}

QyOrmDynamicTableTemplate *QyOrmDynamicTableMaker::createDynamicTable()
{
    QyOrmDynamicTableTemplate* table = new QyOrmDynamicTableTemplate(__privated->dynamicTableName);
    completeDynamicTable(table);
    return table;
}
