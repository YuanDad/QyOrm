#include "QyOrmTable.h"
#include "QyOrmTable_p.h"
#include "QyOrmField.h"
#include "QyOrmField_p.h"

#include <QMetaProperty>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QPointer>
#include <QDebug>

#include "QyOrmConfigure.h"
bool QyOrmConfigure::enabledBrace = false;
bool QyOrmConfigure::enabledBelong = false;
bool QyOrmConfigure::enabledRemoteDatabase = false;

static thread_local bool callNew = false;

QyOrmTable::QyOrmTable(QObject *parent) : QObject(parent)
{
}

QyOrmTable *QyOrmTable::copy()
{
    QyOrmTable* table = constructor();
    if(table) {
        for(QyOrmField* field : listField()){
            QyOrmField* copyField = table->field(field->fieldName());
            switch (field->fieldType()) {
            case QyOrmFieldType::String: {
                copyField->toStringField()->__data = field->toStringField()->__data;
            } break;
            case QyOrmFieldType::Integer: {
                copyField->toIntegerField()->__data = field->toIntegerField()->__data;
            } break;
            case QyOrmFieldType::Double: {
                copyField->toDoubleField()->__data = field->toDoubleField()->__data;
            } break;
            case QyOrmFieldType::DateTime: {
                copyField->toDateTimeField()->__data = field->toDateTimeField()->__data;
            } break;
            case QyOrmFieldType::Boolean: {
                copyField->toBooleanField()->__data = field->toBooleanField()->__data;
            } break;
            case QyOrmFieldType::Text: {
                copyField->toTextField()->__data = field->toTextField()->__data;
            } break;
            case QyOrmFieldType::Blob: {
                copyField->toBlobField()->__data = field->toBlobField()->__data;
            } break;
            }
        }
    }
    return table;
}

void *QyOrmTable::operator new(size_t size)
{
    callNew = true;
    return ::operator new(size);
}

bool QyOrmTable::atHeap()
{
    return __privated->atHeap;
}

QyOrmTable::~QyOrmTable()
{
    delete __privated;
}

QyOrmTable *QyOrmTable::operator=(const QyOrmTable *other)
{
    if(__privated->extra->tableName == other->__privated->extra->tableName){
        foreach(QyOrmField* field, __privated->listField){
            field->copy(other->__privated->hashField.value(field->__privated->fieldName()));
        }
    }
    return this;
}

QString QyOrmTable::tableName()
{
    return __privated->extra->tableName;
}

QString QyOrmTable::tableLabel()
{
    return __privated->extra->tableLabel;
}

QyOrmField *QyOrmTable::primaryKey()
{
    return __privated->primaryKey;
}

const QStringList &QyOrmTable::listFieldName()
{
    return __privated->extra->listFieldName;
}

const QList<QyOrmField *> &QyOrmTable::listField()
{
    return __privated->listField;
}

const QList<QyOrmField *> &QyOrmTable::listDynamicField()
{
    return __privated->listDynamicField;
}

QyOrmField *QyOrmTable::field(const QString &fieldName)
{
    if(fieldName.isEmpty()) return nullptr;
    return __privated->hashField.value(fieldName, nullptr);
}

QyOrmField *QyOrmTable::at(int index)
{
    if(index >= 0 && index < __privated->listField.size()) return __privated->listField.at(index);
    return nullptr;
}

void QyOrmTable::assigning()
{
    __privated->isAssigning = true;
}

void QyOrmTable::assigned()
{
    __privated->isAssigning = false;
}

QList<QyOrmField *> QyOrmTable::recordChangedFields()
{
    return __privated->recordChangedFields.values();
}

QList<QyOrmField *> QyOrmTable::scanChangedFields()
{
    QList<QyOrmField *> result;
    foreach(QyOrmField* field, __privated->listField){
        if(field->isScanChanged()){
            result.append(field);
        }
    }
    return result;
}

bool QyOrmTable::hasScanChanged()
{
    foreach(QyOrmField* field, __privated->listField){
        if(field->isScanChanged()){
            return true;
        }
    }
    return false;
}

bool QyOrmTable::isRecordChanged(const QString &fieldName)
{
    foreach(QyOrmField* field, __privated->recordChangedFields){
        if(fieldName == field->__privated->extraInfo->fieldName) return true;
    }
    return false;
}

QVariant QyOrmTable::getFieldValue(const QString &fieldName)
{
    QyOrmField * field = __privated->hashField.value(fieldName, nullptr);
    if(field) return field->answer();
    return QVariant();
}

QyOrmTable &QyOrmTable::setFieldValue(const QString &fieldName, const QVariant &value)
{
    QyOrmField * field = __privated->hashField.value(fieldName, nullptr);
    if(field) field->query(value);
    return *this;
}

QVariant QyOrmTable::getFieldValue(int index)
{
    if(index >= 0 && index < __privated->listField.size()) return __privated->listField.at(index)->answer();
    return QVariant();
}

QyOrmTable &QyOrmTable::setFieldValue(int index, const QVariant &value)
{
    if(index >= 0 && index < __privated->listField.size()) __privated->listField.at(index)->query(value);
    return *this;
}

void QyOrmTable::clearRecordChanged(QyOrmField *field)
{
    if(field) __privated->recordChangedFields.remove(field);
    else __privated->recordChangedFields.clear();
}

void QyOrmTable::clearScanChanged()
{
    foreach(QyOrmField* field, __privated->listField){
        field->__privated->isScanChanged = false;
    }
}

