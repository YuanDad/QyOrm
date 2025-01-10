#ifndef QYORMREMOTEDATABASE_H
#define QYORMREMOTEDATABASE_H

#include <QList>
#include <functional>

class QJsonDocument;
class QJsonArray;
class QJsonValue;
class QVariant;
using QVariantList = QList<QVariant>;

class QyOrmRemoteDatabase
{
public:
    QyOrmRemoteDatabase();
    static QyOrmRemoteDatabase* instance();

public:
    std::function<QString(QString)> execute;
    std::function<QList<QVariantList>(QString, QString, bool)> selectAll;
    std::function<QList<QVariantList>(QString, QString)> selectRowWithHeader;
    std::function<QVariantList(QString, QString)> selectRow;
    std::function<QVariantList(QString, QString)> selectColumn;
    std::function<QVariant(QString, QString)> selectOne;

public:
    static QJsonDocument fromJson(const QByteArray&);
    static QList<QVariantList> fromJsonArrayArray(const QJsonDocument&);
    static QVariantList fromJsonArray(const QJsonDocument&);
    static QList<QVariantList> fromJsonArrayArrayDirect(const QByteArray&);
    static QVariantList fromJsonArrayDirect(const QByteArray&);

public:
    static QByteArray toJson(const QJsonDocument&);
    static QJsonDocument toJson(const QList<QVariantList>&);
    static QJsonDocument toJson(const QVariantList&);
    static QByteArray toJsonDirect(const QList<QVariantList>& v);
    static QByteArray toJsonDirect(const QVariantList& v);

private:
    static QyOrmRemoteDatabase* __instance;
};

#endif // QYORMREMOTEDATABASE_H
