# QyOrm

### 使用说明

##### 1.   示例:
```cpp
#ifndef MODEL_H
#define MODEL_H

#include "QyOrmDynamicTable.h"

class User : public QyOrmTable
{
    Q_OBJECT
public:
    QYORM_TABLE(User);

    QYORM_FIELD(QyOrmStringField, uid, QyOrmFieldConstraint::PrimaryKey);
    QYORM_FIELD(QyOrmStringField, pwd, QyOrmFieldConstraint::NotNull, "密码");
    QYORM_FIELD(QyOrmStringField, name, "姓名");
    QYORM_FIELD(QyOrmStringField, test);
};

#endif
```

```cpp
#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include "TestModel.h"
#include <QyOrmDatabase.h>

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("test.db");
    db.open();
    db.exec("PRAGMA foreign_keys=ON");
    QyOrmDatabase::setDatabase(db);

    //创建表
    qInfo() << User().sqlCreateTableString();
    User().sqlCreateTable();

    //增
    User user;
    user.uid = "001";
    user.pwd = "001";
    user.name = "张三";
    qInfo() << user.sqlInsertString();
    user.sqlInsert();

    //改
    user.name = "张三三";
    qInfo() << user.sqlUpdateChangedString();
    user.sqlUpdateChanged();

    //删
    qInfo() << user.sqlDeleteString();
    user.sqlDelete();

    //增加测试用例
    user.sqlInsert();
    user.uid = "002";
    user.pwd = "002";
    user.name = "李四";
    user.sqlInsert();
    user.uid = "003";
    user.pwd = "003";
    user.name = "王五";
    user.sqlInsert();
    user.uid = "004";
    user.pwd = "004";
    user.name = "贼六";
    user.sqlInsert();

    //查
    User temp;
    auto users = temp.sqlSelectAllAuto();
    qInfo() << "一共有" << users.size() << "个人";
    foreach(User* user, users){
        qInfo() << "uid:" << user->uid.get();
        qInfo() << "pwd:" << user->pwd.get();
        qInfo() << "name:" << user->name.get() << "\n";
    }
    qDeleteAll(users);

    //查单个人
    temp.name = "%五";
    temp.name.setExecuteOperatorLike();
    User* wangwu = temp.sqlSelectOneAuto();
    qInfo() << "uid:" << wangwu->uid.get();
    qInfo() << "pwd:" << wangwu->pwd.get();
    qInfo() << "name:" << wangwu->name.get() << "\n";
    delete wangwu;

    return 0;
}
```

##### 2.   类型

| 说明   | 语法                     |
|--------|------------------------|
| 字符串   | String          |
| 整数   | Integer       |
| 小数   | Double        |
| 日期 | DateTime    |
| 布尔   | Boolean     |
| 文本 | Text |
| 二进制流(比如:图像) | Blob |

##### 3.   约束

| 说明   | 语法                     |
|--------|------------------------|
| 普通字段   | Plain(默认值)          |
| 非空字段   | NotNull       |
| 唯一字段   | Unique        |
| 唯一非空字段 | NotNullUnique    |
| 主键字段   | PrimaryKey     |
| 自增主键字段 | AutoPrimaryKey |

##### 4.   QYORM_FIELD参数介绍
QYORM_FIELD的第一个参数肯定得是类型,第二个参数肯定得是字段名,剩下的参数会传给QyOrmFieldExtraInfo这个类
QyOrmFieldExtraInfo的第一个参数是默认的不用管,还可以接收两个参数(约束、中文描述),先后顺序都可以