#include "Field.h"
#include "Table.h"

using namespace Qy;

QString Field::createTemplate()
{
    QString extra;
    if(primaryKey){
        extra += " PRIMARY KEY";
    }else{
        if(unique){
            extra += " UNIQUE";
        }
        if(notNull){
            extra += " NOT NULL";
        }
    }
    return fieldName + " %1" + extra;
}

QString Field::toJson(bool completeForeign)
{
    if(isNull) return "null";
    if(fieldType == FieldType::Foreign){
        if(completeForeign){
            return dynamic_cast<ForeignField*>(this)->get()->toJson();
        }else{
            return dynamic_cast<ForeignField*>(this)->get()->primaryKey->toJson(false);
        }
    }else if(fieldType == FieldType::String){
        return QString("\"%1\"").arg(dynamic_cast<StringField*>(this)->get());
    }else if(fieldType == FieldType::Boolean){
        return (dynamic_cast<BooleanField*>(this)->get() ? "true" : "false");
    }else if(fieldType == FieldType::DateTime){
        return QString("\"%1\"").arg(dynamic_cast<DateTimeField*>(this)->toString());
    }else if(fieldType == FieldType::Text){
        return QString("\"%1\"").arg(dynamic_cast<TextField*>(this)->get());
    }else if(fieldType == FieldType::Blob){
        return  QString("\"%1\"").arg(QString::fromUtf8(dynamic_cast<BlobField*>(this)->get().toBase64()));
    }else{
        return select(false);
    }
}

QJsonValue Field::toQJson(bool completeForeign)
{
    if(isNull) return QJsonValue::Null;
    if(fieldType == FieldType::Foreign){
        if(completeForeign){
            return dynamic_cast<ForeignField*>(this)->get()->toQJson();
        }else{
            return dynamic_cast<ForeignField*>(this)->get()->primaryKey->toQJson(false);
        }
    }else if(fieldType == FieldType::String){
        return QJsonValue(dynamic_cast<StringField*>(this)->get());
    }else if(fieldType == FieldType::Boolean){
        return QJsonValue(dynamic_cast<BooleanField*>(this)->get());
    }else if(fieldType == FieldType::Integer){
        return QJsonValue(dynamic_cast<IntegerField*>(this)->get());
    }else if(fieldType == FieldType::Double){
        return QJsonValue(dynamic_cast<DoubleField*>(this)->get());
    }else if(fieldType == FieldType::DateTime){
        return QJsonValue(QString("\"%1\"").arg(select(false)));
    }else if(fieldType == FieldType::Text){
        return QJsonValue(dynamic_cast<TextField*>(this)->get());
    }else if(fieldType == FieldType::Blob){
        return QJsonValue(QString::fromUtf8(dynamic_cast<BlobField*>(this)->get().toBase64()));
    }return QJsonValue();
}

QString Field::condition(bool needTableDot)
{
    static QString betweenSqlDot = "%1.%2 BETWEEN %3 AND %4";
    static QString betweenSql = "%1 BETWEEN %2 AND %3";
    static QString inSqlDot = "%1.%2 in (%3)";
    static QString inSql = "%1 in (%2)";
    static QString sqlDot = "%1.%2 %3 %4";
    static QString sql = "%1 %2 %3";

    if(fieldType == FieldType::Blob) return QString("1=1");
    if(!betweenValue.isNull()){
        QVariant original = answer();
        QString result = (needTableDot ? betweenSqlDot.arg(table->tableName) : betweenSql).arg(fieldName).arg(select());
        query(betweenValue);
        result = result.arg(select());
        query(original);
        return result;
    }else if(!rangeValues.isEmpty()){
        QVariant original = answer();
        QString result = (needTableDot ? inSqlDot.arg(table->tableName) : inSql).arg(fieldName), ranges;
        for(QVariant& var : rangeValues){
            query(var);
            ranges.append(select() + ',');
        }
        ranges.chop(1);
        query(original);
        return result.arg(ranges);
    }
    return (needTableDot ? sqlDot.arg(table->tableName) : sql).arg(fieldName).arg(conditionOperator).arg(select());
}

