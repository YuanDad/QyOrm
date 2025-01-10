#ifndef QYORMTABLE_H
#define QYORMTABLE_H

#include <QPointer>
#include <QObject>
#include <QVariant>
#include <functional>
#include "QyOrmField.h"
#include "QyOrmTableDefine.h"

class QyOrmTable : public QObject
{
    Q_OBJECT
public:
    explicit QyOrmTable(QObject *parent = nullptr);
    virtual QyOrmTable* constructor(QObject* = nullptr) { return nullptr; }
    QyOrmTable* copy();
    void* operator new(size_t size);
    bool atHeap();
    inline qulonglong toULongLong() { return reinterpret_cast<qulonglong>(this); }
    inline qulonglong toPtr() { return reinterpret_cast<qulonglong>(this); }
    ~QyOrmTable() override;

    inline QyOrmTable& operator=(const QyOrmTable& other) { return *QyOrmTable::operator=(&other); }
    QyOrmTable* operator=(const QyOrmTable* other);

public:
    QString tableName();
    QString tableLabel();
    QyOrmField* primaryKey();
    const QStringList& listFieldName();
    const QList<QyOrmField*>& listField();
    const QList<QyOrmField*>& listDynamicField();
    QyOrmField* field(const QString& fieldName);
    QyOrmField* at(int index);
    void assigning();//设置状态: 系统正在对字段进行赋值,不触发修改事件
    void assigned();//取消状态
    QList<QyOrmField *> recordChangedFields();//获取记录下来的修改字段列表
    QList<QyOrmField *> scanChangedFields();//扫描所有字段获取修改了的字段列表
    bool hasScanChanged();
    bool isRecordChanged(const QString& fieldName);

public:
    inline QString name() { return tableName(); }
    inline QString label() { return tableLabel(); }
    inline bool contains(const QString& fieldName) { return field(fieldName); }
    inline QyOrmField* operator [](const QString& fieldName) { return field(fieldName); }
    inline QyOrmField* operator [](int index) { return at(index); }
    inline const QStringList& fieldsName() { return listFieldName(); }
    inline const QList<QyOrmField*>& fields() { return listField(); }
    inline bool hasRecordChanged() { return !recordChangedFields().isEmpty(); }
    inline bool isRecordChanged(QyOrmField* field) { return isRecordChanged(field->fieldName()); }

public:
    QVariant getFieldValue(const QString& fieldName);
    QyOrmTable& setFieldValue(const QString& fieldName, const QVariant &value);
    QVariant getFieldValue(int index);
    QyOrmTable& setFieldValue(int index, const QVariant &value);
    void clearRecordChanged(QyOrmField* field = nullptr);
    void clearScanChanged();
    inline void clearChanged() { clearRecordChanged(); clearScanChanged(); }

public:
    QString toJson(bool isArray = true, const QString& fieldName = QString(),
                   const QStringList& excludes = QStringList(), int writeNull = -1) const;
    static QString toJson(const QList<QyOrmTable*>& tables,
                          bool isArray = true, bool autoRemove = true);
    template <class Type>
    static QString toJson(const QList<Type*>& tables, bool isArray = true, bool autoRemove = true){
        if(tables.isEmpty()) return QString();
        QString result;
        foreach(const Type* table, tables){
            result.append(table->toJson(isArray));
            result.append(',');
        }
        if(autoRemove) qDeleteAll(tables);
        result.chop(1);
        return '[' + result + ']';
    }

