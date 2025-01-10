#ifndef MODEL_H
#define MODEL_H

#include "QyOrmDynamicTable.h"

class User : public QyOrmTable
{
    Q_OBJECT
public:
    QYORM_TABLE(User);

    QYORM_FIELD(QyOrmStringField, uid, QyOrmFieldConstraint::PrimaryKey, "账号");
    QYORM_FIELD(QyOrmStringField, pwd, QyOrmFieldConstraint::NotNull, "密码");
    QYORM_FIELD(QyOrmStringField, name, "姓名");
    QYORM_FIELD(QyOrmIntegerField, age, "年龄");
    QYORM_FIELD(QyOrmDoubleField, bmi, "BMI指数");
    QYORM_FIELD(QyOrmDateTimeField, born, "出生日期");
    QYORM_FIELD(QyOrmBooleanField, sex, "性别(女/男)");
};

#endif
