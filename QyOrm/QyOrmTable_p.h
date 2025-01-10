#ifndef QYORMTABLE_P_H
#define QYORMTABLE_P_H

#include <QString>
#include <QVariant>
#include <QList>
#include <QSet>
#include "QyOrmFieldExtraInfo.h"
#include "QyOrmTableExtraInfo.h"

class QyOrmField;
class QyOrmTable;
class QyOrmTablePrivate
{
public:
    QyOrmField* primaryKey = nullptr;
    bool atHeap = false;
    QString lastError;
    QString lastSelectError;
public:
    QyOrmTableExtraInfo* extra = nullptr;
    QList<QyOrmField*> listField;
    QList<QyOrmField*> singleSelectFields;
    QList<QyOrmField*> listDynamicField;
    QHash<QString, QyOrmField*> hashField;
public:
    bool isAssigning = true;//正在赋初值
    QList<QyOrmBlobField*> blobFields;//需要用QSqlQuery::bindValue转换
    QSet<QyOrmField*> recordChangedFields;
    QList<QyOrmField*> orderByFields;
    QString seniorSelectSql;
    int limit = 0;
    int offset = 0;
    QList<QyOrmField*> joinOnFields;
    QString forceSelectSql;
    QString updateWhereSql;//update的时候加的条件
public:
    friend class DynamicTable;
    static QHash<QString, QyOrmTableExtraInfo*> hashTableNameHashTableExtraInfoReady;
    static QHash<QString, QHash<QString, QyOrmFieldExtraInfo*>> hashTableNameHashFieldExtraInfoReady;
    static QHash<QString, QHash<QString, QyOrmField*>> hashTableNameHashFieldReady;
};

#endif // QYORMTABLE_P_H