    void fromJson(const QByteArray& jsonBytes);
    void fromQJson(const QJsonObject& object);
    void fromQJson(const QJsonArray& array);
    void fromQJson(const QJsonArray& headers, const QJsonArray& array);
    void fromQJson(const QStringList& headers, const QJsonArray& array);
    bool fromVariant(const QString& fieldName, const QVariant& data);

public:
    inline QString toObjectJson(const QString& fieldName = QString(),
                                const QStringList& excludes = QStringList(), bool writeNull = false) const
    { return toJson(false, fieldName, excludes, writeNull); }
    inline QString toArrayJson(const QString& fieldName = QString(),
                               const QStringList& excludes = QStringList(), bool writeNull = true) const
    { return toJson(true, fieldName, excludes, writeNull); }

public:
    virtual QString sqlCreateFieldType(QyOrmFieldType type);
    QString sqlCreateTableString(const QString& other = QString(), bool enableConstraint = true);
    QString sqlDropTableString();
    void addOrderBy(QyOrmField& field, bool desc = false, int castNumber = 0);
    QyOrmTable& addOrderBy(const QString& fieldName = "qyid", bool desc = false, int castNumber = 0);
    bool hasOrderBy();
    void invertOrderBy();
    void copyOrderBy(QyOrmTable* source);
    void setLimit(int limit, int offset = 0);
    bool appendSelectField(const QString& fieldName);
    void appendSelectField(QyOrmField* field);
    inline void appendSelectField(QyOrmField& field)
    { appendSelectField(&field); }
    QString sqlSelectSimpleString() const;
    QString _sqlSelectJoinOnString() const;
    QString sqlSelectAllFieldString() const;
    QString sqlSelectCountString() const;
    QString sqlSelectSelfByFieldString(QyOrmField* field) const;
    QString sqlSelectSelfByFieldsString(QList<QyOrmField*> fields) const;
    QString sqlSelectSelfString() const;
    QString sqlSelectFieldString(const QString& fieldName) const;
    void setSeniorSelectSql(const QString& seniorSelectSql);
    void appendSeniorSelectSql(const QString& seniorSelectSql);
    void setForceSelectSql(const QString& forceSelectSql);
    QString sqlSelectWhere(const QString& customWhere = QString()) const;
    QString sqlSelectOrderBy() const;
    QString sqlLimit() const;
    inline QString sqlSelectWhereOrderBy(const QString& customWhere = QString()) const
    { return sqlSelectWhere(customWhere) + sqlSelectOrderBy(); }
    inline QString sqlSelectWhereOrderByLimit(const QString& customWhere = QString()) const
    { return sqlSelectWhere(customWhere) + sqlSelectOrderBy() + sqlLimit(); }
    inline QString sqlSelectAllString(const QString& customWhere = QString()) const
    { return sqlSelectAllFieldString() + sqlSelectWhereOrderByLimit(customWhere); }
    void setUpdateExtraWhere(const QString& where);
    QString sqlUpdateString(const QList<QyOrmField *> &listField) const;
    QString sqlUpdateChangedString() const;
    QString sqlUpdateFieldString(const QString& fieldName) const;
    QString sqlUpdateFieldString(QyOrmField* field) const;
    inline QString sqlUpdateFieldString(QyOrmField& field) const
    { return sqlUpdateFieldString(&field); }
    QString sqlUpdateAllFieldString() const;
    QString __sqlInsertString(const QList<QyOrmField *> &listField) const;
    QString sqlInsertNotNullString() const;
    QString sqlInsertChangedString() const;
    QString sqlInsertAllFieldString() const;
    inline QString sqlInsertString() const { return sqlInsertAllFieldString(); }
    QString sqlDeleteString() const;
    QString sqlDeleteForChangedString() const;
    QString sqlDeleteForValueString() const;

public:
    using QOFKC = QyOrmForeignKeyConstraint;
    QString sqlForeignKeyString(const QString& fieldName, const QString& fkTableName, const QString& fkFieldName,
                                QOFKC onDelete = QOFKC::Miss, QOFKC onUpdate = QOFKC::Miss) const;
    inline QString sqlForeignKeyString(QyOrmField& field, const QString& fkTableName, const QString& fkFieldName,
                                       QOFKC onDelete = QOFKC::SetNull, QOFKC onUpdate = QOFKC::Cascade)
    { return sqlForeignKeyString(field.fieldName(), fkTableName, fkFieldName, onDelete, onUpdate); }
    inline QString sqlForeignKeyString(QyOrmField& field, QyOrmTable* fkTable,
                                QOFKC onDelete = QOFKC::Miss, QOFKC onUpdate = QOFKC::Miss){
        return sqlForeignKeyString(field.fieldName(), fkTable->tableName(),
                                   fkTable->primaryKey()->fieldName(), onDelete, onUpdate);
    }

#ifdef QY_ORM_DATABASE
public:
    static thread_local QString selectError;
    static QString sqlDatabaseExecute(const QString& sql, QList<QyOrmBlobField*> blobs = QList<QyOrmBlobField*>());
    static QList<QVariantList> sqlDatabaseSelectAll(const QString& sql, QString& error = selectError,
                                                    bool withHeader = false, bool* canRun = nullptr);
    static QList<QVariantList> sqlDatabaseSelectRowWithHeader(const QString& sql, QString& error = selectError);
    static QVariantList sqlDatabaseSelectRow(const QString& sql, QString& error = selectError);
    static QVariantList sqlDatabaseSelectColumn(const QString &sql, QString &error = selectError);
    static QVariant sqlDatabaseSelectOne(const QString& sql, QString& error = selectError);

public:
    QString getLastError();
    bool sqlExecute(const QString& sql);
    inline bool sqlCreateTable(const QString& other = QString(), bool enableConstraint = true)
    { return sqlExecute(sqlCreateTableString(other, enableConstraint)); }
    inline bool sqlDropTable() { return sqlExecute(sqlDropTableString()); }
    inline bool sqlUpdate(const QList<QyOrmField *> &listField) { return sqlExecute(sqlUpdateString(listField)); }
    inline bool sqlUpdateChanged() { return sqlExecute(sqlUpdateChangedString()); }
    inline bool sqlUpdateField(const QString& fieldName) { return sqlExecute(sqlUpdateFieldString(fieldName)); }
    inline bool sqlUpdateField(QyOrmField* field) { return sqlExecute(sqlUpdateFieldString(field)); }
    inline bool sqlUpdateField(QyOrmField& field) { return sqlExecute(sqlUpdateFieldString(&field)); }
    inline bool sqlUpdateAllField() { return sqlExecute(sqlUpdateAllFieldString()); }
    inline bool sqlInsertNotNull() { return sqlExecute(sqlInsertNotNullString()); }
    inline bool sqlInsertChanged() { return sqlExecute(sqlInsertChangedString()); }
    inline bool sqlInsertAllField() { return sqlExecute(sqlInsertAllFieldString()); }
    inline bool sqlInsert() { return sqlExecute(sqlInsertString()); }
    inline bool sqlDelete() { return sqlExecute(sqlDeleteString()); }
    inline bool sqlDeleteForChanged() { return sqlExecute(sqlDeleteForChangedString()); }
    inline bool sqlDeleteForValue() { return sqlExecute(sqlDeleteForValueString()); }