QString Field::create()
{
    return createTemplate().arg(createType());
}

QString Field::execute(bool needTableDot)
{
    static QString sqlDot = "%1.%2=%3";
    static QString sql = "%1=%2";
    return (needTableDot ? sqlDot.arg(table->tableName) : sql).arg(fieldName).arg(select());
}

bool Field::isString()
{
    return false;
}

QString ForeignField::select(bool checkNull)
{
    return data->primaryKey->select(checkNull);
}

QString ForeignField::createType()
{
    return data->primaryKey->createType();
}

void ForeignField::query(const QVariant &var)
{
    if(var.isNull()){
        clear();
    }
    if(var.type() == QVariant::Type::LongLong || var.type() == QVariant::Type::ULongLong){
        if(data){
            data->deleteLater();
        }
        isNull = false;
        if(data){
            data->deleteLater();
        }
        data = reinterpret_cast<Table*>(var.toULongLong());
        table->fieldChanged.append(this);
    }else{
        QList<Table*> result = table->createTable(table->hashForeign.value(fieldName), var);
        if(result.size()){
            isNull = false;
            if(data){
                data->deleteLater();
            }
            data = result.first();
            table->fieldChanged.append(this);
        }
    }
}

QVariant ForeignField::answer()
{
    return QVariant(reinterpret_cast<qintptr>(data));
}

void ForeignField::clear()
{
    isNull = true;
    data = nullptr;
    return;
}

bool ForeignField::set(Table *value, bool isWild)//isWild-表示value是否需要指定parent
{
    if(!table){ data = value; return true; }\
    table->fieldChanged.append(this);
    Table* tmp = data;
    data = value;
    if(isWild){
        data->setParent(this);
    }
    if(!table->isInstance()){
        if(tmp && !tmp->isInstance()){
            tmp->deleteLater();
        }
        isNull = false;
        return true;
    }
    if(table->update(this)){
        emit modify();
        if(tmp && tmp->parent() == this){
            tmp->setParent(nullptr);
            delete tmp;
        }
        isNull = false;
        return true;
    }else{
        data = tmp;
        if(isWild){
            value->setParent(nullptr);
            delete value;
        }
        return false;
    }
}

ForeignField &ForeignField::operator=(const ForeignField &f){this->self = f.self;set(f.data);return *this;}
ForeignField &ForeignField::operator=(Table *value)
{
    set(value);
    return *this;
}

#define FIELD_SET(f,T) bool f ::set(const T &value)\
{\
    if(!table){ data = value; isNull = false; return true; }\
    table->fieldChanged.append(this);\
    T tmp = data;\
    data = value;\
    if(!table->isInstance()) {\
        isNull = false;\
        return true;\
    }\
    if(table->update(this)){\
        emit modify();\
        isNull = false;\
        return true;\
    }else{\
        data = tmp;\
        return false;\
    }\
}

#define FIELD_QUERY(Class, Type)\
void Class::query(const QVariant& var)\
{\
    if(var.isNull()){\
        clear();\
    }else{\
        set(var.Type());\
    }\
}

#define FIELD_ANSWER(Class)\
QVariant Class::answer(){\
    return data;\
}



QString StringField::select(bool checkNull){QString r=QString("'%1'").arg(data);return(checkNull?selectCheckNull(r):r);}
QString StringField::createType(){return ("VARCHAR(255)");}
FIELD_QUERY(StringField, toString)
FIELD_ANSWER(StringField)
void StringField::clear(){isNull = true;data = QString();}
bool StringField::isString(){return true;}
FIELD_SET(StringField, QString)
StringField &StringField::operator=(const StringField &f){this->self = f.self;set(f.data);return *this;}
StringField &StringField::operator=(const QString &value){set(value);return *this;}

QString IntegerField::select(bool checkNull){QString r=QString::number(data);return(checkNull?selectCheckNull(r):r);}
QString IntegerField::createType(){return ("INTEGER");}
FIELD_QUERY(IntegerField, toInt)
FIELD_ANSWER(IntegerField)
void IntegerField::clear(){isNull = true;data = 0;}
FIELD_SET(IntegerField, int32_t)
IntegerField &IntegerField::operator=(const IntegerField &f){this->self = f.self;set(f.data);return *this;}
IntegerField &IntegerField::operator=(const int32_t &value){set(value);return *this;}

