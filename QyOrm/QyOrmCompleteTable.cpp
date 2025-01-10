#include "QyOrmCompleteTable.h"
#include "QyOrmTable_p.h"
#include "QyOrmField.h"
#include "QyOrmField_p.h"
#include <QMetaProperty>

class QyOrmCompleteFieldPtr : public QObject
{
public:
    inline QyOrmCompleteFieldPtr(QyOrmTable *parent, QyOrmField* field): QObject(parent), field(field){}
    inline ~QyOrmCompleteFieldPtr() { if(field) delete field; }
    QyOrmField* field = nullptr;
};

QyOrmCompleteField::~QyOrmCompleteField()
{
    if(info) delete info;
}

void QyOrmCompleteField::make(QyOrmTable *table)
{
    QyOrmField* field = nullptr;
    switch (info->fieldType) {
    case QyOrmFieldType::String:
        field = new QyOrmStringField;
        break;
    case QyOrmFieldType::Integer:
        field = new QyOrmIntegerField;
        break;
    case QyOrmFieldType::Double:
        field = new QyOrmDoubleField;
        break;
    case QyOrmFieldType::DateTime:
        field = new QyOrmDateTimeField;
        break;
    case QyOrmFieldType::Boolean:
        field = new QyOrmBooleanField;
        break;
    case QyOrmFieldType::Text:
        field = new QyOrmTextField;
        break;
    case QyOrmFieldType::Blob:
        field = new QyOrmBlobField;
        break;
    }
    field->__privated->table = table;
    field->__privated->extraInfo = info;
    table->__privated->listField.append(field);
    if(table->__privated->extra->listFieldName.size() < table->__privated->listField.size()){
        table->__privated->extra->hashFieldIndex.insert(
                    info->fieldName, table->__privated->extra->listFieldName.size());
        table->__privated->extra->listFieldName.append(info->fieldName);
    }
    table->__privated->hashField[info->fieldName] = field;
    if(info->type == QyOrmFieldConstraint::PrimaryKey ||
            info->type == QyOrmFieldConstraint::AutoPrimaryKey){
        table->__privated->primaryKey = field;
    }
    new QyOrmCompleteFieldPtr(table, field);
}

void QyOrmCompleteField::__create(const QString &tableName, int index)
{
    QHash<QString, QyOrmField*>& hashTableNameHashFieldReady =
            QyOrmTablePrivate::hashTableNameHashFieldReady[tableName];
    QyOrmTableExtraInfo* extra =
            QyOrmTablePrivate::hashTableNameHashTableExtraInfoReady[tableName];

    hashTableNameHashFieldReady.insert(info->fieldName, nullptr);
    if(index == -1){
        extra->hashFieldIndex.insert(info->fieldName, extra->listFieldName.size());
        extra->listFieldName.append(info->fieldName);
    }else{
        extra->listFieldName.insert(index, info->fieldName);
        extra->hashFieldIndex.insert(info->fieldName, index);
        for(int i = extra->listFieldName.size() - 1; i > index; --i){
            extra->hashFieldIndex[extra->listFieldName.at(i)] = i;
        }
    }
}

void QyOrmCompleteField::__destroy(const QString &tableName, int index)
{
    QHash<QString, QyOrmField*>& hashTableNameHashFieldReady =
            QyOrmTablePrivate::hashTableNameHashFieldReady[tableName];
    QyOrmTableExtraInfo* extra =
            QyOrmTablePrivate::hashTableNameHashTableExtraInfoReady[tableName];

    hashTableNameHashFieldReady.remove(info->fieldName);
    if(index == -1){
        index = extra->hashFieldIndex.value(info->fieldName);
    }
    extra->listFieldName.removeAt(index);
    extra->hashFieldIndex.remove(info->fieldName);
    for(int i = index; i < extra->listFieldName.size(); ++i){
        extra->hashFieldIndex[extra->listFieldName.at(i)] = i;
    }
}
