#ifndef QYORMFIELD_H
#define QYORMFIELD_H

#include <QDateTime>
#include <QVariant>
#include <functional>
#include "QyOrmFieldDefine.h"

class QyOrmField
{
public:
    QyOrmField();
    QyOrmField(const QyOrmField& other);
    QyOrmField(QyOrmField&& other);
    inline QyOrmField& operator=(const QyOrmField& other) { return QyOrmField::operator=(&other); }
    QyOrmField& operator=(const QyOrmField* other);
    QyOrmField& operator=(QyOrmField&& other);
    virtual void copy(const QyOrmField* other);
    virtual void swap(QyOrmField* other);
    virtual void equal(const QyOrmField* other);
    virtual ~QyOrmField();

public:
    QyOrmTable* table() const;
    bool isNull() const;
    bool isScanChanged() const;
    void setScanChanged();
    QString     fieldName() const;
    QString     fieldLabel() const;
    QyOrmFieldType fieldType() const;
    QString typeString() const;

public:
    inline void setChanged() { setScanChanged(); setRecordChanged(); }
    inline bool notIsNull() const{ return !isNull(); }
    inline QyOrmTable* parent() const { return table(); }
    inline QString     name() const { return fieldName(); }
    inline QString     label() const { return fieldLabel(); }
    inline QyOrmFieldType type() const { return fieldType(); }

public:
    bool isConstraintAutoPrimaryKey() const;
    bool isConstraintPrimaryKey() const;
    bool isConstraintUnique() const;
    bool isConstraintNotNull() const;
    inline bool isAutoPrimaryKey() const { return isConstraintAutoPrimaryKey(); }
    inline bool isPrimaryKey() const { return isConstraintPrimaryKey(); }
    inline bool isUnique() const { return isConstraintUnique(); }
    inline bool isNotNull() const { return isConstraintNotNull(); }

public:
    bool isRecordChanged();
    void setRecordChanged();
    QVariant getDefaultValue();
    void queryDefaultValue(const QVariant& data = QVariant());
    inline void setDefaultValue(const QVariant& data = QVariant())
    { queryDefaultValue(data); }
    virtual void    query(const QVariant& var) = 0;//通过QVariant更新值
    virtual QVariant answer() const = 0;//返回实际值的QVariant值
    virtual void    clear();//设置为NULL，不触发修改
    virtual void    setNull();//设置为NULL，触发修改
    void    __setNull(bool isNull = true);

public:
    virtual void fromString(const QString& string) = 0;
    virtual QString toString() const = 0;//返回实际值的字符串值
    virtual QString sqlValueString();//返回数据库能认识的字符串值(如:'test', 1)
    virtual QString sqlFieldString(bool brace = false) const;
    static QString toNumber(const QString& text, int count = 2);
    virtual QString sqlToNumberFieldName() const;
    void appendExecuter(const QString& executer);
    void setExecuterConnectByOr();
    void setExecuteOperator(const QString& operatorString);
    void setExecuteReplace(const QString& source, const QString& target);
    inline void setExecuteOperatorDefault() { setExecuteOperator("="); }
    inline void setExecuteOperatorLessThan() { setExecuteOperator("<"); }
    inline void setExecuteOperatorLessThanEqual() { setExecuteOperator("<="); }
    inline void setExecuteOperatorGreaterThan() { setExecuteOperator(">"); }
    inline void setExecuteOperatorGreaterThanEqual() { setExecuteOperator(">="); }
    inline void setExecuteOperatorNot() { setExecuteOperator("!="); }
    inline void setExecuteOperatorLike() { setExecuteOperator("like"); }
    inline void setExecuteOperatorIsNull() { setRecordChanged(); setExecuteOperator("@1"); }
    inline void setExecuteOperatorIsNotNull() { setRecordChanged(); setExecuteOperator("@2"); }
    virtual QString sqlExecuteString(bool belong = false, bool brace = false);//用于Where条件赋值语句
    void setDesc();
    void setPassSelect(bool is = true, bool isAll = false);//不参与Select
    //SELECT tableName.replaceName AS fieldName() JOIN tableName
    //on tableName().fieldName()=tableName.(onFieldName isNull ? fieldName() : onFieldName)
    void setJoin(const QString& tableName, const QString& replaceName = QString(),
                 const QString& onFieldName = QString());

public:
    QString toJson() const;
    QJsonValue toQJson() const;

public:
    inline QyOrmStringField*   toStringField();
    inline QyOrmIntegerField*  toIntegerField();
    inline QyOrmDoubleField*   toDoubleField();
    inline QyOrmDateTimeField* toDateTimeField();
    inline QyOrmBooleanField*  toBooleanField();
    inline QyOrmTextField*     toTextField();
    inline QyOrmBlobField*     toBlobField();

public:
    QVariant customVariant;

public:
    inline QyOrmField* operator()() { return this; }
protected:
    QyOrmFieldPrivate* __privated = nullptr;
    friend class QyOrmTable;
    friend class QyOrmCompleteField;
};
typedef QyOrmField* QyOrmFieldPtr;
Q_DECLARE_METATYPE(QyOrmField*)