QString QyOrmTable::toJson(bool isArray, const QString &fieldName, const QStringList &excludes, int writeNull) const
{
    QString result = (isArray ? "[" : "{");
    if(!fieldName.isEmpty()){
        QyOrmField* field = __privated->hashField.value(fieldName, nullptr);
        if(!field) {
            int findex = __privated->extra->hashFieldIndex.value(fieldName.toLower(), -1);
            if(findex != -1){
                field = __privated->listField.at(findex);
            }
        }
        if(!field) return (isArray ? "[]" : "{}");
        return (isArray ? '[' + field->toJson() +']' : '{' + field->toJson() +'}');
    }
    if(writeNull == -1) writeNull = isArray;
    foreach(QyOrmField* field, __privated->listField){
        if(!writeNull && field->isNull()) continue;
        if(excludes.size() && excludes.contains(field->fieldName())) continue;
        result += (isArray ? field->toJson() : QString("\"%1\":%2").arg(field->fieldName(), field->toJson())) + ',';
    }
    result[result.size() - 1] = (isArray ? ']' : '}');
    return result;
}

QString QyOrmTable::toJson(const QList<QyOrmTable *> &tables, bool isArray, bool autoRemove)
{
    if(tables.isEmpty()) return "[]";
    QString result;
    foreach(const QyOrmTable* table, tables){
        result.append(table->toJson(isArray));
        result.append(',');
    }
    if(autoRemove) qDeleteAll(tables);
    result.chop(1);
    return '[' + result + ']';
}

void QyOrmTable::fromJson(const QByteArray &jsonBytes)
{
    if(!jsonBytes.isEmpty()){
        QJsonDocument document = QJsonDocument::fromJson(jsonBytes);
        if(document.isObject()){
            return fromQJson(document.object());
        }else if(document.isArray()){
            return fromQJson(document.array());
        }
    }
}

void QyOrmTable::fromQJson(const QJsonObject &object)
{
    __privated->isAssigning = true;
    foreach(const QString& fieldName, object.keys()){
        fromVariant(fieldName, object.value(fieldName).toVariant());
    }
    __privated->isAssigning = false;
}

void QyOrmTable::fromQJson(const QJsonArray &array)
{
    __privated->isAssigning = true;
    for(int index = 0; index < __privated->listField.size(); ++index){
        __privated->listField.at(index)->query(array.at(index).toVariant());
    }
    __privated->isAssigning = false;
}

void QyOrmTable::fromQJson(const QJsonArray &headers, const QJsonArray &array)
{
    __privated->isAssigning = true;
    for(int index = 0; index < headers.size(); ++index){
        fromVariant(headers.at(index).toString(), array.at(index).toVariant());
    }
    __privated->isAssigning = false;
}

void QyOrmTable::fromQJson(const QStringList &headers, const QJsonArray &array)
{
    __privated->isAssigning = true;
    for(int index = 0; index < headers.size(); ++index){
        fromVariant(headers.at(index), array.at(index).toVariant());
    }
    __privated->isAssigning = false;
}

bool QyOrmTable::fromVariant(const QString &fieldName, const QVariant &data)
{
    QyOrmField* field = __privated->hashField.value(fieldName, nullptr);
    if(!field) {
        int findex = __privated->extra->hashFieldIndex.value(fieldName.toLower(), -1);
        if(findex != -1) field = __privated->listField.at(findex);
    }
    if(field){
        field->query(data);
        return true;
    }else{
        return false;
    }
}

#include "QyOrmDatabase.h"
QString QyOrmTable::sqlCreateFieldType(QyOrmFieldType type)
{
    switch (type) {
        case QyOrmFieldType::String: return "VARCHAR";
        case QyOrmFieldType::Integer: return "INTEGER";
        case QyOrmFieldType::Double: return "REAL";
        case QyOrmFieldType::DateTime: {
#ifdef QY_ORM_DATABASE
            if(QyOrmDatabase::isDriverPgsql()){
                return "TIMESTAMP";
            }
#endif
            return QyOrmDateTimeField::__create_type_name;
        }
        case QyOrmFieldType::Boolean: return "BOOLEAN";
        case QyOrmFieldType::Text: return "TEXT";
        case QyOrmFieldType::Blob: return "BYTEA";
    }
    return "VARCHAR";
}

QString QyOrmTable::sqlCreateTableString(const QString &other, bool enableConstraint)
{
    QString sql = "CREATE TABLE " + __privated->extra->tableName + '(';
    foreach(QyOrmField* field, __privated->listField){
        QString fieldSql = field->__privated->fieldName() + ' ' +
                sqlCreateFieldType(field->__privated->fieldType());
        if(enableConstraint){
            if(field->__privated->unique()){
                fieldSql += " UNIQUE";
            }
            if(field->__privated->notNull()){
                fieldSql += " NOT NULL";
            }
        }
        sql += fieldSql + ',';
    }
    sql += "PRIMARY KEY(" + __privated->primaryKey->fieldName() + ")";
    if(!other.isEmpty()) sql += ',' + other;
    return sql + ");";
}

QString QyOrmTable::sqlDropTableString()
{
    return "DROP TABLE " + __privated->extra->tableName;
}

void QyOrmTable::addOrderBy(QyOrmField &field, bool desc, int castNumber)
{
    __privated->orderByFields.append(&field);
    field.__privated->isDesc = desc;
    field.__privated->orderToCastNumber = castNumber;
}

QyOrmTable &QyOrmTable::addOrderBy(const QString &fieldName, bool desc, int castNumber)
{
    QyOrmField* field = __privated->hashField.value(fieldName, nullptr);
    if(field) addOrderBy(*field, desc, castNumber);
    return *this;
}

bool QyOrmTable::hasOrderBy()
{
    return __privated->orderByFields.size();
}

void QyOrmTable::invertOrderBy()
{
    foreach(QyOrmField* field, __privated->orderByFields){
        field->__privated->isDesc = !field->__privated->isDesc;
    }
}

void QyOrmTable::copyOrderBy(QyOrmTable *source)
{
    foreach(QyOrmField* field, __privated->orderByFields){
        field->query(source->getFieldValue(field->__privated->extraInfo->fieldName));
    }
}

void QyOrmTable::setLimit(int limit, int offset)
{
    __privated->limit = limit;
    __privated->offset = offset;
}

bool QyOrmTable::appendSelectField(const QString &fieldName)
{
    QyOrmField* data = field(fieldName);
    if(data) __privated->singleSelectFields.append(data);
    else return false; return true;
}

