#include "QyOrmField.h"
#include "QyOrmField_p.h"
#include "QyOrmTable.h"
#include "QyOrmTable_p.h"
#include "QyOrmConfigure.h"
#include "QyOrmDatabase.h"
#include <QJsonValue>
#include <QDebug>

QyOrmField::QyOrmField()
{
    __privated = new QyOrmFieldPrivate;
}

QyOrmField::QyOrmField(const QyOrmField &other)
{
    customVariant = other.customVariant;
    __privated = new QyOrmFieldPrivate(*other.__privated);
}

QyOrmField::QyOrmField(QyOrmField &&other)
{
    std::swap(customVariant, other.customVariant);
    std::swap(__privated, other.__privated);
}

QyOrmField &QyOrmField::operator=(const QyOrmField *other)
{
    equal(other);
    setRecordChanged();
    return *this;
}

QyOrmField &QyOrmField::operator=(QyOrmField &&other)
{
    swap(&other);
    return *this;
}

void QyOrmField::copy(const QyOrmField *other)
{
    if(__privated->fieldType() != other->__privated->fieldType()) return;
    customVariant = other->customVariant;
    *__privated = *other->__privated;
    equal(other);
}

void QyOrmField::swap(QyOrmField *other)
{
    if(__privated->fieldType() != other->__privated->fieldType()) return;
    std::swap(customVariant, other->customVariant);
    std::swap(__privated, other->__privated);
    switch (__privated->fieldType()) {
    case QyOrmFieldType::String:
        std::swap(dynamic_cast<QyOrmStringField*>(this)->__data,
                  dynamic_cast<QyOrmStringField*>(other)->__data); break;
    case QyOrmFieldType::Integer:
        std::swap(dynamic_cast<QyOrmIntegerField*>(this)->__data,
                  dynamic_cast<QyOrmIntegerField*>(other)->__data); break;
    case QyOrmFieldType::Double:
        std::swap(dynamic_cast<QyOrmDoubleField*>(this)->__data,
                  dynamic_cast<QyOrmDoubleField*>(other)->__data); break;
    case QyOrmFieldType::DateTime:
        std::swap(dynamic_cast<QyOrmDateTimeField*>(this)->__data,
                  dynamic_cast<QyOrmDateTimeField*>(other)->__data); break;
    case QyOrmFieldType::Boolean:
        std::swap(dynamic_cast<QyOrmBooleanField*>(this)->__data,
                  dynamic_cast<QyOrmBooleanField*>(other)->__data); break;
    case QyOrmFieldType::Text:
        std::swap(dynamic_cast<QyOrmTextField*>(this)->__data,
                  dynamic_cast<QyOrmTextField*>(other)->__data); break;
    case QyOrmFieldType::Blob:
        std::swap(dynamic_cast<QyOrmBlobField*>(this)->__data,
                  dynamic_cast<QyOrmBlobField*>(other)->__data); break;
    }
}

void QyOrmField::equal(const QyOrmField *other)
{
    __setNull(false);
    switch (__privated->fieldType()) {
    case QyOrmFieldType::String:
        dynamic_cast<QyOrmStringField*>(this)->__data =
                dynamic_cast<const QyOrmStringField*>(other)->__data; break;
    case QyOrmFieldType::Integer:
        dynamic_cast<QyOrmIntegerField*>(this)->__data =
                dynamic_cast<const QyOrmIntegerField*>(other)->__data; break;
    case QyOrmFieldType::Double:
        dynamic_cast<QyOrmDoubleField*>(this)->__data =
                dynamic_cast<const QyOrmDoubleField*>(other)->__data; break;
    case QyOrmFieldType::DateTime:
        dynamic_cast<QyOrmDateTimeField*>(this)->__data =
                dynamic_cast<const QyOrmDateTimeField*>(other)->__data; break;
    case QyOrmFieldType::Boolean:
        dynamic_cast<QyOrmBooleanField*>(this)->__data =
                dynamic_cast<const QyOrmBooleanField*>(other)->__data; break;
    case QyOrmFieldType::Text:
        dynamic_cast<QyOrmTextField*>(this)->__data =
                dynamic_cast<const QyOrmTextField*>(other)->__data; break;
    case QyOrmFieldType::Blob:
        dynamic_cast<QyOrmBlobField*>(this)->__data =
                dynamic_cast<const QyOrmBlobField*>(other)->__data; break;
    }
}

QyOrmField::~QyOrmField()
{
    if(__privated) delete __privated;
}

QyOrmTable *QyOrmField::table() const
{
    return __privated->table;
}

bool QyOrmField::isNull() const
{
    return __privated->isNull;
}

bool QyOrmField::isScanChanged() const
{
    return __privated->isScanChanged;
}

void QyOrmField::setScanChanged()
{
    __privated->isScanChanged = true;
}

QString QyOrmField::fieldName() const
{
    return __privated->fieldName();
}

QString QyOrmField::fieldLabel() const
{
    return __privated->fieldLabel();
}

QyOrmFieldType QyOrmField::fieldType() const
{
    return __privated->fieldType();
}