#ifdef QY_STD
#include "QyUuid.h"
#endif
class QyOrmStringField : public QyOrmField
{
public:
    virtual void query(const QVariant& var) override;
    QyOrmFieldCommonDefine(QyOrmStringField, QString, QString())
    inline operator const QString () const { return __data; }
    inline QyOrmStringField& operator=(const char* cs) { set(QString(cs)); return *this; }
    inline virtual QString toString() const override { return __data; }
    inline virtual void fromString(const QString& string) override { __setNull(false); __data = string; }
    inline bool isEmpty() { return isNull() || __data.isEmpty(); }
    inline int size() { return isNull() ? 0 : __data.size(); }
    inline void trim() { __data = __data.trimmed(); }
    inline bool isEqual(const QString& v) { return (__data == v); }
#ifdef QY_STD
    inline void autoUuid(bool isSec = true){set(isSec ? QyUuid::uuidSec() : QyUuid::uuidMsecOrder());}
#endif
    void setCurrentDate(const QString& format = "yyyy-MM-dd");
};
QyOrmStringField* QyOrmField::toStringField() { return dynamic_cast<QyOrmStringField*>(this); }

class QyOrmIntegerField : public QyOrmField
{
public:
    virtual void query(const QVariant& var) override;
    QyOrmFieldCommonDefine(QyOrmIntegerField, int, 0)
    inline virtual QString toString() const override { return QString::number(__data); }
    inline virtual void fromString(const QString& string) override { __setNull(false); __data = string.toInt(); }
    inline void increment(int value = 1) { set(__data + value); }
    inline void decrement(int value = 1) { set(__data - value); }
    inline QyOrmIntegerField& operator+=(const int& value) { set(__data + value); return *this; }
    inline QyOrmIntegerField& operator-=(const int& value) { set(__data - value); return *this; }
};
QyOrmIntegerField* QyOrmField::toIntegerField() { return dynamic_cast<QyOrmIntegerField*>(this); }

class QyOrmDoubleField : public QyOrmField
{
public:
    virtual void query(const QVariant& var) override;
    QyOrmFieldCommonDefine(QyOrmDoubleField, double, 0.0)
    inline virtual QString toString() const override { return QString::number(__data); }
    inline virtual void fromString(const QString& string) override { __setNull(false); __data = string.toDouble(); }
};
QyOrmDoubleField* QyOrmField::toDoubleField() { return dynamic_cast<QyOrmDoubleField*>(this); }