void QyOrmTable::appendSelectField(QyOrmField *field)
{
    __privated->singleSelectFields.append(field);
}

QString QyOrmTable::sqlSelectSimpleString() const
{
    return "SELECT * FROM " + __privated->extra->tableName;
}

QString QyOrmTable::_sqlSelectJoinOnString() const
{
    QString sql;
    foreach(QyOrmField* field, __privated->joinOnFields){
        sql += " JOIN " + field->__privated->joinTableName + " ON " + __privated->extra->tableName +
            '.' + field->__privated->extraInfo->fieldName + '=' + field->__privated->joinTableName +
                '.' + (field->__privated->joinOnFieldName.isNull() ?
                 field->__privated->extraInfo->fieldName : field->__privated->joinOnFieldName);
    }
    return sql;
}

QString QyOrmTable::sqlSelectAllFieldString() const
{
    if(__privated->listField.isEmpty()) return QString();
    bool belong = __privated->joinOnFields.size();
    QString sql = "SELECT ";
    QList<QyOrmField*>& listField = (__privated->singleSelectFields.size() ?
                                         __privated->singleSelectFields : __privated->listField);
    for(int index = 0; index < listField.size(); ++index){
        QyOrmField* field = listField.at(index);
        if(field->__privated->isPassSelect){
            sql += "NULL,";
        }else{
            if(belong){
                if(field->__privated->joinTableName.isNull() ||
                        field->__privated->joinReplaceName.isNull()){
                    sql += __privated->extra->tableName + '.' + field->sqlFieldString();
                }else{
                    sql += field->__privated->joinTableName + '.' + field->__privated->joinReplaceName +
                            " AS " + (field->__privated->extraInfo->fieldName);
                }
            }else{
                sql += field->sqlFieldString();
            }
            sql += ',';
        }
    }
    sql[sql.size() - 1] = ' ';
    sql += "FROM " + __privated->extra->tableName;
    if(belong){
        sql += _sqlSelectJoinOnString();
    }
    return sql;
}

QString QyOrmTable::sqlSelectCountString() const
{
    QString sql = "SELECT COUNT(";
    sql += __privated->extra->tableName + '.' + __privated->primaryKey->fieldName();
    sql += ") FROM " + __privated->extra->tableName;
    if(__privated->joinOnFields.size()){
        sql += _sqlSelectJoinOnString();
    }
    sql += sqlSelectWhere();
    return sql;
}

QString QyOrmTable::sqlSelectSelfByFieldString(QyOrmField *field) const
{
    return sqlSelectAllFieldString() + " WHERE " + field->sqlExecuteString();
}

QString QyOrmTable::sqlSelectSelfByFieldsString(QList<QyOrmField *> fields) const
{
    QStringList wheres;
    foreach(QyOrmField* field, fields){
        wheres.append(field->sqlExecuteString());
    }
    return sqlSelectAllFieldString() + " WHERE " + wheres.join(" AND ");
}

QString QyOrmTable::sqlSelectSelfString() const
{
    return sqlSelectAllFieldString() + " WHERE " + __privated->primaryKey->sqlExecuteString();
}

QString QyOrmTable::sqlSelectFieldString(const QString &fieldName) const
{
    return "SELECT " + fieldName + " FROM " + __privated->extra->tableName;
}

void QyOrmTable::setSeniorSelectSql(const QString &seniorSelectSql)
{
    __privated->seniorSelectSql = seniorSelectSql;
}

void QyOrmTable::appendSeniorSelectSql(const QString &seniorSelectSql)
{
    if(__privated->seniorSelectSql.isEmpty()){
        __privated->seniorSelectSql = seniorSelectSql;
    }else{
        __privated->seniorSelectSql += " AND " + seniorSelectSql;
    }
}

void QyOrmTable::setForceSelectSql(const QString &forceSelectSql)
{
    __privated->forceSelectSql = forceSelectSql;
}

QString QyOrmTable::sqlSelectWhere(const QString &customWhere) const
{
    bool belong = __privated->joinOnFields.size();
    QString where = __privated->seniorSelectSql;
    foreach(QyOrmField* field, __privated->recordChangedFields){
        if(!where.isEmpty()) where += " AND ";
        where += field->sqlExecuteString(belong);
    }
    if(!where.isEmpty()){
        where = " WHERE " + where;
    }
    if(!customWhere.isEmpty()){
        if(where.isEmpty()) return " WHERE " + customWhere;
        else return where + " AND " + customWhere;
    }
    return where;
}

QString QyOrmTable::sqlSelectOrderBy() const
{
    bool belong = __privated->joinOnFields.size();
    QString orderBy;
    foreach(QyOrmField* field, __privated->orderByFields){
        if(!orderBy.isNull()) orderBy += ',';
        if(belong) orderBy += field->__privated->table->
                       __privated->extra->tableName + '.';
        if(field->__privated->orderToCastNumber && field->__privated->isString()){
            orderBy += field->sqlToNumberFieldName();
        }else{
            orderBy += field->__privated->fieldName();
        }
        orderBy += (field->__privated->isDesc ? " DESC" : " ASC");
    }
    if(!orderBy.isEmpty()) return " ORDER BY " + orderBy;
    return QString();
}

QString QyOrmTable::sqlLimit() const
{
    if(__privated->limit) {
        return " LIMIT " + QString::number(__privated->limit) +
               " OFFSET " + QString::number(__privated->offset);
    }
    return QString();
}

void QyOrmTable::setUpdateExtraWhere(const QString &where)
{
    __privated->updateWhereSql = where;
}