QString QyOrmField::typeString() const
{
    switch (type()) {
    case QyOrmFieldType::String: return "String";
    case QyOrmFieldType::Integer: return "Integer";
    case QyOrmFieldType::Double: return "Double";
    case QyOrmFieldType::DateTime: return "DateTime";
    case QyOrmFieldType::Boolean: return "Boolean";
    case QyOrmFieldType::Text: return "Text";
    case QyOrmFieldType::Blob: return "Blob";
    }
    return "Unknown";
}

bool QyOrmField::isConstraintAutoPrimaryKey() const
{
    return __privated->autoPrimaryKey();
}

bool QyOrmField::isConstraintPrimaryKey() const
{
    return __privated->primaryKey();
}

bool QyOrmField::isConstraintUnique() const
{
    return __privated->unique();
}

bool QyOrmField::isConstraintNotNull() const
{
    return __privated->notNull();
}

bool QyOrmField::isRecordChanged()
{
    if(__privated->table && !__privated->table->__privated->isAssigning)
        return __privated->table->__privated->recordChangedFields.contains(this);
    return false;
}

void QyOrmField::setRecordChanged()
{
    if(__privated->table && !__privated->table->__privated->isAssigning)
        __privated->table->__privated->recordChangedFields.insert(this);
}

QVariant QyOrmField::getDefaultValue()
{
    return __privated->extraInfo->defaultValue;
}

void QyOrmField::queryDefaultValue(const QVariant &data)
{
    if(__privated->extraInfo->defaultValue.isValid()){
        query(__privated->extraInfo->defaultValue);
    }else{
        query(data);
    }
}

void QyOrmField::clear()
{
    bool isAssigning = __privated->table->__privated->isAssigning;
    __privated->table->__privated->isAssigning = true;
    query(QVariant());
    __privated->table->__privated->isAssigning = isAssigning;

}

void QyOrmField::setNull()
{
    query(QVariant());
}

void QyOrmField::__setNull(bool isNull)
{
    __privated->isNull = isNull;
    setRecordChanged();
}

QString QyOrmField::sqlValueString()
{
    if(__privated->isNull) return "NULL";
    if(__privated->isString()) {
        return '\'' + toString().replace('\'', "''") + '\'';
    }
    if(__privated->fieldType() == QyOrmFieldType::DateTime) {
        return '\'' + toString() + '\'';
    }
    if(__privated->fieldType() == QyOrmFieldType::Boolean){
        if(answer().toBool()) return "'1'"; else return "'0'";
    }
    if(__privated->fieldType() == QyOrmFieldType::Blob){
        __privated->table->__privated->blobFields.append(toBlobField());
        return ':' + __privated->extraInfo->fieldName;
    }
    return toString();
}

QString QyOrmField::sqlFieldString(bool brace) const
{
    if(brace || QyOrmConfigure::enabledBrace){
        return '"' + __privated->fieldName() + '"';
    }else{
        return __privated->fieldName();
    }
}

QString QyOrmField::toNumber(const QString &text, int count)
{
    if(QyOrmDatabase::isDriverPgsql()){
        QString result;
        for(int i = 0; i < count; ++i) result.append('9');
        return "TO_NUMBER(REGEXP_REPLACE(" + text + ", '\\D', '0', 'g'), '" + result + "')";
    }else{
        return "CAST(" + text + " AS INTEGER)";
    }
}

QString QyOrmField::sqlToNumberFieldName() const
{
    return toNumber(__privated->fieldName(), __privated->orderToCastNumber);
}

void QyOrmField::appendExecuter(const QString &executer)
{
    __privated->executers.append(executer);
    __privated->table->__privated->recordChangedFields.insert(this);
}

void QyOrmField::setExecuterConnectByOr()
{
    __privated->isAndExecuter = false;
}

void QyOrmField::setExecuteOperator(const QString &operatorString)
{
    __privated->operatorString = ' ' + operatorString + ' ';
}

void QyOrmField::setExecuteReplace(const QString &source, const QString &target)
{
    __privated->replaceSource = source;
    __privated->replaceTarget = target;
}

QString QyOrmField::sqlExecuteString(bool belong, bool brace)
{
    QString result;
    if(belong || QyOrmConfigure::enabledBelong){
        result = __privated->table->tableName() + '.';
    }
    QString suffix;
    if(__privated->replaceSource.size()){
        suffix = QString("=REPLACE(%1, '%2', '%3')")
                .arg(sqlFieldString(brace), __privated->replaceSource, __privated->replaceTarget);
    }else if(__privated->executers.size()){
        QString temp;
        foreach(QString executer, __privated->executers){
            if(temp.size()) temp += __privated->isAndExecuter ? " AND " : " OR ";
            temp += result + sqlFieldString(brace) + ' ' + executer;
        }
        return '(' + temp + ')';
    }else if(__privated->operatorString.size() > 1 &&
             __privated->operatorString.at(1) == '@'){
        int code = __privated->operatorString.at(2).unicode();
        switch (code) {
        case '1': suffix = " IS NULL "; break;
        case '2': suffix = " IS NOT NULL "; break;
        default: suffix = " IS NOT NULL AND " + result + sqlFieldString(brace) + "!='' ";
        }
    }else if(__privated->isNull){
        suffix = " IS NULL ";
    }else{
        suffix = __privated->operatorString + sqlValueString();
    }
    return result + sqlFieldString(brace) + suffix;
}

