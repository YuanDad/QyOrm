#include "Table.h"
#include <QSqlDriver>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlError>
#include <QMetaProperty>
#include <QThread>
#include <QJsonObject>
#include <QJsonArray>
#include <QSet>

using namespace Qy;
static thread_local bool callNew = false;
QString Table::testConnectSql = "SELECT 1;";
QSqlDatabase Table::db;
uint Table::maxTimeOut;
QHash<QString, QHash<QString, Field*>> Table::hashClassNameHashFieldReady;
QHash<QString, QStringList> Table::hashClassNameListFieldNameReady;
QHash<QString, Table*> Table::hashTable;
uint Table::timeStampTimeout = 0;
QHash<QString, QStringList> Table::hashPartSelect;

static int qRegisterMetaType()
{
    int result = qRegisterMetaType<Qy::ForeignField>("Qy::ForeignField");
    result = qRegisterMetaType<Qy::StringField>("Qy::StringField");
    result = qRegisterMetaType<Qy::IntegerField>("Qy::IntegerField");
    result = qRegisterMetaType<Qy::DoubleField>("Qy::DoubleField");
    result = qRegisterMetaType<Qy::DateTimeField>("Qy::DateTimeField");
    result = qRegisterMetaType<Qy::BooleanField>("Qy::BooleanField");
    result = qRegisterMetaType<Qy::TextField>("Qy::TextField");
    result = qRegisterMetaType<Qy::BlobField>("Qy::BlobField");
    return result;
}

class SqlDatabaseWriter{
public:
    inline SqlDatabaseWriter(): db(Table::database()) {}
    inline explicit SqlDatabaseWriter(QSqlDatabase db):db(db){if(!db.isOpen()) db.open();}
    inline ~SqlDatabaseWriter(){if(db.driverName() == "QSQLITE") db.close();}
    inline QSqlDatabase database() {return db;}
    inline void trySupportTransaction() { supportTransaction = db.driver()->hasFeature(QSqlDriver::Transactions); }
    inline bool transaction() { if(supportTransaction) return db.transaction(); return true; }
    inline bool rollback() { if(supportTransaction) return db.rollback(); return true; }
    inline bool commit() { if(supportTransaction) return db.commit(); return true; }
    inline bool execute(const QString& sql) { QSqlQuery query(db); if(query.exec(sql)) return true;
        Table::setLastError(query.lastQuery(), query.lastError().text()); return false; }
    inline QSqlQuery select(const QString& sql) { QSqlQuery query(db); query.setForwardOnly(true); if(query.exec(sql)) return query;
        Table::setLastError(query.lastQuery(), query.lastError().text()); return QSqlQuery(); }
    QSqlDatabase db;
    bool supportTransaction;
};

Table::Table() : QObject(nullptr)
{
    static int b_qRegisterMetaType = qRegisterMetaType();
    Q_UNUSED(b_qRegisterMetaType)
    if(callNew){
        atHeap = true;
        callNew = false;
    }
}