QString QyOrmTable::sqlUpdateString(const QList<QyOrmField *> &listField) const
{
    QString sql = "UPDATE " + __privated->extra->tableName + " SET ";
    QStringList fus;
    for(QyOrmField* field : listField){
        if(!field || field->__privated->primaryKey()) continue;
        fus << field->__privated->extraInfo->fieldName + '=' + field->sqlValueString();
    }
    if(fus.isEmpty()) return QString();
    sql += fus.join(',');
    sql += " WHERE " + __privated->primaryKey->sqlExecuteString();
    if(__privated->updateWhereSql.size()){
        sql += " AND " + __privated->updateWhereSql;
    }
    return sql + ';';
}

QString QyOrmTable::sqlUpdateChangedString() const
{
#if QT_VERSION < QT_VERSION_CHECK(5, 14, 0)
    return sqlUpdateString(__privated->recordChangedFields.toList());
#else
    return sqlUpdateString(__privated->recordChangedFields.values());
#endif
}

QString QyOrmTable::sqlUpdateFieldString(const QString &fieldName) const
{
    return sqlUpdateString({__privated->hashField.value(fieldName, nullptr)});
}

QString QyOrmTable::sqlUpdateFieldString(QyOrmField *field) const
{
    return sqlUpdateString({field});
}

QString QyOrmTable::sqlUpdateAllFieldString() const
{
    return sqlUpdateString(__privated->listField);
}

QString QyOrmTable::__sqlInsertString(const QList<QyOrmField *> &listField) const
{
    QString sql = "INSERT INTO " + __privated->extra->tableName + '(', fieldNames, fieldValues;
    for(QyOrmField* field : listField){
        if(field->__privated->autoPrimaryKey()) continue;
        fieldNames += field->sqlFieldString() + ',';
        fieldValues += field->sqlValueString() + ',';
    }
    return sql + fieldNames.mid(0, fieldNames.size() - 1) + ") VALUES(" +
            fieldValues.mid(0, fieldValues.size() - 1) + ");";
}

QString QyOrmTable::sqlInsertNotNullString() const
{
    QList<QyOrmField *> fields;
    foreach(QyOrmField* field, __privated->listField){
        if(field->__privated->isNull) continue;
        fields.append(field);
    }
    return __sqlInsertString(fields);
}

QString QyOrmTable::sqlInsertChangedString() const
{
    return __sqlInsertString(__privated->recordChangedFields.toList());
}

QString QyOrmTable::sqlInsertAllFieldString() const
{
    return __sqlInsertString(__privated->listField);
}

QString QyOrmTable::sqlDeleteString() const
{
    return "DELETE FROM " + __privated->extra->tableName + " WHERE " + __privated->primaryKey->sqlExecuteString() + ';';
}

QString QyOrmTable::sqlDeleteForChangedString() const
{
    QString sql = "DELETE FROM " + __privated->extra->tableName + " WHERE ", where;
    foreach(QyOrmField* field, __privated->recordChangedFields){
        if(!where.isEmpty()) where += " and ";
        where += field->sqlExecuteString();
    }
    if(where.isEmpty()) where = __privated->primaryKey->sqlExecuteString();
    return sql + where + ';';
}

QString QyOrmTable::sqlDeleteForValueString() const
{
    QString sql = "DELETE FROM " + __privated->extra->tableName + " WHERE ", where;
    foreach(QyOrmField* field, __privated->listField){
        if(field->notIsNull()){
            if(!where.isEmpty()) where += " AND ";
            where += field->sqlExecuteString();
        }
    }
    if(where.isEmpty()) where = __privated->primaryKey->sqlExecuteString();
    return sql + where + ';';
}

QString QyOrmTable::sqlForeignKeyString(const QString &fieldName, const QString &fkTableName,
    const QString &fkFieldName, QOFKC onDelete, QOFKC onUpdate) const
{
    QString sql = "CONSTRAINT fk_" + __privated->extra->tableName + '_' + fieldName + '_' +
            fkTableName + '_' + fkFieldName + " FOREIGN KEY(" + fieldName +
            ") REFERENCES " + fkTableName + '(' + fkFieldName + ')';
    switch (onDelete) {
    case QOFKC::Restrict: sql += " ON DELETE RESTRICT"; break;
    case QOFKC::NoAction: sql += " ON DELETE NO ACTION"; break;
    case QOFKC::Cascade: sql += " ON DELETE CASCADE"; break;
    case QOFKC::SetNull: sql += " ON DELETE SET NULL"; break;
    case QOFKC::SetDefault: sql += " ON DELETE SET DEFAULT"; break;
    default: break;
    }
    switch (onUpdate) {
    case QOFKC::Restrict: sql += " ON UPDATE RESTRICT"; break;
    case QOFKC::NoAction: sql += " ON UPDATE NO ACTION"; break;
    case QOFKC::Cascade: sql += " ON UPDATE CASCADE"; break;
    case QOFKC::SetNull: sql += " ON UPDATE SET NULL"; break;
    case QOFKC::SetDefault: sql += " ON UPDATE SET DEFAULT"; break;
    default: break;
    }
    return sql;
}

bool QyOrmTable::sqlSelectSelfByField(QyOrmField *field)
{
    QString error;
    auto datas = sqlDatabaseSelectRow(sqlSelectSelfByFieldString(field), error);
    if(error.isEmpty()){
        __privated->lastSelectError.clear();
    }else{
        __privated->lastSelectError = error;
        return false;
    }
    if(datas.isEmpty()){
        __privated->lastSelectError = "Empty";
        return false;
    }

    QList<QyOrmField*>& listField = (__privated->singleSelectFields.size() ?
                                         __privated->singleSelectFields : __privated->listField);
    __privated->isAssigning = true;
    for(int column = 0; column < listField.size(); ++column){
        if(datas.size() <= column) break;
        QVariant data = datas.at(column);
        if(data.isValid()) listField.at(column)->query(data);
    }
    __privated->isAssigning = false;
    return true;
}