void QyOrmField::setDesc()
{
    __privated->isDesc = true;
}

void QyOrmField::setPassSelect(bool is, bool isAll)
{
    if(isAll) __privated->extraInfo->isPassSelect = is;
    __privated->isPassSelect = is;
}

//SELECT tableName.replaceName AS fieldName() JOIN tableName
//on tableName().fieldName()=tableName.(onFieldName isNull ? fieldName() : onFieldName)
void QyOrmField::setJoin(const QString &tableName, const QString &replaceName, const QString &onFieldName)
{
    __privated->table->__privated->joinOnFields.append(this);
    __privated->joinTableName = tableName;
    __privated->joinReplaceName = replaceName;
    __privated->joinOnFieldName = onFieldName;
}

QString QyOrmField::toJson() const
{
    if(__privated->isNull) return "null";
    switch (__privated->fieldType()) {
    case QyOrmFieldType::String: return QString("\"%1\"").arg(toString());
    case QyOrmFieldType::Text: return QString("\"%1\"").arg(toString());
    case QyOrmFieldType::DateTime: return QString("\"%1\"").arg(toString());
    case QyOrmFieldType::Boolean: return (answer().toBool() ? "true" : "false");
    default: return toString();
    }
}

QJsonValue QyOrmField::toQJson() const
{
    if(__privated->isNull) return QJsonValue::Null;
    switch (__privated->fieldType()) {
    case QyOrmFieldType::Blob:
    case QyOrmFieldType::DateTime: return toString();
    default: return QJsonValue::fromVariant(answer());
    }
}

QString QyOrmDateTimeField::__y_datetime_in_format = "yyyy-MM-dd hh:mm:ss";
QString QyOrmDateTimeField::__y_datetime_out_format = "yyyy-MM-dd hh:mm:ss";
QString QyOrmDateTimeField::__create_type_name = "DATETIME";

#define QUERY_FUNCTION(ClassName, Type) \
void ClassName::query(const QVariant &var) \
{ \
    if(var.isNull()) { __setNull(); __data = QVariant().value<Type>();} \
    else set(var.value<Type>()); \
}
QUERY_FUNCTION(QyOrmStringField, QString)
QUERY_FUNCTION(QyOrmIntegerField, int)
QUERY_FUNCTION(QyOrmDoubleField, double)
QUERY_FUNCTION(QyOrmBooleanField, bool)
QUERY_FUNCTION(QyOrmTextField, QString)
void QyOrmStringField::setCurrentDate(const QString& format)
{
    __setNull(false);
    __data = QDateTime::currentDateTime().toString(format);
}
void QyOrmDateTimeField::query(const QVariant &var)
{
    if(var.type() == QVariant::String){
        QString vf = getValidInFormat();
        set(QDateTime::fromString(var.toString().left(vf.size()), vf));
    }else{
        set(var.toDateTime());
    }
}

void QyOrmDateTimeField::setLikeDateTime(const QString &sDateTime, const QString &likeFormat)
{
    setRecordChanged();
    __privated->dateTimePrivated = new QyOrmDateTimeFieldPrivate;
    __privated->dateTimePrivated->isLike = true;
    __privated->dateTimePrivated->likeString = sDateTime;
    __privated->dateTimePrivated->likeFormat = likeFormat;
}

QString QyOrmDateTimeField::sqlExecuteString(bool belong, bool brace)
{
    if(__privated->dateTimePrivated && __privated->dateTimePrivated->isLike){
        QString result;
        if(belong || QyOrmConfigure::enabledBelong){
            result = __privated->table->tableName() + '.';
        }
        if(__privated->dateTimePrivated->likeFormat.isEmpty()){
            if(QyOrmDatabase::isDriverSqlite()){
                __privated->dateTimePrivated->likeFormat = "%Y%m%d%H%M%S";
            }else{
                __privated->dateTimePrivated->likeFormat = "YYYYMMDDHH24MISS";
            }
        }
        if(QyOrmDatabase::isDriverSqlite()){
            result = QString("strftime('%1', %2%3)").arg(
                __privated->dateTimePrivated->likeFormat, result, __privated->fieldName());
        }else{
            result = QString("to_char(%1%2, '%3')").arg(
                result, __privated->fieldName(), __privated->dateTimePrivated->likeFormat);
        }
        return (result + " LIKE '" + __privated->dateTimePrivated->likeString + '\'');
    }
    return QyOrmField::sqlExecuteString(belong, brace);
}

void QyOrmBlobField::query(const QVariant &var)
{
    if(var.isNull()){
        __setNull();
    }else if(var.type() == QVariant::String){
        set(QByteArray::fromBase64(var.toString().toUtf8()));
        return;
    }
    set(var.toByteArray());
}