void Table::initialize()
{
    tableName = this->metaObject()->className();
    bool hasHashField = hashClassNameHashFieldReady.contains(tableName);
    if(hasHashField){
        hashField = hashClassNameHashFieldReady.value(tableName);
        listFieldName = hashClassNameListFieldNameReady.value(tableName);
    }
    int metaObjectPropertyCount = metaObject()->propertyCount();
    for(int i = 1; i < metaObjectPropertyCount; ++i){
        auto m = metaObject()->property(i);
        QString name = m.name(), typeName = m.typeName();
        if(typeName == "char"){
            int _index = name.indexOf('_');
            QString type = name.left(_index);
            QString field = name.mid(_index + 1);
            if(type == "auto"){
                primaryKey = hashField.value(field);
                primaryKey->autoPrimaryKey = primaryKey->primaryKey = primaryKey->notNull = primaryKey->unique = true;
            }else if(type == "primarykey"){
                primaryKey = hashField.value(field);
                primaryKey->primaryKey = primaryKey->notNull = primaryKey->unique = true;
            }else if(type == "notnull"){
                hashField.value(field)->notNull = true;
            }else{
                hashField.value(field)->unique = true;
            }
        }else if(typeName == "short"){
            int _index = name.indexOf('_');
            int _lastIndex = name.lastIndexOf('_');
            QString type = name.left(_index);
            QString fieldName = name.mid(_index + 1, _lastIndex - _index - 1);
            QString shortName = name.mid(_lastIndex + 1);
            if(type == "foreign"){
                hashForeign.insert(fieldName, shortName);
            }else if(type == "func"){
                QMetaObject::invokeMethod(this, shortName.toStdString().data());
            }
        }else{
            Field* field = nullptr;
            if(typeName == "Qy::StringField"){
                field = reinterpret_cast<Qy::StringField*>(property(m.name()).data())->self;
            }else if(typeName == "Qy::IntegerField"){
                field = reinterpret_cast<Qy::IntegerField*>(property(m.name()).data())->self;
            }else if(typeName == "Qy::DoubleField"){
                field = reinterpret_cast<Qy::DoubleField*>(property(m.name()).data())->self;
            }else if(typeName == "Qy::DateTimeField"){
                field = reinterpret_cast<Qy::DateTimeField*>(property(m.name()).data())->self;
            }else if(typeName == "Qy::BooleanField"){
                field = reinterpret_cast<Qy::BooleanField*>(property(m.name()).data())->self;
            }else if(typeName == "Qy::TextField"){
                field = reinterpret_cast<Qy::TextField*>(property(m.name()).data())->self;
            }else if(typeName == "Qy::BlobField"){
                field = reinterpret_cast<Qy::BlobField*>(property(m.name()).data())->self;
            }else /*if(typeName == "Qy::ForeignField")*/{
                field = reinterpret_cast<Qy::ForeignField*>(property(m.name()).data())->self;
            }
            listField.append(field);
            field->fieldName = name;
            field->table = this;
            if(hasHashField){
                hashField[name] = field;
            }else{
                hashField.insert(name, field);
                listFieldName.append(name);
            }
        }
    }
    if(!hasHashField){
        hashClassNameHashFieldReady.insert(tableName, hashField);
        hashClassNameListFieldNameReady.insert(tableName, listFieldName);
    }
}

QString Table::parseSelectFrom(const Table *table)
{
    QString result = table->tableName;
    for(const Table* tmp : table->innerJoin){
        result = QString("(%1 inner join %2 on %3.%4 = %2.%4)").arg(result).arg(tmp->tableName).arg(table->tableName).arg(tmp->innerJoinFieldName);
    }
    return result;
}

void Table::parseSelectSql(QSqlQuery& query, const Table *table, QStringList& area)
{
    QString areaString = "*";
    if(!area.isEmpty()){
        for(int i = area.size() - 1; i >= 0; --i){
            if(!table->hashField.contains(area.at(i))){
                area.removeAt(i);
            }
        }
        if(!area.isEmpty()){
            areaString.clear();
            for(const QString& field : area) areaString += table->tableName + '.' + field + ',';
            areaString.chop(1);
        }
    }
    QString sql = QString("select %1 from %2").arg(areaString).arg(parseSelectFrom(table));
    if(!table->specialSql.isEmpty()) sql += ' ' + table->specialSql + ' ';
    QString where = table->getLimitSql();

    if(!where.isEmpty() || !table->whereSql.isEmpty()){
        sql += " where ";
        if(!where.isEmpty()){
            sql += where;
        }
        if(!table->whereSql.isEmpty()){
            if(!where.isEmpty()){
                sql += " and ";
            }
            sql += table->whereSql;
        }
    }
    query.prepare(sql);
}