    bool sqlSelectSelfByField(QyOrmField* field);//根据一个字段更新自己
    inline bool sqlSelectSelfByField(QyOrmField& field)//根据一个字段更新自己
    { return sqlSelectSelfByField(&field); }
    bool sqlSelectSelfByFields(QList<QyOrmField*> fields);//根据多个字段更新自己
    bool sqlSelectSelfByChangedFields();//根据多个字段更新自己
    bool sqlSelectSelf();//根据主键更新自己
    bool sqlSelectSelfField(QyOrmField* field);//根据主键更新自己的一个字段
    inline bool sqlSelectSelfField(QyOrmField& field)//根据主键更新自己的一个字段
    { return sqlSelectSelfField(&field); }
    inline bool sqlSelectSelfFieldName(const QString& fieldName)//根据主键更新自己的一个字段
    { return sqlSelectSelfField(field(fieldName)); }
    bool sqlSelectSelfFields(QList<QyOrmField*> fields);//根据主键更新自己的多个字段
    bool sqlSelectSelfFieldNames(QStringList fieldNames);//根据主键更新自己的多个字段
    QString getLastSelectError();//获取最后一条查询报错
    //disorder 是否是混乱顺序, *就是混乱的/按照fieldName就是false
    QList<QyOrmTable*> sqlSelectAllFor(const QString& selectSql, bool disorder = true);//通过自定义sql语句查询,查询的字段仅限一个表
    inline QList<QyOrmTable*> sqlSelectAllSimple()//通过select*来查询
    { return sqlSelectAllFor(sqlSelectSimpleString() + sqlSelectWhereOrderByLimit(), false); }
    QList<QyOrmTable*> sqlSelectAll(const QString& customWhere = QString());//通过定义的所有字段来查询
    QyOrmTable* sqlSelectOne();//只查询一个
    QyOrmTable* sqlSelectPrevious(const QString& fieldName, bool toNumber = false, QVariant data = QVariant());//查询上一个
    inline QyOrmTable* sqlSelectPrevious(QyOrmField* field, bool toNumber = false, QVariant data = QVariant())//查询上一个
    {return sqlSelectPrevious(field->fieldName(), toNumber, data);}
    inline QyOrmTable* sqlSelectPrevious(QyOrmField& field, bool toNumber = false, QVariant data = QVariant())//查询上一个
    {return sqlSelectPrevious(field.fieldName(), toNumber, data);}
    QyOrmTable* sqlSelectNext(const QString& fieldName, bool toNumber = false, QVariant data = QVariant());//查询下一个
    inline QyOrmTable* sqlSelectNext(QyOrmField* field, bool toNumber = false, QVariant data = QVariant())//查询下一个
    {return sqlSelectNext(field->fieldName(), toNumber, data);}
    inline QyOrmTable* sqlSelectNext(QyOrmField& field, bool toNumber = false, QVariant data = QVariant())//查询下一个
    {return sqlSelectNext(field.fieldName(), toNumber, data);}
    QyOrmTable* sqlSelectMax(const QString& fieldName, bool toNumber = false);//查询最大的那个
    inline QyOrmTable* sqlSelectMax(QyOrmField* field, bool toNumber = false)//查询最大的那个
    {return sqlSelectMax(field->fieldName(), toNumber);}
    inline QyOrmTable* sqlSelectMax(QyOrmField& field, bool toNumber = false)//查询最大的那个
    {return sqlSelectMax(field.fieldName(), toNumber);}
    QyOrmTable* sqlSelectMin(const QString& fieldName, bool toNumber = false);//查询最小的那个
    inline QyOrmTable* sqlSelectMin(QyOrmField* field, bool toNumber = false)//查询最小的那个
    {return sqlSelectMin(field->fieldName(), toNumber);}
    inline QyOrmTable* sqlSelectMin(QyOrmField& field, bool toNumber = false)//查询最小的那个
    {return sqlSelectMin(field.fieldName(), toNumber);}
    bool sqlSelectUnique();//将无序的更改记录字段集中第一个unique字段用来判断是否数据库中数据唯一
    QVariant sqlSelectFieldData(const QString& fieldName);
    inline QString sqlSelectField(const QString& fieldName)
    { return sqlSelectFieldData(fieldName).toString(); }
    QList<QPointer<QyOrmTable> > sqlSelectAllSafe();
    QList<QPointer<QyOrmTable> > sqlSelectAllSimpleSafe();
    QPointer<QyOrmTable> sqlSelectOneSafe();

