#ifndef QYORMDATABASE_H
#define QYORMDATABASE_H

#include <QSqlDatabase>

class QyOrmDatabase
{
public:
    enum DatabaseType{
        Common = 0, PostgreSQL, DB2, Oracle,
    };
public:
    //设置数据库连接模板
    static void setDatabase(QSqlDatabase db);
    //清除当前线程的数据库连接
    static void clearCurrentDatabase();
    //多线程获取当前线程的数据库可用连接
    static QSqlDatabase getValidDatabase();
    //获取连接模板的驱动类型
    static QString driverName();
    static bool isDriverSqlite(){ return driverName() == "QSQLITE"; }
    static bool isDriverPgsql(){ return driverName() == "QPSQL"; }

public:
    //设置测试数据库连接是否正常的sql语句
    static void setTryConnectSql(const QString& sql);
    static void setTryConnectSql(DatabaseType type);
    static QString getTryConnectSql();
    //数据库连接最后一次访问时间为基准判断超时
    static void setTimeOut(time_t second = 0);
    static time_t getTimeOut();
    //数据库连接失败重试最大等待时间
    static void setMaxConnectionTimeOut(unsigned long second = 0);
    static unsigned long getMaxConnectionTimeOut();

    //获取当前线程数据库连接最后一条语句的数据信息
    static QString getLastError();
    //在第一次创建数据库连接时判断是否开启数据库定时检查连接状态
    static void setTimerCheck(int msec = 500000);
};
using QyOrmDatabaseType = QyOrmDatabase::DatabaseType;

#endif // QYORMDATABASE_H