Table *Table::parseQuery(QSqlQuery &query, Table *table, QStringList area)
{
    for(const QString& areaFieldName : (area.isEmpty() ? table->listFieldName : area)){
        table->hashField.value(areaFieldName)->query(query.value(areaFieldName));
    }
    if(!area.isEmpty()){
        table->isPart = true;
        table->area = area;
    }
    return table;
}

void Table::parseSelectInstance(Table *&table)
{
    Table* ins = table->createInstance();
    if(ins != nullptr){
        delete table;
        table = ins;
    }
}

bool Table::update(Field *field, bool checkInstance)
{
    QString setString;
    BlobFieldBindValues bfbvs;
    if(field){
        setString = field->execute(false);
        if(field->fieldType == FieldType::Blob){
            bfbvs.append(BlobFieldBindValue(field->select(), field->answer()));
        }
    }else{
        for(Field* field : fieldChanged){
            if(!field->primaryKey){
                setString += field->execute(false) + ',';
                if(field->fieldType == FieldType::Blob){
                    bfbvs.append(BlobFieldBindValue(field->select(), field->answer()));
                }
            }
        }
        setString.chop(1);
    }
    if(setString.isEmpty()) return false;
    QString sql = QString("update %1 set %2 where %3").arg(tableName).arg(setString).arg(primaryKey->condition(false));

    SqlDatabaseWriter dbw(database());
    QSqlQuery query(dbw.database());
    query.prepare(sql);
    for(const BlobFieldBindValue& bfbv : bfbvs){
        query.bindValue(bfbv.placeholder, bfbv.val);
    }
    if(query.exec()){
        if(checkInstance && !instance){
            Table* real = searchInstance();
            if(real){
                if(field){
                    real->setFieldValue(field->fieldName, field->answer());
                }else{
                    for(Field* field : fieldChanged){
                        if(!field->primaryKey){
                            real->setFieldValue(field->fieldName, field->answer());
                        }
                    }
                    real->clearChanged();
                }
            }
        }
        return true;
    }else{
        setLastError(sql, query.lastError().text());
    }
    return false;
}

bool Table::insert(bool checkInstance, bool writeNull)
{
    if(checkInstance && (instance || searchInstance())) return false;
    QString keys, values;
    BlobFieldBindValues bfbvs;
    for(QString& fieldName : listFieldName){
        Field* field = hashField.value(fieldName);
        if(field->isNull){
            if(!field->autoPrimaryKey){
                if(field->notNull) return false;
                if(writeNull){
                    keys += field->fieldName + ',';
                    values += "NULL,";
                }
            }
        }else{
            keys += field->fieldName + ',';
            values += field->select() + ',';
            if(field->fieldType == FieldType::Blob){
                bfbvs.append(BlobFieldBindValue(field->select(), field->answer()));
            }
        }
    }
    if(keys.isEmpty()) return false;
    keys.chop(1);
    values.chop(1);
    QString sql = QString("insert into %1 (%2) values(%3);").arg(tableName).arg(keys).arg(values);

    SqlDatabaseWriter dbw(database());
    QSqlQuery query(dbw.database());
    query.prepare(sql);
    for(const BlobFieldBindValue& bfbv : bfbvs){
        query.bindValue(bfbv.placeholder, bfbv.val);
    }
    if(query.exec()){
        if(checkInstance){
            return (createInstance() == nullptr);
        }
        return true;
    }else{
        setLastError(sql, query.lastError().text());
    }
    return false;
}

bool Table::remove()
{
    QString sql = QString("delete from %1 where %2;").arg(tableName).arg(primaryKey->condition(false));
    SqlDatabaseWriter write;
    if(write.execute(sql)){
        if(hashTable.contains(instanceKey)){
            hashTable.take(instanceKey)->deleteLater();
        }
        return true;
    }
    return false;
}