bool QyOrmTable::sqlSelectSelfByFields(QList<QyOrmField *> fields)
{
    QString error;
    auto datas = sqlDatabaseSelectRow(sqlSelectSelfByFieldsString(fields), error);
    if(error.isEmpty()){
        __privated->lastSelectError.clear();
    }else{
        __privated->lastSelectError = error;
        return false;
    }
    if(datas.isEmpty()){
        __privated->lastSelectError = "Empty";
        return false;
    }

    QList<QyOrmField*>& listField = (__privated->singleSelectFields.size() ?
                                         __privated->singleSelectFields : __privated->listField);
    __privated->isAssigning = true;
    for(int column = 0; column < listField.size(); ++column){
        if(datas.size() <= column) break;
        QVariant data = datas.at(column);
        if(data.isValid()) listField.at(column)->query(data);
    }
    __privated->isAssigning = false;
    return true;
}

bool QyOrmTable::sqlSelectSelfByChangedFields()
{
    return sqlSelectSelfByFields(__privated->recordChangedFields.toList());
}

#ifdef QY_ORM_DATABASE
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include "QyOrmDatabase.h"
#include "QyOrmRemoteDatabase.h"
thread_local QString QyOrmTable::selectError;
QString QyOrmTable::sqlDatabaseExecute(const QString &sql, QList<QyOrmBlobField *> blobs)
{
    if(sql.isEmpty()) return "Empty query sql";
    if(QyOrmConfigure::enabledRemoteDatabase){
        return QyOrmRemoteDatabase::instance()->execute(sql);
    }
    QSqlDatabase db = QyOrmDatabase::getValidDatabase();
    QSqlQuery query(db);
    bool result = false;
    if(blobs.size()){
        query.prepare(sql);
        foreach(QyOrmBlobField* blob, blobs){
            query.bindValue(':' + blob->__privated->extraInfo->fieldName, blob->get(),
                            QSql::In | QSql::Binary);
        }
        result = query.exec();
    }else{
        result = query.exec(sql);
    }
    if(result){
        if(db.driverName() == "QSQLITE"){
            db.commit();
        }
        return QString();
    }
    return query.lastError().text();
}

#define QyOrmSqlDatabaseSelect(ret) \
    QSqlQuery query(QyOrmDatabase::getValidDatabase()); \
    query.setForwardOnly(true); \
    if(!query.exec(sql)){ \
        error = query.lastError().text(); \
        return ret; \
    } \

QList<QVariantList> QyOrmTable::sqlDatabaseSelectAll(
    const QString &sql, QString &error, bool withHeader, bool* canRun)
{
    if(sql.isEmpty()) return QList<QVariantList>();
    if(QyOrmConfigure::enabledRemoteDatabase){
        return QyOrmRemoteDatabase::instance()->selectAll(sql, error, withHeader);
    }
    QList<QVariantList> result;
    QyOrmSqlDatabaseSelect(result)
    QSqlRecord record = query.record();
    int columnCount = record.count() ;
    if(withHeader){
        QVariantList headers;
        for(int i = 0; i < columnCount; ++i){
            headers.append(record.fieldName(i));
        }
        result.append(headers);
    }
    const QVariant Null;
    while(query.next()){
        if(canRun && !*canRun) return result;
        QVariantList row;
        for(int index = 0; index < columnCount; ++index) {
            QVariant data = query.value(index);
            row.append(data.isNull() ? Null : data);
        }
        result.append(row);
    }
    return result;
}

QList<QVariantList> QyOrmTable::sqlDatabaseSelectRowWithHeader(const QString &sql, QString &error)
{
    if(sql.isEmpty()) return QList<QVariantList>();
    if(QyOrmConfigure::enabledRemoteDatabase){
        return QyOrmRemoteDatabase::instance()->selectRowWithHeader(sql, error);
    }
    QList<QVariantList> result;
    QyOrmSqlDatabaseSelect(result)
    QSqlRecord record = query.record();
    int columnCount = record.count() ;
    QVariantList headers;
    for(int i = 0; i < columnCount; ++i){
        headers.append(record.fieldName(i));
    }
    result.append(headers);
    while(query.next()){
        QVariantList row;
        for(int index = 0; index < columnCount; ++index) {
            QVariant data = query.value(index);
            row.append(data.isNull() ? QVariant() : data);
        }
        result.append(row);
    }
    return result;
}

QVariantList QyOrmTable::sqlDatabaseSelectRow(const QString &sql, QString &error)
{
    if(sql.isEmpty()) return QVariantList();
    if(QyOrmConfigure::enabledRemoteDatabase){
        return QyOrmRemoteDatabase::instance()->selectRow(sql, error);
    }
    QVariantList result;
    QyOrmSqlDatabaseSelect(result)
    QSqlRecord record = query.record();
    int columnCount = record.count();
    if(!query.next()) return QVariantList();
    for(int index = 0; index < columnCount; ++index) {
        QVariant data = query.value(index);
        result.append(data.isNull() ? QVariant() : data);
    }
    return result;
}

QVariantList QyOrmTable::sqlDatabaseSelectColumn(const QString &sql, QString &error)
{
    if(sql.isEmpty()) return QVariantList();
    if(QyOrmConfigure::enabledRemoteDatabase){
        return QyOrmRemoteDatabase::instance()->selectColumn(sql, error);
    }
    QVariantList result;
    QyOrmSqlDatabaseSelect(result)
    while (query.next()) {
        result.append(query.value(0));
    }
    return result;
}

QVariant QyOrmTable::sqlDatabaseSelectOne(const QString &sql, QString &error)
{
    if(sql.isEmpty()) return QVariant();
    if(QyOrmConfigure::enabledRemoteDatabase){
        return QyOrmRemoteDatabase::instance()->selectOne(sql, error);
    }
    QVariant result;
    QyOrmSqlDatabaseSelect(result)
    if (query.next()) {
        QVariant data = query.value(0);
        if(data.isValid()) result = data;
    }
    return result;
}

QString QyOrmTable::getLastError()
{
    return __privated->lastError;
}

bool QyOrmTable::sqlExecute(const QString &sql)
{
    QString error = sqlDatabaseExecute(sql, __privated->blobFields);
    if(error.isEmpty()) return true;
    __privated->lastError = error;
    return false;
}

