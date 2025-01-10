#include <QApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QDebug>
#include "TestModel.h"
#include <QyOrmDatabase.h>

void setDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("test.db");
    db.open();
    db.exec("PRAGMA foreign_keys=ON");
    QyOrmDatabase::setDatabase(db);
}

bool testCreate()
{
    qInfo() << User().sqlCreateTableString();
    return User().sqlCreateTable();
}

void testInsert()
{
    User user;
    user.uid = "001";
    user.pwd = "001";
    user.name = "张三";
    qInfo() << user.sqlInsertString();
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
}

void testUpdate()
{
    User user;
    user.uid = "001";
    user.name = "张三三";
    qInfo() << user.sqlUpdateChangedString();
    user.sqlUpdateChanged();
}

void testDelete()
{
    User user;
    user.uid = "001";
    qInfo() << user.sqlDeleteString();
    user.sqlDelete();
}

void testRestoreDelete()
{
    User user;
    user.uid = "001";
    user.pwd = "001";
    user.name = "张三";
    user.sqlInsert();
}

void testSelect()
{
    User temp;
    QList<User*> users = temp.sqlSelectAllAuto();
    qInfo() << "一共有" << users.size() << "个人";
    foreach(User* user, users){
        qInfo() << "uid:" << user->uid.get();
        qInfo() << "pwd:" << user->pwd.get();
        qInfo() << "name:" << user->name.get() << "\n";
    }
    qDeleteAll(users);

    qInfo() << "查单个人";
    temp.name = "%五";
    temp.name.setExecuteOperatorLike();
    User* wangwu = temp.sqlSelectOneAuto();
    qInfo() << "uid:" << wangwu->uid.get();
    qInfo() << "pwd:" << wangwu->pwd.get();
    qInfo() << "name:" << wangwu->name.get() << "\n";
    delete wangwu;
}

#include "TestMainWindow.h"
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    setDatabase();

    //创建表
    if(testCreate()){

        //增
        testInsert();

        //改
        testUpdate();

        //删
        testDelete();
        testRestoreDelete();

        //查
        testSelect();
    }

    TestMainWindow w;
    w.show();

    return a.exec();
}