QString Table::createSqlString()
{
    QString sql = QString("create table %1(").arg(tableName);
    for(QString& fieldName : listFieldName){
        Field* field = hashField.value(fieldName);
        sql += field->create() + ',';
    }
    for(QString& key : hashForeign.keys()){
        Table* table = dynamic_cast<ForeignField*>(hashField.value(key))->get();
        sql += QString("foreign key(%1) references %2(%3),").arg(key).arg(table->tableName).arg(table->primaryKey->fieldName);
    }
    sql.chop(1);
    sql += ')';
    return sql;
}

bool Table::create()
{
    return SqlDatabaseWriter().execute(createSqlString());
}

QJsonObject Table::toQJson(bool completeForeign) const
{
    QJsonObject object;
    const QStringList* realFieldList = &listFieldName;
    if(isPart){
        realFieldList = &area;
    }
    for(const QString& fieldName : *realFieldList){
        object.insert(fieldName, hashField.value(fieldName)->toQJson(completeForeign));
    }
    return object;
}

QString Table::toJson(bool toObject, bool completeForeign) const
{
    QString result = (toObject ? "{" : "[");
    const QStringList* realFieldList = &listFieldName;
    if(isPart){
        realFieldList = &area;
    }
    for(const QString& fieldName : *realFieldList){
        if(toObject){
            result += QString("\"%1\":").arg(fieldName);
        }
        result += hashField.value(fieldName)->toJson(completeForeign) + ',';
    }
    result[result.size() - 1] = (toObject ? '}' : ']');
    return result;
}

QString Table::changedToJson(bool toObject, bool completeForeign) const
{
    QString result = (toObject ? "{" : "[");
    for(Field* field : fieldChanged){
        if(toObject){
            result += QString("\"%1\":").arg(field->fieldName);
        }
        result += field->toJson(completeForeign) + ',';
    }
    if(toObject){
        result += QString("\"%1\":").arg(primaryKey->fieldName);
    }
    result += primaryKey->toJson(completeForeign);
    result += (toObject ? '}' : ']');
    return result;
}

QJsonObject Table::fromJson(const QByteArray &jsonBytes)
{
    if(!jsonBytes.isEmpty()){
        QJsonDocument document = QJsonDocument::fromJson(jsonBytes);
        if(document.isObject()){
            return fromQJson(document.object());
        }
    }
    return QJsonObject();
}

QJsonObject Table::fromQJson(const QJsonObject &object)
{
    QJsonObject extra;
    for(const QString& key : object.keys()){
        QJsonValue value = object.value(key);
        if(hashField.contains(key)){
            if(value.isArray()){
                QJsonArray array = value.toArray();
                if(array.size() > 1) hashField.value(key)->setRange(array.toVariantList());
                else if(array.size() > 0) hashField.value(key)->query(array.first().toVariant());
            }else if(value.isObject()){
                QJsonObject object = value.toObject();
                if(object.contains("operator") && object.contains("value")){
                    if(object.value("operator").toString().toLower() == "between"){
                        QJsonArray array = object.value("value").toArray();
                        if(array.size() == 2) {
                            hashField.value(key)->query(array.first().toVariant());
                            hashField.value(key)->setBetween(array.at(1).toVariant());
                        }
                    }else{
                        hashField.value(key)->query(object.value("value").toVariant());
                        hashField.value(key)->setConditionOperator(object.value("operator").toString());
                        if(object.contains("connect")){
                            hashField.value(key)->setConditionConnectOperatorIsOr();
                        }
                    }
                }
            }else{
                hashField.value(key)->query(value.toVariant());
            }
        }else{
            extra.insert(key, value);
        }
    }
    return extra;
}

