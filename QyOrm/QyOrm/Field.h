#ifndef FIELD_H
#define FIELD_H

#include <QObject>
#include <QDateTime>
#include <QVariant>
#include <QMetaType>
#include <functional>

#include <QDebug>
#ifndef DP
    #ifdef Q_OS_WIN
        #define DP qDebug()
    #else
        #define DP qWarning()
    #endif
    #define DF DP << __FUNCTION__
#endif

namespace Qy {

class Table;

enum class FieldType{Foreign, String, Integer, Double, DateTime, Boolean, Text, Blob};

class Field : public QObject
{
    Q_OBJECT
public:
    inline Field() : QObject(nullptr), self(this){}
    inline Field(const Field& field) : QObject(nullptr), self(field.self){}
    QString createTemplate();
    inline void setTable(Table* table) { this->table = table; }

public:
    inline void setData(Field* field) { query(field->answer()); }//通过同类型的字段更新值
    QString toJson(bool completeForeign = true);//根据不同类型转换成Json字符串,参数代表外键取PrimaryKey值还是取表Json值
    QJsonValue toQJson(bool completeForeign = true);//根据不同类型转换成QJson对象,参数代表外键取PrimaryKey值还是取表Json值
    inline void setConditionOperator(const QString& _operator) { conditionOperator = _operator; }//设置条件语句运算符
    inline void setConditionConnectOperatorIsOr() { conditionConnectOperator = true; }//设置条件语句连接运算符为or
    inline bool getConditionConnectOperatorIsOr() { return conditionConnectOperator; }//判断条件语句连接运算符为or
    inline void setConditionOperatorWithLike() { conditionOperator = "like"; }//设置条件语句运算符为like
    inline void setBetween(const QVariant& var) { betweenValue = var; }//设置为 between 条件语句,参数为设置第二个操作数，自身的值为第一个
    inline void setRange(const QVariantList& var) { rangeValues = var; }//设置为 in 条件语句,参数为in的列表操作数
    QString condition(bool needTableDot = true);//获取条件语句
    inline QString getLabel() { return (fieldLabel.isEmpty() ? fieldName : fieldLabel); }//获取字段用FIELD_LABEL设置的label
    inline bool hasValueList() { return bool(fieldValueList); }//是否是列表项
    inline bool canHandleValueList() { return (getFieldValueList && setFieldValueList); }//是否有显示/值转换

public:
    inline QString selectCheckNull(const QString& data) { if(isNull) return "NULL"; else return data; }
    virtual QString select(bool checkNull = true) = 0;//返回实际值的字符串值
    virtual QString createType() = 0;//返回该类型的数据库字段类型,如varchar(255)
    virtual QString create();//返回创建该字段的sql语句段
    virtual QString execute(bool needTableDot = true);//用于update赋值语句
    virtual bool isString();//是否是字符串类型
    virtual void query(const QVariant& var) = 0;//通过QVariant更新值
    virtual QVariant answer() = 0;//返回实际值的QVariant值
    virtual void clear() = 0;//清空数据，设置为null
public:
    Table* table = nullptr;
    QString fieldName;
    bool autoPrimaryKey = false;
    bool primaryKey = false;
    bool unique = false;
    bool notNull = false;
    Field* self = nullptr;
    FieldType fieldType;
    QString conditionOperator = "=";
    bool conditionConnectOperator = false;
    QVariant betweenValue;
    QVariantList rangeValues;
    bool isBackstage = false;
    QVariant customVariant;
    QString customString;
public://moc定义的inline初始化
    QString fieldLabel;//FIELD_LABEL
    std::function<QStringList()> fieldValueList;
    std::function<QVariant(QVariant)> setFieldValueList;
    std::function<QVariant(int)> getFieldValueList;
public:
    bool isNull = true;
signals:
    void modify();//Table是instance才会再更新后发出此信号
};

class ForeignField: public Field
{
public:
    inline ForeignField(Table* data): data(data){ fieldType = FieldType::Foreign; isNull = false; }
    inline ForeignField() { fieldType = FieldType::Foreign; }
    QString select(bool checkNull = true) override;
    QString createType() override;
    void query(const QVariant& var) override;
    QVariant answer() override;
    void clear() override;
public:
    inline Table* get() const { if(isNull) return nullptr; return data; }
    template<class T>T* get();
    bool set(Table *value, bool isWild = false);//isWild-表示value是否需要指定this为value的parent
public:
    inline ForeignField& operator() () { return *this; }
    inline operator Table* () { return data; }
    ForeignField& operator=(const ForeignField&);
    ForeignField& operator=(Table* value);
private:
    Table* data = nullptr;
};

template<class T>
T *ForeignField::get()
{
    return dynamic_cast<T*>(get());
}

class StringField : public Field
{
public:
    inline StringField(const char* data): data(QString(data)){ fieldType = FieldType::String; isNull = false; }
    inline StringField(const QString& data): data(data){ fieldType = FieldType::String; isNull = false; }
    inline StringField() { fieldType = FieldType::String; }
    QString select(bool checkNull = true);
    QString createType();
    void query(const QVariant& var);//调用 set() ,触发 fieldChanged
    QVariant answer();
    void clear();
    bool isString();
public:
    inline QString get() const { if(isNull) return QString(); return data; }
    bool set(const QString &value);//触发 fieldChanged
public:
    inline StringField& operator() () { return *this; }
    inline operator QString () { return data; }
    inline StringField& operator=(const char* cs) { this->operator=(QString(cs)); return *this; }//直接赋值 data ,不触发 fieldChanged
    StringField& operator=(const StringField&);//直接赋值 data ,不触发 fieldChanged
    StringField& operator=(const QString&);//调用 set() ,触发 fieldChanged
private:
    QString data = "";
};

class IntegerField : public Field
{
public:
    inline IntegerField(const int32_t& data): data(data){ fieldType = FieldType::Integer; isNull = false; }
    inline IntegerField() { fieldType = FieldType::Integer; }
    QString select(bool checkNull = true);
    QString createType();
    void query(const QVariant& var);
    QVariant answer();
    void clear();
public:
    inline int32_t get() const { if(isNull) return 0; return data; }
    bool set(const int32_t &value);
public:
    inline IntegerField& operator() () { return *this; }
    inline operator int () { return data; }
    IntegerField& operator=(const IntegerField&);
    IntegerField& operator=(const int32_t&);
private:
    int32_t data = 0;
};

class DoubleField : public Field
{
public:
    inline DoubleField(const double& data): data(data){ fieldType = FieldType::Double; isNull = false; }
    inline DoubleField() { fieldType = FieldType::Double; }
    QString select(bool checkNull = true);
    QString createType();
    void query(const QVariant& var);
    QVariant answer();
    void clear();
public:
    inline double get() const { if(isNull) return 0.0; return data; }
    bool set(const double &value);
public:
    inline DoubleField& operator() () { return *this; }
    inline operator double () { return data; }
    DoubleField& operator=(const DoubleField&);
    DoubleField& operator=(const double&);
private:
    double data = 0.0;
};

class DateTimeField : public Field
{
public:
    inline DateTimeField(const QDateTime& data): data(data){ fieldType = FieldType::DateTime; isNull = false; }
    inline DateTimeField() { fieldType = FieldType::DateTime; }
    QString select(bool checkNull = true);
    QString createType();
    void query(const QVariant& var);
    QVariant answer();
    void clear();
    inline QString toString() { return data.toString(getValidFormat()); }
    inline QString getValidFormat() { return (date_format.isNull() ? y_datetime_format : date_format); }
    inline void setLocalFormat(const QString& formatString) { date_format = formatString; }
    inline QString getLocalFormat() { return date_format; }
    static inline void setFormat(const QString& formatString) { y_datetime_format = formatString; }
    static inline QString getFormat() { return y_datetime_format; }
public:
    inline QDateTime get() const { if(isNull) return QDateTime(); return data; }
    bool set(const QDateTime &value);
public:
    inline DateTimeField& operator() () { return *this; }
    inline operator QDateTime () { return data; }
    DateTimeField& operator=(const DateTimeField&);
    DateTimeField& operator=(const QDateTime&);
private:
    QDateTime data;
    QString date_format;
    static QString y_datetime_format;
};

class BooleanField : public Field
{
public:
    inline BooleanField(const bool& data): data(data){ fieldType = FieldType::Boolean; isNull = false; }
    inline BooleanField() { fieldType = FieldType::Boolean; }
    QString select(bool checkNull = true);
    QString createType();
    void query(const QVariant& var);
    QVariant answer();
    void clear();
public:
    inline bool get() const { if(isNull) return false; return data; }
    bool set(const bool &value);
public:
    inline BooleanField& operator() () { return *this; }
    inline operator bool () { return data; }
    BooleanField& operator=(const BooleanField&);
    BooleanField& operator=(const bool&);
private:
    bool data = false;
};

class TextField : public Field
{
public:
    inline TextField(const QString& data): data(data){ fieldType = FieldType::Text; isNull = false; }
    inline TextField() { fieldType = FieldType::Text; }
    QString select(bool checkNull = true);
    QString createType();
    void query(const QVariant& var);
    QVariant answer();
    void clear();
    bool isString();
public:
    inline QString get() const { if(isNull) return QString(); return data; }
    bool set(const QString &value);
public:
    inline TextField& operator() () { return *this; }
    inline operator QString () { return data; }
    TextField& operator=(const TextField&);
    TextField& operator=(const QString&);
private:
    QString data;
};

struct BlobFieldBindValue
{
public:
    BlobFieldBindValue(const QString& placeholder, const QVariant& val): placeholder(placeholder), val(val){}
    QString placeholder;
    QVariant val;
};
typedef QList<BlobFieldBindValue> BlobFieldBindValues;
class BlobField : public Field
{
public:
    inline BlobField(const QByteArray& data): data(data){ fieldType = FieldType::Blob; isNull = false; }
    inline BlobField() { fieldType = FieldType::Blob; }
    QString select(bool checkNull = true);
    QString createType();
    void query(const QVariant& var);
    QVariant answer();
    void clear();
public:
    inline QByteArray get() const { if(isNull) return QByteArray(); return data; }
    bool set(const QByteArray &value);
public:
    inline BlobField& operator() () { return *this; }
    inline operator QByteArray () { return data; }
    BlobField& operator=(const BlobField&);
    BlobField& operator=(const QByteArray&);
private:
    QByteArray data;
};

}

Q_DECLARE_METATYPE(Qy::ForeignField)
Q_DECLARE_METATYPE(Qy::StringField)
Q_DECLARE_METATYPE(Qy::IntegerField)
Q_DECLARE_METATYPE(Qy::DoubleField)
Q_DECLARE_METATYPE(Qy::DateTimeField)
Q_DECLARE_METATYPE(Qy::BooleanField)
Q_DECLARE_METATYPE(Qy::TextField)
Q_DECLARE_METATYPE(Qy::BlobField)

#endif // FIELD_H