QString DoubleField::select(bool checkNull){QString r=QString::number(data);return(checkNull?selectCheckNull(r):r);}
QString DoubleField::createType(){return ("REAL");}
FIELD_QUERY(DoubleField, toDouble)
FIELD_ANSWER(DoubleField)
void DoubleField::clear(){isNull = true;data = 0.0;}
FIELD_SET(DoubleField, double)
DoubleField &DoubleField::operator=(const DoubleField &f){this->self = f.self;set(f.data);return *this;}
DoubleField &DoubleField::operator=(const double &value){set(value);return *this;}

QString DateTimeField::y_datetime_format = "yyyy-MM-dd hh:mm:ss";
QString DateTimeField::select(bool checkNull){QString r=QString("'%1'").arg(toString());
                                              return(checkNull?selectCheckNull(r):r);}
QString DateTimeField::createType(){return ("datetime");}
void DateTimeField::query(const QVariant& var)
{
    if(!var.isNull()){
        if(var.type() == QVariant::String){
            set(QDateTime::fromString(var.toString(), getValidFormat()));
        }else if(var.type() == QVariant::DateTime){
            set(var.toDateTime());
        }
    }
}
FIELD_ANSWER(DateTimeField)
void DateTimeField::clear(){isNull = true;data = QDateTime();}
bool DateTimeField::set(const QDateTime &value)
{
    if(!table){ data = value; return true; }
    if(value.isNull()) return false;
    table->fieldChanged.append(this);
    QDateTime tmp = data;
    data = value;
    if(!table->isInstance()) {
        isNull = false;
        return true;
    }
    if(table->update(this)){
        emit modify();
        isNull = false;
        return true;
    }else{
        data = tmp;
        return false;
    }
}
DateTimeField &DateTimeField::operator=(const DateTimeField &f){this->self = f.self;set(f.data);return *this;}
DateTimeField &DateTimeField::operator=(const QDateTime &value){set(value);return *this;}

QString BooleanField::select(bool checkNull){QString r=QString::number(int(data));return(checkNull?selectCheckNull(r):r);}
QString BooleanField::createType(){return ("BOOLEAN");}
FIELD_QUERY(BooleanField, toBool)
FIELD_ANSWER(BooleanField)
void BooleanField::clear(){isNull = true;data = false;}
FIELD_SET(BooleanField, bool)
BooleanField &BooleanField::operator=(const BooleanField &f){this->self = f.self;set(f.data);return *this;}
BooleanField &BooleanField::operator=(const bool &value){set(value);return *this;}


QString TextField::select(bool checkNull){QString r=QString("'%1'").arg(data);return(checkNull?selectCheckNull(r):r);}
QString TextField::createType(){return ("TEXT");}
FIELD_QUERY(TextField, toString)
FIELD_ANSWER(TextField)
void TextField::clear(){isNull = true;data = QString();}
bool TextField::isString(){return true;}
FIELD_SET(TextField, QString)
TextField &TextField::operator=(const TextField &f){this->self = f.self;set(f.data);return *this;}
TextField &TextField::operator=(const QString &value){set(value);return *this;}


QString BlobField::select(bool checkNull){QString r=QString(":%1").arg(fieldName);return(checkNull?selectCheckNull(r):r);}
QString BlobField::createType(){return ("BLOB");}
void BlobField::query(const QVariant& var)
{
    if(!var.isNull()){
        if(var.type() == QVariant::String){
            set(QByteArray::fromBase64(var.toString().toUtf8()));
        }else if(var.type() == QVariant::ByteArray){
            set(var.toByteArray());
        }
    }
}
FIELD_ANSWER(BlobField)
void BlobField::clear(){isNull = true;data = QByteArray();}
FIELD_SET(BlobField, QByteArray)
BlobField &BlobField::operator=(const BlobField &f){this->self = f.self;set(f.data);return *this;}
BlobField &BlobField::operator=(const QByteArray &value){set(value);return *this;}
