#include "QyOrmDatabase.h"
#include <QHash>
#include <QThread>
#include <QSqlQuery>
#include <QSqlError>
#include <QTimer>
#include <QDebug>

static QString tryConnectSql = "SELECT 1";
static QSqlDatabase databaseTemplate;
static time_t timeOut = 600;
static unsigned long maxConnectionTimeOut = 43200;
static int timerCheck = 0;
static QHash<QString, QSqlDatabase> hashThreadSqlDatabase;

inline QString getThreadId()
{
    return QString::number(reinterpret_cast<quintptr>(QThread::currentThreadId()));
}

void QyOrmDatabase::setDatabase(QSqlDatabase db)
{
    if(databaseTemplate.isValid()){
        QString connectName = ::getThreadId();
        QSqlDatabase temp = QSqlDatabase::database(connectName);
        if(temp.isOpen()) temp.close();
        QSqlDatabase::removeDatabase(connectName);
        hashThreadSqlDatabase.remove(connectName);
    }
    databaseTemplate = db;
}

inline bool tryConnect(QSqlDatabase& db)
{
    return QSqlQuery(db).exec(tryConnectSql);
}

void tryOpen(QSqlDatabase db)
{
    if(!tryConnect(db)){
        if(db.isOpen()){
            db.close();
        }
        db.open();
    }
}

void retryConnect(QSqlDatabase db)
{
    unsigned long second = 5;
    while(!db.isOpen()){
        qWarning() << "Open" << db.databaseName() << "Database error,"
                   << db.lastError().text()
                   << ",retry in" << QString::number(second) << "seconds.";
        QThread::sleep(second);
        if(db.open()) break;
        second <<= 1;
        if(second > maxConnectionTimeOut){
            exit(0);
        }
    }
}

void QyOrmDatabase::clearCurrentDatabase()
{
    QString threadId = ::getThreadId();
    if(hashThreadSqlDatabase.contains(threadId)){
        hashThreadSqlDatabase.take(threadId).close();
        QSqlDatabase::removeDatabase(threadId);
    }
}

QSqlDatabase getThreadDatabase()
{
    QString threadId = ::getThreadId();

    if(hashThreadSqlDatabase.contains(threadId)){
        return hashThreadSqlDatabase.value(threadId);
    }else{
        QSqlDatabase tDatabase;
        if(QSqlDatabase::contains(threadId)){
            tDatabase = QSqlDatabase::database(threadId);
        }else{
            tDatabase = QSqlDatabase::cloneDatabase(databaseTemplate, threadId);
        }
        QObject::connect(QThread::currentThread(), &QThread::finished, [threadId]{
            hashThreadSqlDatabase.remove(threadId);
            QSqlDatabase::removeDatabase(threadId);
        });
        hashThreadSqlDatabase.insert(threadId, tDatabase);
        if(timerCheck){
            QTimer* timer = new QTimer(QThread::currentThread());
            timer->setInterval(timerCheck);
            timer->connect(timer, &QTimer::timeout, []{
                tryOpen(::getThreadDatabase());
            });
        }
        return tDatabase;
    }
}

QSqlDatabase QyOrmDatabase::getValidDatabase()
{
    static QHash<QString, time_t> hashThreadSqlDatabaseLastTime;

    QString threadId = ::getThreadId();
    QSqlDatabase db = ::getThreadDatabase();

    time_t lastTime = hashThreadSqlDatabaseLastTime.value(threadId, 0), now = time(nullptr);
    if(!lastTime){
        hashThreadSqlDatabaseLastTime.insert(threadId, now);
        db.open();
    }else if(!db.isOpen()){
        db.open();
    }else if(!timeOut && (now - lastTime) > timeOut){
        tryOpen(db);
    }
    hashThreadSqlDatabaseLastTime[threadId] = now;

    return db;
}

QString QyOrmDatabase::driverName()
{
    return databaseTemplate.driverName();
}

void QyOrmDatabase::setTryConnectSql(const QString &sql)
{
    tryConnectSql = sql;
}

/*
hsqldb	select 1 from INFORMATION_SCHEMA.SYSTEM_USERS
Oracle	select 1 from dual
DB2	select 1 from sysibm.sysdummy1
postgresql	select version()
derby	values 1
*/
void QyOrmDatabase::setTryConnectSql(QyOrmDatabase::DatabaseType type)
{
    switch (type) {
    case Common: tryConnectSql = "SELECT 1"; break;
    case PostgreSQL: tryConnectSql = "SELECT VERSION()"; break;
    case DB2: tryConnectSql = "SELECT 1 FROM SYSIBM.SYSDUMMY1"; break;
    case Oracle: tryConnectSql = "SELECT 1 FROM DUAL"; break;
    }
}

QString QyOrmDatabase::getTryConnectSql()
{
    return tryConnectSql;
}

void QyOrmDatabase::setTimeOut(time_t second)
{
    timeOut = second;
}

time_t QyOrmDatabase::getTimeOut()
{
    return timeOut;
}

void QyOrmDatabase::setMaxConnectionTimeOut(unsigned long second)
{
    maxConnectionTimeOut = second;
}

unsigned long QyOrmDatabase::getMaxConnectionTimeOut()
{
    return maxConnectionTimeOut;
}

QString QyOrmDatabase::getLastError()
{
    return ::getThreadDatabase().lastError().text();
}

void QyOrmDatabase::setTimerCheck(int msec)
{
    timerCheck = msec;
}