class QyOrmDateTimeField : public QyOrmField
{
public:
    inline void set(const QDateTime & value) { __setNull(value.isNull()); __data = value; }
    virtual void query(const QVariant& var) override;
    QyOrmFieldCommonDefineWithoutSet(QyOrmDateTimeField, QDateTime, QDateTime())
    inline operator const QDateTime () const { return __data; }
    inline virtual QString toString() const override { return __data.toString(getValidOutFormat()); }
    inline virtual void fromString(const QString& string) override
    { __setNull(false); __data = QDateTime::fromString(string, getValidInFormat()); }
    inline void setCurrentDateTime() { __setNull(false); __data = QDateTime::currentDateTime(); }
    static inline QDateTime currentDateTime() { return QDateTime::currentDateTime(); }
    static inline QDate currentDate() { return QDate::currentDate(); }
    static inline QTime currentTime() { return QTime::currentTime(); }
    static inline QString currentDateTimeString(QString format)
    { return currentDateTime().toString(format); }
    static inline QString currentDateString(QString format)
    { return QDate::currentDate().toString(format); }
    static inline QString currentTimeString(QString format)
    { return QTime::currentTime().toString(format); }

public:
    void setLikeDateTime(const QString& sDateTime, const QString& likeFormat = QString());
    virtual QString sqlExecuteString(bool belong = false, bool brace = false) override;//用于update赋值语句

public:
    inline QDate date() { return __data.date(); }
    inline int year() { return __data.date().year(); }
    inline int month() { return __data.date().month(); }
    inline int day() { return __data.date().day(); }
    inline QTime time() { return __data.time(); }
    inline int hour() { return __data.time().hour(); }
    inline int minute() { return __data.time().minute(); }
    inline int second() { return __data.time().second(); }
public:
    inline QString getValidInFormat() { return
                (__datetime_in_format.isNull() ? __y_datetime_in_format : __datetime_in_format); }
    inline void setLocalInFormat(const QString& formatString) { __datetime_in_format = formatString; }
    inline QString getLocalInFormat() { return __datetime_in_format; }
    static inline void setGlobalInFormat(const QString& formatString) { __y_datetime_in_format = formatString; }
    static inline QString getGlobalInFormat() { return __y_datetime_in_format; }

    inline QString getValidOutFormat() const { return
                (__datetime_out__format.isNull() ? __y_datetime_out_format : __datetime_out__format); }
    inline void setLocalOutFormat(const QString& formatString) { __datetime_out__format = formatString; }
    inline QString getLocalOutFormat() { return __datetime_out__format; }
    static inline void seGlobaltOutFormat(const QString& formatString) { __y_datetime_out_format = formatString; }
    static inline QString getGlobalOutFormat() { return __y_datetime_out_format; }
private:
    QString __datetime_in_format;
    QString __datetime_out__format;
    static QString __y_datetime_in_format;
    static QString __y_datetime_out_format;
public:
    static QString __create_type_name;
};
QyOrmDateTimeField* QyOrmField::toDateTimeField() { return dynamic_cast<QyOrmDateTimeField*>(this); }

class QyOrmBooleanField : public QyOrmField
{
public:
    virtual void query(const QVariant& var) override;
    QyOrmFieldCommonDefine(QyOrmBooleanField, bool, false)
    inline QyOrmBooleanField& operator=(int data) { set(data); return *this; }
    inline virtual QString toString() const override { return QString::number(__data); }
    inline virtual void fromString(const QString& string) override { __setNull(false); __data = string.toInt(); }
    inline void reverse() { __setNull(false); __data = !__data; }
};
QyOrmBooleanField* QyOrmField::toBooleanField() { return dynamic_cast<QyOrmBooleanField*>(this); }

class QyOrmTextField : public QyOrmField
{
public:
    virtual void query(const QVariant& var) override;
    QyOrmFieldCommonDefine(QyOrmTextField, QString, QString())
    inline operator const QString () const { return __data; }
    inline virtual QString toString() const override { return __data; }
    inline virtual void fromString(const QString& string) override { __setNull(false); __data = string; }
    inline bool isEmpty() {  return isNull() || __data.isEmpty(); }
};
QyOrmTextField* QyOrmField::toTextField() { return dynamic_cast<QyOrmTextField*>(this); }

class QyOrmBlobField : public QyOrmField
{
public:
    virtual void query(const QVariant& var) override;
    QyOrmFieldCommonDefine(QyOrmBlobField, QByteArray, QByteArray())
    inline operator const QByteArray () const { return __data; }
    inline virtual QString toString() const override { return QString::fromUtf8(__data.toBase64()); }
    inline virtual void fromString(const QString& string) override
    { __setNull(false); __data = QByteArray::fromBase64(string.toUtf8()); }
};
QyOrmBlobField* QyOrmField::toBlobField() { return dynamic_cast<QyOrmBlobField*>(this); }

#endif // QYORMFIELD_H
