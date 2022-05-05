#ifndef TABLE_H
#define TABLE_H

#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QObject>
#include <QHash>
#include <QSharedPointer>
#include <QMap>
#include <QJsonDocument>
#include <QJsonObject>
#include "Field.h"

#define QY_PROPERTY_READ_CHAR READ QY_FIELD_READ_FUNCTION_CHAR
#define QY_PROPERTY_READ_SHORT READ QY_FIELD_READ_FUNCTION_SHORT

#define FIELD(type,name) Q_PROPERTY(type name READ name)\
    type name;
#define NOTNULL(type,name) FIELD(type,name)\
    Q_PROPERTY(char notnull_##name QY_PROPERTY_READ_CHAR)
#define UNIQUE(type,name) NOTNULL(type,name)\
    Q_PROPERTY(char unique_##name QY_PROPERTY_READ_CHAR)
#define UNIQUE_NULL(type,name) FIELD(type,name)\
    Q_PROPERTY(char unique_##name QY_PROPERTY_READ_CHAR)
#define PRIMARYKEY(type,name) FIELD(type,name)\
    Q_PROPERTY(char primarykey_##name QY_PROPERTY_READ_CHAR)
#define AUTOPRIMARYKEY(type,name) FIELD(type,name)\
    Q_PROPERTY(char auto_##name QY_PROPERTY_READ_CHAR)

#define FIELD_DEFAULT(type,name,value) Q_PROPERTY(type name READ name)\
    type name = value;
#define NOTNULL_DEFAULT(type,name,value) FIELD_DEFAULT(type,name,value)\
    Q_PROPERTY(char notnull_##name QY_PROPERTY_READ_CHAR)
#define UNIQUE_DEFAULT(type,name,value) NOTNULL_DEFAULT(type,name,value)\
    Q_PROPERTY(char unique_##name QY_PROPERTY_READ_CHAR)
#define UNIQUE_NULL_DEFAULT(type,name,value) FIELD_DEFAULT(type,name,value)\
    Q_PROPERTY(char unique_##name QY_PROPERTY_READ_CHAR)
#define PRIMARYKEY_DEFAULT(type,name,value) FIELD_DEFAULT(type,name, value)\
    Q_PROPERTY(char primarykey_##name QY_PROPERTY_READ_CHAR)
#define PRIMARYKEY_DEFAULT_UUID(type,name) PRIMARYKEY_DEFAULT(type,name, Qy::Table::DefaultStringUuid())

#define QY_PROPERTY_LABEL(name, labelName) \
    Q_PROPERTY(short func_##name##_##name##LABEL QY_PROPERTY_READ_SHORT)\
    Q_INVOKABLE inline void name##LABEL() { name .fieldLabel = labelName;}

#define FIELD_LABEL(type,name,labelName) FIELD(type,name)\
    QY_PROPERTY_LABEL(name, labelName)
#define NOTNULL_LABEL(type,name,labelName) NOTNULL(type,name)\
    QY_PROPERTY_LABEL(name, labelName)
#define UNIQUE_LABEL(type,name,labelName) UNIQUE(type,name)\
    QY_PROPERTY_LABEL(name, labelName)
#define UNIQUE_NULL_LABEL(type,name,labelName) UNIQUE_NULL(type,name)\
    QY_PROPERTY_LABEL(name, labelName)
#define PRIMARYKEY_LABEL(type,name,labelName) PRIMARYKEY(type,name)\
    QY_PROPERTY_LABEL(name, labelName)
#define AUTOPRIMARYKEY_LABEL(type,name,labelName) AUTOPRIMARYKEY(type,name)\
    QY_PROPERTY_LABEL(name, labelName)

#define FIELD_DEFAULT_LABEL(type,name,value,labelName)  FIELD_DEFAULT(type,name,value)\
    QY_PROPERTY_LABEL(name, labelName)
#define NOTNULL_DEFAULT_LABEL(type,name,value,labelName) NOTNULL_DEFAULT(type,name,value)\
    QY_PROPERTY_LABEL(name, labelName)
#define UNIQUE_DEFAULT_LABEL(type,name,value,labelName) UNIQUE_DEFAULT(type,name,value)\
    QY_PROPERTY_LABEL(name, labelName)
#define UNIQUE_NULL_DEFAULT_LABEL(type,name,value,labelName) UNIQUE_NULL_DEFAULT(type,name,value)\
    QY_PROPERTY_LABEL(name, labelName)
#define PRIMARYKEY_DEFAULT_LABEL(type,name,value,labelName) PRIMARYKEY_DEFAULT(type,name,value)\
    QY_PROPERTY_LABEL(name, labelName)
#define PRIMARYKEY_DEFAULT_UUID_LABEL(type,name,labelName) PRIMARYKEY_DEFAULT_UUID(type,name)\
    QY_PROPERTY_LABEL(name, labelName)

#define FIELD_VALUE_LIST(name)\
    static std::function<QStringList()> name##_value_list;\
    static std::function<QVariant(QVariant)> name##_value_list_set;\
    static std::function<QVariant(int)> name##_value_list_get;\
    Q_PROPERTY(short func_##name##_##name##VALUELIST QY_PROPERTY_READ_SHORT)\
    Q_INVOKABLE inline void name##VALUELIST() { \
    name .fieldValueList = name##_value_list; \
    name .setFieldValueList = name##_value_list_set; \
    name .getFieldValueList = name##_value_list_get;}

#define RegisterFieldValueListOnly(table, name, list)\
    std::function<QStringList()> table :: name##_value_list = list;\
    std::function<QVariant(QVariant)> table :: name##_value_list_set;\
    std::function<QVariant(int)> table :: name##_value_list_get;
#define RegisterFieldValueList(table, name, list, set, get)\
    std::function<QStringList()> table :: name##_value_list = list;\
    std::function<QVariant(QVariant)> table :: name##_value_list_set = set;\
    std::function<QVariant(int)> table :: name##_value_list_get = get;

#define TABLE(name) inline name() : Qy::Table(){initialize();}\
    TABLE_INLINE_FUNCTION(name)
#define TABLE_FUNC(name, func) inline name() : Qy::Table(){initialize(); func;}\
    TABLE_INLINE_FUNCTION(name)
#define TABLE_INLINE_FUNCTION(name) \
    inline name& setField(const QString& fieldName, const QVariant &value){setFieldValue(fieldName, value);return*this;}\
    inline name* setFieldPtr(const QString& fieldName, const QVariant &value){setFieldValue(fieldName, value);return this;}
//setField:通过字段名和值构造一个表对象,主要用于快捷查询,主要支持语法: QyOrmSelect(<TableName>().setField(<fieldName>,<value>))

#define FOREIGN(type, name, foreignName) NOTNULL(type,name)\
    Q_PROPERTY(short foreign_##name##_##foreignName QY_PROPERTY_READ_SHORT)
#define RegisterForeign(T) Qy::Table::registerTable<T>()
/*
*/
#define QyOrmSelectBase Qy::Table::selectTable
#define QyOrmSelect Qy::Table::select
#define QyOrmSelectOne Qy::Table::selectOne
#define QyOrmSelectOneSafe Qy::Table::selectOneSafe
#define QyOrmSelectPart Qy::Table::selectPart
#define QyOrmInstance(T,key) dynamic_cast<T*>(Qy::Table::searchInstance(#T, key))
class QSqlDatabase;
class QJsonObject;
typedef Qy::Table QyTable;
namespace Qy {

class Table : public QObject
{
    Q_OBJECT
public:
    explicit Table();
    void* operator new(size_t size);
    void initialize();
    virtual ~Table();
private:
    static QString parseSelectFrom(const Table* table);
    static void parseSelectSql(QSqlQuery& query, const Table* table, QStringList& area);
    static Table* parseQuery(QSqlQuery& query, Table* table, QStringList area = QStringList());
    static void parseSelectInstance(Table* &table);
public://对外暴露接口
    QVariant customVariant;
    QString customString;
    int customInteger = 0;
    bool customBoolean = false;

    template<class T>static QList<Table*> selectTable(const T& table, QStringList area = QStringList());//查询多条
    //checkInstance代表是否要将对象放进实例池,多搜默认不放，单搜默认放
    template<class T>static QList<T*> select(const T& table, bool checkInstance = false);//查询多条指定表
    template<class T>static T* selectOne(const T& table, bool checkInstance = true);//查询单条指定表
    template<class T>static QSharedPointer<T> selectOneSafe(const T& table);//查询单条指定表
    bool update(Field* field = nullptr, bool checkInstance = false);//修改,checkInstance代表是否更新实例池
    bool insert(bool checkInstance = false, bool writeNull = false);//插入,checkInstance代表是否检查和插入实例池,检查实例池存在返回false
    bool remove();//删除
    QString createSqlString();//创建表结构语句
    bool create();//创建表结构
    QJsonObject toQJson(bool completeForeign = true) const;//对象转QJSON对象
    QString toJson(bool toObject = true, bool completeForeign = true) const;//对象转JSON对象，可选false转JSON列表
    inline bool hasChanged() { return !fieldChanged.isEmpty(); }//查看是否有字段被更新
    QString changedToJson(bool toObject = true, bool completeForeign = true) const;//对象转JSON对象,只要changed字段，可选false转JSON列表
    QJsonObject fromJson(const QByteArray& jsonBytes);//从JSON字符串读取数据,并返回不在自己表字段范围的Json对象
    QJsonObject fromQJson(const QJsonObject& object);//从QJSON对象读取数据,并返回不在自己表字段范围的Json对象
    bool updateTablesField(const QStringList& primaryKeys, const QStringList& fieldNames, const QList<QStringList>& fieldValues);//批量更新表字段
    inline void clearChanged() { fieldChanged.clear(); }//清除修改记录
    void setNew(bool withoutBackstage = false);//清除所有数据,field设置为NULL
    inline void clear(bool withoutBackstage = false) { setNew(withoutBackstage); }
    bool isTimeout();

    template<class T>static QString toJson(const QList<T*>& tables, bool toObject = true, bool isDelete = true);//将表对象集合转换为JSON
    template<class T>static QList<T*> distinct(QList<T*> tables);//根据主键去重
    template<class T>static void sort(QList<T*> &tables, const QString& fieldName);//根据字段名对表集合进行排序

    static QPair<QString, QString> lastError(const QString &sqlString = QString(), const QString &errorString = QString());
    static void setLastError(const QString& sqlString, const QString& errorString);

public:
    bool isInstance();//当前是否是在实例池中的对象
    Table* searchInstance();//以当前对象作为参考去实例池搜索实例

public:
    void setFieldValue(const QString& fieldName, const QVariant &value);
    QVariant getFieldValue(const QString& fieldName);
    typedef QList<Table*> (*Constructor)(const QVariant& var);
    template<typename T>static bool registerTable()
    { constructors().insert( T::staticMetaObject.className(), &constructorHelper<T> );return true; }
    static QList<Table*> createTable(const QString& className, const QVariant& var);
    template<typename T>static QList<Table*> constructorHelper(const QVariant& var)
    {if(var.isNull())return{new T()};T t;t.primaryKey->query(var);return selectTable<T>(t); }
    static QHash<QString, Constructor>& constructors();
    bool foreignInited = false;
    QString getLimitSql() const;//将设置了值的字段拼接,获取条件限定语句
public:
    static QString testConnectSql;
    static QSqlDatabase db;
    static QSqlDatabase database(bool isDelete = false);
    static uint maxTimeOut;//(单位:秒),此次数据库连接与上一次使用相隔时间,超过便主动测试一下,默认1800s
    Field* primaryKey = nullptr;
    QString tableName;
    QString tableLabel;
    QList<Field*> fieldChanged;
public:
    QStringList listFieldName;
    QList<Field*> listField;
    QHash<QString, Field*> hashField;
    QHash<QString, QString> hashForeign;
protected:
    static QHash<QString, QHash<QString, Field*>> hashClassNameHashFieldReady;//针对每一个class对第一次的hashField进行备份,之后的直接拷贝使用
    static QHash<QString, QStringList> hashClassNameListFieldNameReady;//针对每一个class对第一次的listFieldName进行备份,之后的直接拷贝使用
    static QHash<QString, Table*> hashTable;//对象唯一
    QString instanceKey;
    static uint timeStampTimeout;//实例过期时间,默认1800秒(半小时),设置为0不过期
    uint timeStamp = 0;
    Table *createInstance();
    bool atHeap = false;//是否是在堆上
    bool instance = false;//是否是数据库的实际值
public:
    static Table* searchInstance(const QString& tableName, const QString& primaryKey);
    static void setInstanceTimeout(uint second);
    static void destoryAllInstance();
    static void destoryAllTimeoutInstance();
    static QString DefaultStringUuid();
    static QDateTime DefaultCurrentDateTime();

public://特殊查询
    bool isPart = false;//是否是只查询了部分字段
    QStringList area;//保存限定查询了哪些字段的集合
    QString specialSql;//自定义主语句和where语句中间的SQL语句
    QString whereSql;//自定义where字段
    QList<Table*> innerJoin;//联合查询
    inline void clearInner(){for(Table* t:innerJoin)delete t;}
    QString innerJoinFieldName;//表示联合的表要联合的字段
    inline Table* setInnerJoin(const QString& fieldName = QString()){//为空就默认为主键
        this->innerJoinFieldName = (fieldName.isEmpty() ? this->primaryKey->fieldName : fieldName);
        return this;
    }
    static QHash<QString, QStringList> hashPartSelect;
    template<class T>static QList<Table*> selectPart(const QString&, const T& table);

public://提供直接sql执行支持
    static QList<QVariantList> sqlSelect(const QString& sqlString);
    static bool sqlexec(const QString& sqlString);
public:
    static inline char QY_FIELD_READ_FUNCTION_CHAR(){return '\0';}
    static inline short QY_FIELD_READ_FUNCTION_SHORT(){return 0;}
};

template<class T>
QString Table::toJson(const QList<T*>& tables, bool toObject, bool isDelete)
{
    static_assert(std::is_base_of<Table, T>::value, "T must base of Table!");

    QString result = "[";
    for(const Table* table : tables){
        result += table->toJson(toObject) + ',';
        if(isDelete){
            delete table;
        }
    }
    if(!tables.isEmpty()){ result.chop(1); result.replace('\n', ""); }
    result += ']';
    return result;
}

template<class T>
QList<T*> Table::distinct(QList<T*> tables)
{
    static_assert(std::is_base_of<Table, T>::value, "T must base of Table!");

    QSet<QString> qset;
    for(int i = tables.size() - 1; i >= 0; --i){
        const QString key = tables.at(i)->primaryKey->select();
        if(qset.contains(key)){
            tables.removeAt(i);
        }else{
            qset.insert(key);
        }
    }
    return tables;
}

template<class T>
void Table::sort(QList<T*>& tables, const QString &fieldName)
{
    static_assert(std::is_base_of<Table, T>::value, "T must base of Table!");

    if(tables.size() > 1 && tables.first()->hashField.contains(fieldName)){
        std::sort(tables.begin(), tables.end(), [fieldName](Table* table1, Table* table2){
            return (table1->getFieldValue(fieldName) < table2->getFieldValue(fieldName));
        });
    }
}

template<class T>
QList<Table *> Table::selectTable(const T& table, QStringList area)
{
    static_assert(std::is_base_of<Table, T>::value, "T must base of Table!");

    QList<Table *> result;
    QSqlQuery query(database());
    parseSelectSql(query, &table, area);
    query.setForwardOnly(true);
    if(query.exec()){
        while (query.next()) {
            result.append(parseQuery(query, new T(), area));
        }
    }else{
        Table::setLastError(query.lastQuery(), query.lastError().text());
    }
    return result;
}

template<class T>
QList<T *> Table::select(const T& table, bool checkInstance)
{
    static_assert(std::is_base_of<Table, T>::value, "T must base of Table!");

    QList<T*> result;
    for(Table* t : selectTable(table)){
        if(checkInstance) parseSelectInstance(t);
        result.append(dynamic_cast<T*>(t));
    }
    return result;
}

template<class T>
T *Table::selectOne(const T &table, bool checkInstance)
{
    static_assert(std::is_base_of<Table, T>::value, "T must base of Table!");

    QList<T*> result = select(table, checkInstance);
    while(result.size() > 1) delete result.takeLast();
    if(result.size() > 0){
        return result.first();
    }else{
        return nullptr;
    }
}

template<class T>
QSharedPointer<T> Table::selectOneSafe(const T &table)
{
    static_assert(std::is_base_of<Table, T>::value, "T must base of Table!");

    return QSharedPointer<T>(selectOne(table, false));
}

template<class T>
QList<Table*> Table::selectPart(const QString & partKey, const T &table)
{
    static_assert(std::is_base_of<Table, T>::value, "T must base of Table!");

    static QList<Table*> None = QList<Table*>();
    const QStringList& partField = hashPartSelect.value(partKey);
    if(partField.isEmpty()) return None;

    return selectTable(table, partField);
}

}

#endif // TABLE_H
