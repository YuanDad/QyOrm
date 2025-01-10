#include "QyOrmSelectGenerator.h"
#include "QyOrmConfigure.h"
#include "QyOrmTable.h"
#include "QyOrmDatabase.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>
#include "QyOrmTable_p.h"
#include <QDebug>

class QyOrmDatabaseSelectGeneratorPrivate
{
public:
    QyOrmDatabaseSelectGeneratorPrivate():
        query(QyOrmDatabase::getValidDatabase()){}
    QVariantList fieldNames;
    int columnCount;
    QString error;
    QString sql;
    QSqlQuery query;
};

QyOrmDatabaseSelectGenerator::QyOrmDatabaseSelectGenerator(const QString& sql)
{
    __privated = new QyOrmDatabaseSelectGeneratorPrivate;
    __privated->sql = sql;

    __privated->query.setForwardOnly(true);
    if(!__privated->query.exec(sql)){
        __privated->error = __privated->query.lastError().text();
        return;
    }
    QSqlRecord record = __privated->query.record();
    __privated->columnCount = record.count();
    for(int i = 0; i < __privated->columnCount; ++i){
        __privated->fieldNames.append(record.fieldName(i));
    }
}

QyOrmDatabaseSelectGenerator::~QyOrmDatabaseSelectGenerator()
{
    delete __privated;
}

const QVariantList &QyOrmDatabaseSelectGenerator::fieldNames()
{
    return __privated->fieldNames;
}

int QyOrmDatabaseSelectGenerator::columnCount() const
{
    return __privated->columnCount;
}

const QString& QyOrmDatabaseSelectGenerator::error() const
{
    return __privated->error;
}

const QString& QyOrmDatabaseSelectGenerator::sql() const
{
    return __privated->sql;
}

QVariantList QyOrmDatabaseSelectGenerator::yield() const
{
    const QVariant Null;
    QVariantList row;
    if(__privated->query.next()){
        for(int index = 0; index < __privated->columnCount; ++index) {
            QVariant data = __privated->query.value(index);
            row.append(data.isNull() ? Null : data);
        }
    }
    return row;
}

const QList<QVariantList>& QyOrmDatabaseSelectGenerator::yields(int count) const
{
    QList<QVariantList> result;
    for(int row = 0; row < count && __privated->query.next(); ++row){
        QVariantList datas;
        for(int index = 0; index < __privated->columnCount; ++index) {
            QVariant data = __privated->query.value(index);
            datas.append(data.isNull() ? QVariant() : data);
        }
        result.append(datas);
    }
    return std::move(result);
}

const QList<QVariantList> &QyOrmDatabaseSelectGenerator::yieldAll() const
{
    QList<QVariantList> result;
    while(__privated->query.next()){
        QVariantList datas;
        for(int index = 0; index < __privated->columnCount; ++index) {
            QVariant data = __privated->query.value(index);
            datas.append(data.isNull() ? QVariant() : data);
        }
        result.append(datas);
    }
    return std::move(result);
}

class QyOrmSelectGeneratorPrivate
{
public:
    ~QyOrmSelectGeneratorPrivate(){
        delete generator;
        if(autoDelete) delete selectObejct;
    }
    QyOrmDatabaseSelectGenerator* generator;
    QyOrmTable *selectObejct;
    bool disorder;
    bool autoDelete;
    char __padding[6];
    QList<int> columnIndexs;
    int rowCount = -1;
};

QyOrmSelectGenerator::QyOrmSelectGenerator(QyOrmTable &selectObejct, const QString &sql, bool disorder):
    QyOrmSelectGenerator(selectObejct.constructor(selectObejct.parent()), true, sql, disorder){}
QyOrmSelectGenerator::QyOrmSelectGenerator(QyOrmTable *selectObejct, bool autoDelete,
                                           const QString &sql, bool disorder)
{
    __privated = new QyOrmSelectGeneratorPrivate;
    __privated->selectObejct = selectObejct;
    __privated->autoDelete = autoDelete;
    if(sql.isEmpty()){
        __privated->generator = new QyOrmDatabaseSelectGenerator(
                    selectObejct->sqlSelectAllFieldString() +
                    selectObejct->sqlSelectWhere() + selectObejct->sqlSelectOrderBy());
        __privated->disorder = false;
    }else{
        __privated->generator = new QyOrmDatabaseSelectGenerator(sql);
        __privated->disorder = disorder;
    }

    if(__privated->disorder){
        for(int column = 0; column < __privated->generator->columnCount(); ++column){
            QString fieldName = __privated->generator->__privated->fieldNames.at(column).toString();
            int index = selectObejct->__privated->extra->hashFieldIndex.value(fieldName.toLower(), -1);
            __privated->columnIndexs.append(index);
        }
    }
}

QyOrmSelectGenerator::~QyOrmSelectGenerator()
{
    delete __privated;
}

int QyOrmSelectGenerator::rowCount() const
{
    if(__privated->rowCount < 0){
        __privated->rowCount = __privated->selectObejct->sqlSelectCount();
    }
    return __privated->rowCount;
}

bool QyOrmSelectGenerator::__yield(const QVariantList &rowData, QyOrmTable *table) const
{
    table->__privated->isAssigning = true;
    for(int column = 0; column < __privated->generator->columnCount(); ++column){
        int index = __privated->disorder ? __privated->columnIndexs.at(column) : column;
        if(index < 0) continue;
        const QVariant& data = rowData.at(column);
        QyOrmField* field = table->__privated->listField.at(index);
        if(data.isValid()){
            field->query(data);
        }else{
            field->queryDefaultValue();
        }
    }
    table->__privated->isAssigning = false;
    return table;
}

QyOrmTable *QyOrmSelectGenerator::makeTable() const
{
    return __privated->selectObejct->constructor(__privated->selectObejct->parent());
}

QyOrmTable *QyOrmSelectGenerator::__yield(const QVariantList &rowData) const
{
    QyOrmTable* table = makeTable();
    __yield(rowData, table);
    return table;
}

QyOrmTable *QyOrmSelectGenerator::yield() const
{
    const QVariantList& rowData = __privated->generator->yield();
    return rowData.isEmpty() ? nullptr : __yield(rowData);
}

bool QyOrmSelectGenerator::yield(QyOrmTable * table) const
{
    const QVariantList& rowData = __privated->generator->yield();
    return rowData.isEmpty() ? false : __yield(rowData, table);
}

QList<QyOrmTable *> QyOrmSelectGenerator::yields(int count) const
{
    QList<QyOrmTable *> result;
    const QList<QVariantList>& rowDatas = __privated->generator->yields(count);
    foreach(const QVariantList& rowData, rowDatas){
        result.append(__yield(rowData));
    }
    return result;
}

QList<QyOrmTable *> QyOrmSelectGenerator::yieldAll() const
{
    QList<QyOrmTable *> result;
    const QList<QVariantList>& rowDatas = __privated->generator->yieldAll();
    foreach(const QVariantList& rowData, rowDatas){
        result.append(__yield(rowData));
    }
    return result;
}

const QyOrmDatabaseSelectGenerator *QyOrmSelectGenerator::generator()
{
    return __privated->generator;
}

const QyOrmTable *QyOrmSelectGenerator::selectObejct()
{
    return __privated->selectObejct;
}