bool QyOrmTable::sqlSelectSelf()
{
    QString error;
    auto datas = sqlDatabaseSelectRow(sqlSelectSelfString(), error);
    if(error.isEmpty()){
        __privated->lastSelectError.clear();
    }else{
        __privated->lastSelectError = error;
        return false;
    }
    if(datas.isEmpty()){
        __privated->lastSelectError = "Empty";
        return false;
    }

    QList<QyOrmField*>& listField = (__privated->singleSelectFields.size() ?
                                         __privated->singleSelectFields : __privated->listField);
    __privated->isAssigning = true;
    for(int column = 0; column < listField.size(); ++column){
        if(datas.size() <= column) break;
        listField.at(column)->query(datas.at(column));
    }
    __privated->isAssigning = false;
    return true;
}

bool QyOrmTable::sqlSelectSelfField(QyOrmField *field)
{
    QString error;
    auto data = sqlDatabaseSelectOne(sqlSelectFieldString(field->fieldName()) + " WHERE "
                                         + primaryKey()->sqlExecuteString(), error);
    if(error.isEmpty()){
        __privated->lastSelectError.clear();
    }else{
        __privated->lastSelectError = error;
        return false;
    }

    __privated->isAssigning = true;
    field->query(data);
    __privated->isAssigning = false;
    return data.isValid();
}

bool QyOrmTable::sqlSelectSelfFields(QList<QyOrmField *> fields)
{
    QStringList fieldNames;
    foreach(const QyOrmField* field, fields){
        fieldNames.append(field->fieldName());
    }
    QString error;
    auto datas = sqlDatabaseSelectRow(sqlSelectFieldString(fieldNames.join(',')) + " WHERE "
                                          + primaryKey()->sqlExecuteString(), error);
    if(error.isEmpty()){
        __privated->lastSelectError.clear();
    }else{
        __privated->lastSelectError = error;
        return false;
    }
    if(datas.isEmpty()){
        __privated->lastSelectError = "Empty";
        return false;
    }

    __privated->isAssigning = true;
    for(int index = 0; index < datas.size(); ++index){
        fields.at(index)->query(datas.at(index));
    }
    __privated->isAssigning = false;
    return true;
}

bool QyOrmTable::sqlSelectSelfFieldNames(QStringList fieldNames)
{
    QList<QyOrmField *> fields;
    foreach(const QString& fieldName, fieldNames){
        QyOrmField* field = this->field(fieldName);
        fields.append(field);
    }
    return sqlSelectSelfFields(fields);
}

QString QyOrmTable::getLastSelectError()
{
    return __privated->lastSelectError;
}

QList<QyOrmTable *> QyOrmTable::sqlSelectAllFor(const QString &selectSql, bool disorder)
{
    QString error;
    auto datas = sqlDatabaseSelectAll(selectSql, error, disorder);
    if(error.isEmpty()){
        __privated->lastSelectError.clear();
    }else{
        __privated->lastSelectError = error;
        return QList<QyOrmTable *>();
    }
    if(datas.isEmpty()) return QList<QyOrmTable *>();

    int columnCount = datas.first().count();
    QList<int> columnIndexs;
    if(disorder){
        auto headers = datas.takeFirst();
        for(int column = 0; column < columnCount; ++column){
            QString fieldName = headers.at(column).toString();
            int index = __privated->extra->hashFieldIndex.value(fieldName.toLower(), -1);
            columnIndexs.append(index);
        }
    }else if(__privated->singleSelectFields.size()){
        foreach(QyOrmField* field, __privated->singleSelectFields){
            int index = __privated->extra->hashFieldIndex.value(field->fieldName().toLower(), -1);
            columnIndexs.append(index);
        }
    }else{
        columnCount = qMin(columnCount, __privated->listField.count());
    }

    QList<QyOrmTable *> result;
    for(auto iter = datas.begin(); iter != datas.end(); ++iter){
        QyOrmTable* table = constructor(parent());
        table->__privated->isAssigning = true;
        for(int column = 0; column < columnCount; ++column){
            int col = (columnIndexs.size() ? columnIndexs.at(column) : column);
            if(col < 0) continue;
            QVariant data = iter->at(column);
            if(data.isValid()){
                table->__privated->listField.at(col)->query(data);
            }
        }
        table->__privated->isAssigning = false;
        result.append(table);
    }
    return result;
}

QList<QyOrmTable *> QyOrmTable::sqlSelectAll(const QString &customWhere)
{
    if(__privated->forceSelectSql.size()){
        return sqlSelectAllFor(__privated->forceSelectSql, true);
    }
    return sqlSelectAllFor(sqlSelectAllString(customWhere), false);
}

QyOrmTable *QyOrmTable::sqlSelectOne()
{
    auto datas = sqlSelectAll();
    if(datas.isEmpty()) return nullptr;
    QyOrmTable * table = datas.takeFirst();
    qDeleteAll(datas);
    return table;
}

QyOrmTable *QyOrmTable::sqlSelectPrevious(const QString& fieldName, bool toNumber, QVariant data)
{
    QyOrmField* tempField = __privated->hashField.value(fieldName, nullptr);
    if(!tempField || tempField->isNull()) return nullptr;
    QString tv;
    if(data.isValid()){
        tv = data.toString();
        if(tempField->__privated->isString() ||
            tempField->__privated->fieldType() == QyOrmFieldType::DateTime) {
            tv = '\'' + tv + '\'';
        }
    }else{
        tv = tempField->sqlValueString();
    }
    if(toNumber && tempField->__privated->isString()){
        tv = QyOrmField::toNumber(tv);
    }
    QString sql = QString("%1=(SELECT MAX(%1) FROM %2 WHERE %1<%3")
                      .arg(toNumber ? tempField->sqlToNumberFieldName() : fieldName,
                           __privated->extra->tableName, tv);
    if(data.isValid()){
        sql += QString(" AND %1!=").arg(primaryKey()->fieldName()) +
               primaryKey()->sqlValueString() + ')';
    }else{
        sql += ')';
    }
    auto datas = sqlSelectAll(sql);
    if(datas.isEmpty()) return nullptr;
    QyOrmTable * table = datas.takeFirst();
    qDeleteAll(datas);
    return table;
}