bool Table::updateTablesField(const QStringList &primaryKeys, const QStringList &fieldNames, const QList<QStringList> &fieldValues)
{
    if(primaryKeys.size() != fieldValues.size()) return false;
    SqlDatabaseWriter dbw(database());
    dbw.trySupportTransaction();
    dbw.transaction();
    QSqlQuery query(dbw.database());
    QString baseSqlString = "update " + tableName + " set %1 where " + primaryKey->fieldName + (primaryKey->isString() ? "='%2'" : "=%2");

    QList<bool> fieldsIsString;
    for(const QString& fieldName : fieldNames){
        fieldsIsString.append(hashField.value(fieldName)->isString());
    }

    for(int cursor = 0; cursor < primaryKeys.size(); ++cursor){
        QString setString;
        for(int col = 0; col < fieldNames.size(); ++col){
            setString += fieldNames.at(col) + '=' + fieldValues.at(cursor).at(col) + ',';
        }
        setString.chop(1);

        if(!query.exec(baseSqlString.arg(setString).arg(primaryKeys.at(cursor)))){
            setLastError(query.lastQuery(), query.lastError().text());
            dbw.rollback();
            return false;
        }
    }
    return dbw.commit();
}

void Table::setNew(bool withoutBackstage)
{
    for(Field* field : listField){
        if(withoutBackstage && field->isBackstage) continue;
        field->clear();
    }
    clearChanged();
}

bool Table::isTimeout()
{
    return (timeStampTimeout != 0 && QDateTime::currentDateTime().toTime_t() - timeStamp > timeStampTimeout);
}

QPair<QString, QString> Table::lastError(const QString &sqlString, const QString &errorString)
{
    static QHash<QString, QPair<QString, QString> > hashThreadSqlError;
    QString threadId = QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));
    if(hashThreadSqlError.contains(threadId)){
        if(sqlString.isEmpty()){
            return hashThreadSqlError.value(threadId);
        }else{
            QPair<QString, QString>& errorPair = hashThreadSqlError[threadId];
            errorPair.first = sqlString;
            errorPair.second = errorString;
            return errorPair;
        }
    }else{
        QPair<QString, QString> errorPair;
        if(!sqlString.isEmpty()){
            errorPair.first = sqlString;
            errorPair.second = errorString;
        }
        return errorPair;
    }
}

void Table::setLastError(const QString &sqlString, const QString &errorString)
{
    lastError(sqlString, errorString);
}

bool Table::isInstance()
{
    return instance;
}

Table *Table::searchInstance()
{
    return searchInstance(tableName, primaryKey->select());
}

void Table::setFieldValue(const QString &fieldName, const QVariant &value)
{
    Field* field = hashField.value(fieldName, nullptr);
    if(!field) return;
    field->query(value);
}

QVariant Table::getFieldValue(const QString &fieldName)
{
    Field* field = hashField.value(fieldName, nullptr);
    if(!field) return QVariant();
    return field->answer();
}

QList<Table *> Table::createTable(const QString &className, const QVariant &var)
{
    Constructor constructor = constructors().value( className );
    if ( constructor == nullptr ) return {};
    return (*constructor)(var);
}

QHash<QString, Table::Constructor> &Table::constructors()
{
    static QHash<QString, Constructor> instance;
    return instance;
}

QString Table::getLimitSql() const
{
    QString result, orSqlString;
    QList<Field*> orFields;
    for(Field* field : fieldChanged){
        if(field->getConditionConnectOperatorIsOr()){
            orFields.append(field);
            continue;
        }
        result += (result.isEmpty() ? " " : " and ") + field->condition();
    }
    for(const Table* tmp : innerJoin){
        for(Field* field : tmp->fieldChanged){
            if(field->getConditionConnectOperatorIsOr()){
                orFields.append(field);
                continue;
            }
            result += (result.isEmpty() ? " " : " and ") + field->condition();
        }
    }
    for(Field* field : orFields){
        orSqlString += (orSqlString.isEmpty() ? "" : " or ") + field->condition();
    }
    if(!orSqlString.isEmpty()){
        result += (result.isEmpty() ? " (" : " and (") + orSqlString + ')';
    }
    return result;
}

