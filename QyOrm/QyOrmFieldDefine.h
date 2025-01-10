#ifndef QYORMFIELDDEFINE_H
#define QYORMFIELDDEFINE_H

#include <QString>

namespace QyOrmEnum {
enum QyOrmFieldType{
    String = 0, Integer, Double, DateTime, Boolean, Text, Blob
};
enum QyOrmFieldConstraint{
    Plain = 0, NotNull, Unique, NotNullUnique, PrimaryKey, AutoPrimaryKey
};
}
typedef QyOrmEnum::QyOrmFieldType QyOrmFieldType;
typedef QyOrmEnum::QyOrmFieldConstraint QyOrmFieldConstraint;

class QyOrmTable;
class QyOrmStringField;
class QyOrmIntegerField;
class QyOrmDoubleField;
class QyOrmDateTimeField;
class QyOrmBooleanField;
class QyOrmTextField;
class QyOrmBlobField;
class QyOrmFieldPrivate;

#define QyOrmFieldCommonDefine(ClassName, Type, defaultDataValue) \
public: \
    inline void set(const Type & value) { __data = value; __setNull(false); } \
    QyOrmFieldCommonDefineWithoutSet(ClassName, Type, defaultDataValue)\

#define QyOrmFieldCommonDefineWithoutSet(ClassName, Type, defaultDataValue) \
public: \
    inline ClassName() = default; \
    inline ClassName(const ClassName &) = default; \
    inline ClassName(ClassName &&) = default; \
    inline ClassName(const Type& value) { set(value); } \
    inline operator QVariant () { return __data; } \
    inline Type get() const { return __data; } \
    inline operator Type () { return __data; } \
    inline ClassName& operator=(const Type& value) { set(value); return *this; } \
    inline ClassName& operator=(const ClassName& data) { set(data.__data); return *this; } \
    inline ClassName& operator=(const QVariant& data) { query(data); return *this; } \
    inline ClassName& operator=(QyOrmField* field) { dynamic_cast<QyOrmField&>(*this) = field; return *this; } \
public: \
    Type __data = defaultDataValue; \
public: \
    inline virtual QVariant answer() const override { return isNull() ? QVariant() : QVariant(get()); } \

#endif // QYORMFIELDDEFINE_H
//