QyOrmTable *QyOrmTable::sqlSelectNext(const QString& fieldName, bool toNumber, QVariant data)
{
    QyOrmField* tempField = __privated->hashField.value(fieldName, nullptr);
    if(!tempField || (tempField->isNull() && data.isNull())) return nullptr;

    QString tv;
    if(data.isValid()){
        tv = data.toString();
        if(tempField->__privated->isString() ||
            tempField->__privated->fieldType() == QyOrmFieldType::DateTime) {
            tv = '\'' + tv + '\'';
        }
    }else{
        tv = tempField->sqlValueString();
    }
    if(toNumber && tempField->__privated->isString()){
        tv = QyOrmField::toNumber(tv);
    }
    QString sql = QString("%1=(SELECT MIN(%1) FROM %2 WHERE %1>%3")
                      .arg(toNumber ? tempField->sqlToNumberFieldName() : fieldName,
                           __privated->extra->tableName, tv);
    if(data.isValid()){
        sql += QString(" AND %1!=").arg(primaryKey()->fieldName()) +
               primaryKey()->sqlValueString() + ')';
    }else{
        sql += ')';
    }
    auto datas = sqlSelectAll(sql);
    if(datas.isEmpty()) return nullptr;
    QyOrmTable * table = datas.takeFirst();
    qDeleteAll(datas);
    return table;
}

QyOrmTable *QyOrmTable::sqlSelectMax(const QString &fieldName, bool toNumber)
{
    QyOrmField* tempField = __privated->hashField.value(fieldName, nullptr);
    if(!tempField || tempField->isNull()) return nullptr;

    auto datas = sqlSelectAll(QString("%1=(SELECT MAX(%1) FROM %2")
                                  .arg(toNumber ? tempField->sqlToNumberFieldName() : fieldName,
                                       __privated->extra->tableName));
    if(datas.isEmpty()) return nullptr;
    QyOrmTable * table = datas.takeFirst();
    qDeleteAll(datas);
    return table;
}

QyOrmTable *QyOrmTable::sqlSelectMin(const QString &fieldName, bool toNumber)
{
    QyOrmField* tempField = __privated->hashField.value(fieldName, nullptr);
    if(!tempField || tempField->isNull()) return nullptr;

    auto datas = sqlSelectAll(QString("%1=(SELECT MIN(%1) FROM %2")
                                  .arg(toNumber ? tempField->sqlToNumberFieldName() : fieldName,
                                       __privated->extra->tableName));
    if(datas.isEmpty()) return nullptr;
    QyOrmTable * table = datas.takeFirst();
    qDeleteAll(datas);
    return table;
}

bool QyOrmTable::sqlSelectUnique()
{
    if(__privated->recordChangedFields.isEmpty()) return false;
    foreach(QyOrmField* field, __privated->recordChangedFields){
        if(field->isUnique()){
            return sqlSelectFieldData(field->fieldName()).isNull();
        }
    }
    return false;
}

QVariant QyOrmTable::sqlSelectFieldData(const QString &fieldName)
{
    QString error;
    const QVariant& data = sqlDatabaseSelectOne(
        sqlSelectFieldString(fieldName) + sqlSelectWhereOrderByLimit(), error);
    if(error.isEmpty()){
        __privated->lastSelectError.clear();
    }else{
        __privated->lastSelectError = error;
    }
    return data;
}

QList<QPointer<QyOrmTable> > QyOrmTable::sqlSelectAllSafe()
{
    QList<QPointer<QyOrmTable> > result;
    foreach(QyOrmTable * table, sqlSelectAll()){
        result.append(table);
    }
    return result;
}

QList<QPointer<QyOrmTable> > QyOrmTable::sqlSelectAllSimpleSafe()
{
    QList<QPointer<QyOrmTable> > result;
    foreach(QyOrmTable * table, sqlSelectAllSimple()){
        result.append(table);
    }
    return result;
}

QPointer<QyOrmTable> QyOrmTable::sqlSelectOneSafe()
{
    return sqlSelectOne();
}

bool QyOrmTable::sqlSelectAllFor(QList<QyOrmTable *> &tables, const QString &selectSql, bool disorder)
{
    QString error;
    auto datas = sqlDatabaseSelectAll(selectSql, error, disorder);
    if(error.isEmpty()){
        __privated->lastSelectError.clear();
    }else{
        __privated->lastSelectError = error;
        qDeleteAll(tables);
        tables.clear();
        return false;
    }
    if(datas.isEmpty()) {
        qDeleteAll(tables);
        tables.clear();
        return true;
    }

    int columnCount = 0;
    QList<int> columnIndexs;
    if(disorder){
        auto headers = datas.takeFirst();
        columnCount = headers.size();
        for(int column = 0; column < columnCount; ++column){
            QString fieldName = headers.at(column).toString();
            int index = __privated->extra->hashFieldIndex.value(fieldName.toLower(), -1);
            columnIndexs.append(index);
        }
    }else if(__privated->singleSelectFields.size()){
        foreach(QyOrmField* field, __privated->singleSelectFields){
            int index = __privated->extra->hashFieldIndex.value(field->fieldName().toLower(), -1);
            columnIndexs.append(index);
        }
    }else{
        columnCount = __privated->listField.size();
    }

    for(int index = 0; index < datas.size() && index < tables.size(); ++index){
        QyOrmTable* table = tables.at(index);
        const QVariantList& iter = datas.at(index);
        table->__privated->isAssigning = true;
        for(int column = 0; column < columnCount; ++column){
            int col = (columnIndexs.size() ? columnIndexs.at(column) : column);
            if(col < 0) continue;
            const QVariant& data = iter.at(column);
            QyOrmField* field = table->__privated->listField.at(col);
            if(data.isNull() && field->__privated->extraInfo->defaultValue.isValid()){
                field->query(field->__privated->extraInfo->defaultValue);
            }else{
                field->query(data);
            }
        }
        table->__privated->isAssigning = false;
    }

    for(int index = tables.size(); index < datas.size(); ++index){
        QyOrmTable* table = constructor(parent());
        const QVariantList& iter = datas.at(index);
        table->__privated->isAssigning = true;
        for(int column = 0; column < columnCount; ++column){
            int col = (columnIndexs.size() ? columnIndexs.at(column) : column);
            if(col < 0) continue;
            const QVariant& data = iter.at(column);
            if(data.isValid()){
                table->__privated->listField.at(col)->query(data);
            }
        }
        table->__privated->isAssigning = false;
        tables.append(table);
    }

    for(int index = tables.size() - 1; index >= datas.size(); --index){
        delete tables.takeAt(index);
    }
    return true;
}

