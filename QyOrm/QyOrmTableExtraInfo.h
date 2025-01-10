#ifndef QYORMTABLEEXTRAINFO_H
#define QYORMTABLEEXTRAINFO_H

#include <QStringList>
#include <QHash>

class QyOrmTableExtraInfo
{
public:
    QString tableName;
    QString tableLabel;
    QString primaryKeyFieldName;
    QStringList listFieldName;
    QHash<QString, int> hashFieldIndex;
};

#endif // QYORMTABLEEXTRAINFO_H
