#ifndef QYORMTABLEDEFINE_H
#define QYORMTABLEDEFINE_H

#include "QyOrmFieldExtraInfo.h"

namespace QyOrmEnum {
enum QyOrmItemRole{
    Id = Qt::UserRole, Data, Index, Count, Parent,
};
enum QyOrmInputWidgetType{
    QLineEditType = 0, QTextEditType, QPlainTextEditType, QDateTimeEditType, QComboBoxType,
    QComboBoxIndexType, QComboBoxDataType,
    QSpinBoxType, QDoubleSpinBoxType, QAbstractButtonType, QTableWidgetItemType, BlobType,
    StringAbstractType
};
enum QyOrmForeignKeyConstraint{
    Miss = 0, Restrict, NoAction, Cascade, SetNull, SetDefault
};
}
typedef QyOrmEnum::QyOrmItemRole QyItemRole;
typedef QyOrmEnum::QyOrmItemRole QyOrmItemRole;
typedef QyOrmEnum::QyOrmInputWidgetType QyOrmInputWidgetType;
typedef QyOrmEnum::QyOrmForeignKeyConstraint QyOrmForeignKeyConstraint;

class QyOrmField;
class QyOrmTablePrivate;

#define QYORM_TABLE(TABLE) \
    public: \
    explicit inline TABLE(QObject *parent = nullptr) : QyOrmTable(parent){ __initialize(); }\
    QYORM_TABLE_INLINE_FUNCTION(TABLE) \

#define QYORM_TABLE_FUNC(TABLE, func) \
    public: \
    explicit inline TABLE(QObject *parent = nullptr) : QyOrmTable(parent){__initialize(); func ();}\
    QYORM_TABLE_INLINE_FUNCTION(TABLE) \

#define QYORM_TABLE_INLINE_FUNCTION(TABLE) \
    virtual QyOrmTable* constructor(QObject *parent = nullptr) { return new TABLE(parent); } \
    static inline TABLE* __constructor() { return new TABLE; } \
    QYORM_TABLE_INLINE_FUNCTION_EXTRA(TABLE) \

#define QYORM_TABLE_INLINE_FUNCTION_EXTRA(TABLE) \
    inline QList<TABLE*> sqlSelectAllAuto(){return sqlSelectAll<TABLE>();} \
    inline bool sqlSelectAllAuto(QList<TABLE*>& tables){return sqlSelectAll<TABLE>(tables);} \
    inline TABLE* sqlSelectOneAuto(){return sqlSelectOne<TABLE>();} \
    inline QList<QPointer<TABLE> > sqlSelectAllSafeAuto(){return sqlSelectAllSafe<TABLE>();} \
    inline QPointer<TABLE> sqlSelectOneSafeAuto(){return sqlSelectOneSafe<TABLE>();} \
    inline TABLE& chain(const QString& __fieldName, const QVariant &value) \
    {setFieldValue(__fieldName, value);return*this;} \
    inline TABLE* chainp(const QString& __fieldName, const QVariant &value) \
    {setFieldValue(__fieldName, value);return this;} \
    QString customString; \
    int customInteger; \
    int customBoolean; \
    int customInteger2; \
    QVariant customVariant

#define QYORM_FIELD(type,name, ...) Q_PROPERTY(QyOrmField* name READ name)\
    Q_PROPERTY(QyOrmFieldExtraInfo* __qyorm_extra_info__##name READ __qyorm_extra_info__##name) \
    inline QyOrmFieldExtraInfo* __qyorm_extra_info__##name () \
    { return new QyOrmFieldExtraInfo(#type, ##__VA_ARGS__); } \
    type name

#define QYORM_FIELD_DEFAULT(type,name,defaultValue, ...) Q_PROPERTY(QyOrmField* name READ name) \
    Q_PROPERTY(QyOrmFieldExtraInfo* __qyorm_extra_info__##name READ __qyorm_extra_info__##name) \
    inline QyOrmFieldExtraInfo* __qyorm_extra_info__##name () \
    { return new QyOrmFieldExtraInfo(#type, ##__VA_ARGS__); } \
    type name = defaultValue \

#define QYORM_REINTERPRET_CAST_ULONGLONG(Type) \
inline Type* to##Type(QVariant ptr){ \
    return reinterpret_cast<Type*>(ptr.toULongLong()); \
} \

#endif // QYORMTABLEDEFINE_H