bool QyOrmTable::sqlSelectAll(QList<QyOrmTable *> &tables)
{
    return sqlSelectAllFor(tables, sqlSelectAllString(), false);
}

int QyOrmTable::sqlSelectCount() const
{
    return sqlDatabaseSelectOne(sqlSelectCountString()).toInt();
}

#endif

QHash<QString, QyOrmTableExtraInfo*> QyOrmTablePrivate::hashTableNameHashTableExtraInfoReady;
QHash<QString, QHash<QString, QyOrmFieldExtraInfo*>> QyOrmTablePrivate::hashTableNameHashFieldExtraInfoReady;
QHash<QString, QHash<QString, QyOrmField*>> QyOrmTablePrivate::hashTableNameHashFieldReady;
/*
 * hashField、listFieldName、listField、field-extraInfo
*/
void QyOrmTable::__initialize()
{
    if(__privated) return;
    __privated = new QyOrmTablePrivate;

    if(callNew){
        __privated->atHeap = true;
        callNew = false;
    }

    QString tableName = __dynamicTableName.isEmpty() ? metaObject()->className() : __dynamicTableName;
    bool hasHash = __privated->hashTableNameHashFieldExtraInfoReady.contains(tableName);
    if(!hasHash){
        __privated->hashTableNameHashFieldExtraInfoReady.insert(tableName, QHash<QString, QyOrmFieldExtraInfo*>());
        __privated->extra = new QyOrmTableExtraInfo;
        __privated->hashTableNameHashTableExtraInfoReady.insert(tableName, __privated->extra);
    }else{
        __privated->extra = __privated->hashTableNameHashTableExtraInfoReady.value(tableName);
    }
    QHash<QString, QyOrmFieldExtraInfo*>& hashReady = __privated->hashTableNameHashFieldExtraInfoReady[tableName];

    if(hasHash){
        __privated->hashField = __privated->hashTableNameHashFieldReady.value(tableName);
    }
    int metaObjectPropertyCount = metaObject()->propertyCount();
    int hashFieldIndex = -1;
    for(int i = 1; i < metaObjectPropertyCount; ++i){
        auto m = metaObject()->property(i);
        QString name = m.name(), typeName = m.typeName();
        if(typeName == "QyOrmField*"){
            QyOrmField* field = *reinterpret_cast<QyOrmField**>(property(m.name()).data());
            field->__privated->table = this;
            __privated->listField.append(field);
            if(hasHash){
                __privated->hashField[name] = field;
                field->__privated->extraInfo = hashReady.value(name);
                field->__privated->isPassSelect = field->__privated->extraInfo->isPassSelect;
            }else{
                __privated->hashField.insert(name, field);
                __privated->extra->listFieldName.append(name);
                __privated->extra->hashFieldIndex.insert(name.toLower(), ++hashFieldIndex);
            }
        }else if(!hasHash && typeName == "QyOrmFieldExtraInfo*"){
            QyOrmFieldExtraInfo* extraInfo = *reinterpret_cast<QyOrmFieldExtraInfo**>(property(m.name()).data());
            QString fieldName = name.right(name.size() - 20);
            QyOrmField* field = __privated->hashField.value(fieldName);
            field->__privated->extraInfo = extraInfo;
            if(!field->__privated->isNull) extraInfo->defaultValue = field->answer();
            extraInfo->fieldName = fieldName;
            hashReady.insert(fieldName, extraInfo);
            if(extraInfo->fieldTypeString.size() > 6){
                switch (extraInfo->fieldTypeString.at(5).unicode()) {
                case 'S': extraInfo->fieldType = QyOrmFieldType::String;
                    extraInfo->isString = true; break;
                case 'I': extraInfo->fieldType = QyOrmFieldType::Integer; break;
                case 'D': extraInfo->fieldType = (extraInfo->fieldTypeString.at(6) == 'o' ?
                                                      QyOrmFieldType::Double :QyOrmFieldType::DateTime); break;
                case 'B': extraInfo->fieldType = (extraInfo->fieldTypeString.at(6) == 'o' ?
                                                      QyOrmFieldType::Boolean :QyOrmFieldType::Blob); break;
                case 'T': extraInfo->fieldType = QyOrmFieldType::Text;
                    extraInfo->isString = true; break;
                default: extraInfo->fieldType = QyOrmFieldType::String; break;
                }
            }
            if(extraInfo->type == QyOrmFieldConstraint::PrimaryKey ||
                    extraInfo->type == QyOrmFieldConstraint::AutoPrimaryKey){
                __privated->extra->primaryKeyFieldName = fieldName;
                __privated->primaryKey = field;
            }
        }
    }
    if(!hasHash){
        __privated->hashTableNameHashFieldReady.insert(tableName, __privated->hashField);
        __privated->extra->tableName = tableName;
    }else{
        __privated->primaryKey = __privated->hashField.value(__privated->extra->primaryKeyFieldName, nullptr);
    }
    __privated->isAssigning = false;
}
