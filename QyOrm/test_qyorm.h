#ifndef TEST_QYORM_H
#define TEST_QYORM_H


#include "QyOrm/Table.h"

class User : public Qy::Table
{
    Q_OBJECT
public:
    TABLE(User)

    PRIMARYKEY_LABEL(Qy::StringField,uid, "编号")
    NOTNULL_LABEL(Qy::StringField,pwd, "密码")
    NOTNULL_LABEL(Qy::StringField,name, "名字")
    FIELD_LABEL(Qy::StringField,phone, "电话")
    FIELD_LABEL(Qy::StringField,pos, "地址")
    FIELD_LABEL(Qy::IntegerField,height, "身高")
    NOTNULL_LABEL(Qy::StringField,sex, "性别")

    FIELD_VALUE_LIST(sex)
};

class Info : public Qy::Table
{
    Q_OBJECT
public:
    TABLE(Info)

    PRIMARYKEY(Qy::StringField, iid)
    FOREIGN(Qy::ForeignField, uid, User)
    NOTNULL(Qy::StringField, date)
};

#endif // TEST_QYORM_H