    bool sqlSelectAllFor(QList<QyOrmTable*>& tables, const QString& selectSql, bool disorder = true);
    bool sqlSelectAll(QList<QyOrmTable*>& tables);
    int sqlSelectCount() const;

    //模板用于将QyOrmTable转化成特定的子类
    template<class Type>QList<Type*> sqlSelectAllFor(const QString& selectSql, bool disorder = true){
        QList<Type*> result;
        foreach(QyOrmTable* table, sqlSelectAllFor(selectSql, disorder)){
            result.append(table->dynamicCast<Type>());
        }
        return result;
    }
    template<class Type>QList<Type*> sqlSelectAll(){
        QList<Type*> result;
        foreach(QyOrmTable* table, sqlSelectAll()){
            result.append(table->dynamicCast<Type>());
        }
        return result;
    }
    template<class Type>QList<Type*> sqlSelectAllSimple(){
        QList<Type*> result;
        foreach(QyOrmTable* table, sqlSelectAllSimple()){
            result.append(table->dynamicCast<Type>());
        }
        return result;
    }
    template<class Type>inline Type* sqlSelectOne(){
        QyOrmTable* table = sqlSelectOne();
        if(table) return table->dynamicCast<Type>();
        return nullptr;
    }
    template<class Type>QList<QPointer<Type> > sqlSelectAllSafe(){
        QList<QPointer<Type>> result;
        for(Type* table : sqlSelectAll<Type>()){
            result.append(table);
        }
        return result;
    }
    template<class Type>QList<QPointer<Type> > sqlSelectAllSimpleSafe(){
        QList<QPointer<Type>> result;
        for(Type* table : sqlSelectAllSimple<Type>()){
            result.append(table);
        }
        return result;
    }
    template<class Type>inline QPointer<Type> sqlSelectOneSafe(){ return sqlSelectOne<Type>(); }

    template<class Type>bool sqlSelectAllFor
    (QList<Type*>& tables, const QString& selectSql, bool disorder = true){
        QList<QyOrmTable*> simpleTables;
        foreach(Type* table, tables) simpleTables.append(table);
        bool ret = sqlSelectAllFor(simpleTables, selectSql, disorder);
        tables.clear();
        foreach(QyOrmTable* table, simpleTables) tables.append(table->dynamicCast<Type>());
        return ret;
    }
    template<class Type>bool sqlSelectAll(QList<Type*>& tables){
        QList<QyOrmTable*> simpleTables;
        foreach(Type* table, tables) simpleTables.append(table);
        bool ret = sqlSelectAll(simpleTables);
        tables.clear();
        foreach(QyOrmTable* table, simpleTables) tables.append(table->dynamicCast<Type>());
        return ret;
    }
#else
    template<class Type>QList<Type*> sqlSelectAll(){
        return QList<Type*>();
    }
    template<class Type>bool sqlSelectAll(QList<Type*>& tables){
        Q_UNUSED(tables)
        return false;
    }
    template<class Type>QList<QPointer<Type> > sqlSelectAllSafe(){
        return QList<QPointer<Type>>();
    }
    template<class Type>inline Type* sqlSelectOne(){
        return nullptr;
    }
    template<class Type>inline QPointer<Type> sqlSelectOneSafe(){ return nullptr; }
#endif

public:
    template<class Type>inline Type* dynamicCast(){ return dynamic_cast<Type*>(this); }

protected:
    friend class QyOrmField;
    friend class QyOrmCompleteField;
    friend class QyOrmSelectGenerator;
    QyOrmTablePrivate* __privated = nullptr;
    QString __dynamicTableName;
    virtual void __initialize();//必须在继承类的构造函数中执行
};

#endif // QYORMTABLE_H
