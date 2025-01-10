#ifndef QYORMCOMPLETETABLE_H
#define QYORMCOMPLETETABLE_H

#include "QyOrmFieldExtraInfo.h"
#include "QyOrmTable.h"

class QyOrmCompleteField
{
public:
    explicit inline QyOrmCompleteField(QyOrmFieldExtraInfo* info): info(info){}
    virtual ~QyOrmCompleteField();
    virtual void make(QyOrmTable* table);
    void __create(const QString& tableName, int index = -1);
    void __destroy(const QString& tableName, int index = -1);
public:
    QyOrmFieldExtraInfo* info = nullptr;
};

template<class TableName> class QyOrmCompleteTable
{
public:
    inline QyOrmCompleteTable(){tableName = TableName().tableName();}
    inline ~QyOrmCompleteTable(){qDeleteAll(fields);}
    virtual TableName* create(){
        TableName* table = new TableName;
        for(QyOrmCompleteField* field : fields){
            field->make(table);
        }
    }
    inline void append(QyOrmCompleteField* field)
    {fields.append(field); field->__create(tableName);}
    inline void remove(QyOrmCompleteField* field)
    {int index = fields.indexOf(field); field->__destroy(tableName, index); delete fields.takeAt(index);}
    inline void insert(int index, QyOrmCompleteField* field)
    {fields.insert(index, field); field->__create(tableName, index); }
    inline void remove(int index)
    {fields.at(index)->__destroy(tableName, index); delete fields.takeAt(index);}

    inline void append(QyOrmFieldExtraInfo* info)
    {QyOrmCompleteField* field = new QyOrmCompleteField(info); fields.append(field); field->__create(tableName);}
    inline QyOrmCompleteField* take(QyOrmFieldExtraInfo* info)
    {int index = indexOf(info); if(index < 0) return nullptr; return fields.takeAt(index);}
    inline void remove(QyOrmFieldExtraInfo* info)
    {int index = indexOf(info); if(index < 0)return; fields.at(index)->__destroy(tableName, index); delete fields.takeAt(index);}
    inline void insert(int index, QyOrmFieldExtraInfo* info)
    {QyOrmCompleteField* field = new QyOrmCompleteField(info); fields.insert(index, field); field->__create(tableName, index); }
    QyOrmFieldExtraInfo* find(const QString& fieldName);
    QyOrmFieldExtraInfo* find(int index);
    int indexOf(QyOrmFieldExtraInfo* info);
protected:
    QList<QyOrmCompleteField*> fields;
    QString tableName;
};

template<class TableName>
QyOrmFieldExtraInfo *QyOrmCompleteTable<TableName>::find(const QString &fieldName)
{
    for(QyOrmCompleteField* field : fields){
        if(field->info->fieldName == fieldName){
            return field->info;
        }
    }
    return nullptr;
}

template<class TableName>
QyOrmFieldExtraInfo *QyOrmCompleteTable<TableName>::find(int index)
{
    if(index >= 0 && index < fields.size()){
        return fields.at(index)->info;
    }
    return nullptr;
}

template<class TableName>
int QyOrmCompleteTable<TableName>::indexOf(QyOrmFieldExtraInfo *info)
{
    for(int index = 0; index < fields.size(); ++index){
        if(fields.at(index)->info == info){
            return index;
        }
    }
    return -1;
}
typedef QyOrmCompleteTable<QyOrmTable> QyOrmCompleteBaseTable;

#endif // QYORMCOMPLETETABLE_H