QSqlDatabase Table::database(bool isDelete)
{
    static QHash<QString, QSqlDatabase> hashThreadSqlDatabase;
    static QHash<QString, uint> hashThreadSqlDatabaseLastTime;
    QSqlDatabase tDatabase;
    QString threadId = QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));

    if(isDelete){
        hashThreadSqlDatabase[threadId].close();
        return tDatabase;
    }

    if(hashThreadSqlDatabase.contains(threadId)){
        tDatabase = hashThreadSqlDatabase.value(threadId);
        QSqlQuery query(tDatabase);
        query.setForwardOnly(true);
        uint &sec = hashThreadSqlDatabaseLastTime[threadId], now = QDateTime::currentDateTime().toTime_t();
        if(!tDatabase.isOpen() || (now - sec >= maxTimeOut && (!query.exec(testConnectSql) || query.lastError().type() != QSqlError::NoError))) {
            tDatabase.open();
        }
        sec = now;
    }else{
        tDatabase = QSqlDatabase::cloneDatabase(db, threadId);
        hashThreadSqlDatabase.insert(threadId, tDatabase);
        tDatabase.open();
        hashThreadSqlDatabaseLastTime.insert(threadId, QDateTime::currentDateTime().toTime_t());
    }
    Q_ASSERT_X(tDatabase.isOpen(), "Open Database", tDatabase.lastError().text().toStdString().c_str());
    return tDatabase;
}

void *Table::operator new(size_t size)
{
    callNew = true;
    return ::operator new(size);
}

Table* Table::createInstance()
{
    instance = true;
    instanceKey = tableName + '_' + (primaryKey->isString() ? primaryKey->answer().toString() : primaryKey->select());
    if(hashTable.contains(instanceKey)) return hashTable.value(instanceKey);
    hashTable.insert(instanceKey, this);
    timeStamp = QDateTime::currentDateTime().toTime_t();
    return nullptr;
}

Table *Table::searchInstance(const QString &tableName, const QString &primaryKey)
{
    QString instanceKey = tableName + '_' + primaryKey;
    if(hashTable.contains(instanceKey)) {
        Table* table = hashTable.value(instanceKey);
        if(table->isTimeout()){
            hashTable.remove(instanceKey);
            table->deleteLater();
        }else{
            return table;
        }
    }
    return nullptr;
}

void Table::setInstanceTimeout(uint second)
{
    timeStampTimeout = second;
}

void Table::destoryAllInstance()
{
    for(const QString& key : hashTable.keys()){
        delete hashTable.take(key);
    }
}

void Table::destoryAllTimeoutInstance()
{
    if(timeStampTimeout == 0) return;
    for(const QString& key : hashTable.keys()){
        Table* table = hashTable.value(key);
        if(table->isTimeout()){
            hashTable.remove(key);
            table->deleteLater();
        }
    }
}

#include <QUuid>
QString Table::DefaultStringUuid()
{
    QString uuid = QUuid::createUuid().toString();
    uuid.remove(0, 1);
    uuid.chop(1);
    return uuid;
}

QDateTime Table::DefaultCurrentDateTime()
{
    return QDateTime::currentDateTime();
}

QList<QVariantList> Table::sqlSelect(const QString &sqlString)
{
    static QList<QVariantList> None = QList<QVariantList>();
    QSqlQuery query = SqlDatabaseWriter().select(sqlString);
    if(query.isValid()){
        QList<QVariantList> result;
        int columnCount = query.record().count();
        while(query.next()){
            QVariantList row;
            for(int i = 0; i < columnCount; i++){
                row.append(query.value(i));
            }
            result.append(row);
        }
        return result;
    }
    return None;
}

bool Table::sqlexec(const QString &sqlString)
{
    return SqlDatabaseWriter().execute(sqlString);
}

Table::~Table()
{
    if(instance){
        hashTable.remove(instanceKey);
    }
}















