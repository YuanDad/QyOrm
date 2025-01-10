#ifndef QYORMDYNAMICTABLE_H
#define QYORMDYNAMICTABLE_H

#include "QyOrmTable.h"
#include "QyOrmCompleteTable.h"

#define QYORM_TABLE_DYNAMIC_FIELD(TABLE) \
    explicit inline TABLE(QObject *parent = nullptr) : QyOrmDynamicFieldInfo(parent){ __initialize(); } \
    QYORM_TABLE_INLINE_FUNCTION(TABLE)
#define QYORM_TABLE_DYNAMIC_FIELD_FUNC(TABLE, func) \
    explicit inline TABLE(QObject *parent = nullptr) : QyOrmDynamicFieldInfo(parent){__initialize(); func ();} \
    QYORM_TABLE_INLINE_FUNCTION(TABLE)

class QyOrmDynamicFieldInfo : public QyOrmTable
{
    Q_OBJECT
public:
    explicit inline QyOrmDynamicFieldInfo(QObject *parent = nullptr) : QyOrmTable(parent){}

    //主键和字段名不可更改
    QYORM_FIELD(QyOrmStringField, fieldName, QyOrmFieldConstraint::PrimaryKey);//字段名
    QYORM_FIELD(QyOrmStringField, fieldTableName, QyOrmFieldConstraint::Plain);//属于的表

    QYORM_FIELD(QyOrmStringField, fieldLabel, QyOrmFieldConstraint::Plain);//字段名称
    QYORM_FIELD_DEFAULT(QyOrmIntegerField, fieldType, 0, QyOrmFieldConstraint::Plain);//字段类型
    QYORM_FIELD_DEFAULT(QyOrmBooleanField, fieldUnique, false, QyOrmFieldConstraint::Plain);//唯一
    QYORM_FIELD_DEFAULT(QyOrmBooleanField, fieldNotnull, false, QyOrmFieldConstraint::Plain);//非空

public:
    QString sqlAddField();
    QString sqlDelField();

public:
    QyOrmFieldExtraInfo* createExtraInfo();
    void update(QyOrmFieldExtraInfo*);
    void setDefaultFieldName();
    inline void setDefaultName() { return setDefaultFieldName(); }

protected:
    virtual QString getTypeString();
};

class QyOrmDynamicTableMaker;
class QyOrmDynamicTableTemplate: public QyOrmTable
{
    Q_OBJECT
public:
    virtual QyOrmTable* constructor(QObject *) { return nullptr; }
    QYORM_TABLE_INLINE_FUNCTION_EXTRA(QyOrmDynamicTableTemplate);
    explicit inline QyOrmDynamicTableTemplate(const QString& _tableName, QObject *parent = nullptr):
        QyOrmTable(parent){ __dynamicTableName = _tableName; __initialize(); }
    explicit inline QyOrmDynamicTableTemplate(QObject *parent, const QString& _tableName):
        QyOrmTable(parent){ __dynamicTableName = _tableName; }
    QYORM_FIELD(QyOrmStringField, qyid, QyOrmFieldConstraint::PrimaryKey);
    QyOrmDynamicTableMaker* maker = nullptr;
};

class QyOrmDynamicTableMakerPrivated{
public:
    QString dynamicTableName;
    QList<QyOrmFieldExtraInfo*> extraInfos;
};
class QyOrmDynamicTableMaker : public QObject
{
    Q_OBJECT
public:
    explicit inline QyOrmDynamicTableMaker(const QString& _tableName, QObject *parent = nullptr):
        QObject(parent), __privated(new QyOrmDynamicTableMakerPrivated){ __privated->dynamicTableName = _tableName; }
    ~QyOrmDynamicTableMaker();

    bool isEmpty();
    void setFieldInfos(QList<QyOrmDynamicFieldInfo*> fieldInfos);
    void appendFieldInfo(QyOrmDynamicFieldInfo* fieldInfo);
    void completeDynamicTable(QyOrmDynamicTableTemplate*);
    QyOrmDynamicTableTemplate* createDynamicTable();
    template<class Type> inline Type* createDynamicTable(){
        Type* table = new Type(__privated->dynamicTableName);
        completeDynamicTable(table);
        table->maker = this;
        return table;
    }

private:
    friend class QyOrmDynamicTableMakerPrivated;
    QyOrmDynamicTableMakerPrivated* __privated = nullptr;
};

#endif // QYORMDYNAMICTABLE_H
