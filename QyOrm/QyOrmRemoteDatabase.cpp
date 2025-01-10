#include "QyOrmRemoteDatabase.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonValue>
#include <QVariant>
#include "QyOrmConfigure.h"

QyOrmRemoteDatabase::QyOrmRemoteDatabase()
{
    QyOrmConfigure::enabledRemoteDatabase = true;
    __instance = this;
}

QyOrmRemoteDatabase* QyOrmRemoteDatabase::__instance = nullptr;
QyOrmRemoteDatabase *QyOrmRemoteDatabase::instance()
{
    return __instance;
}

QJsonDocument QyOrmRemoteDatabase::fromJson(const QByteArray & bytes)
{
    QJsonParseError error;
    return QJsonDocument::fromJson(bytes, &error);
}

QList<QVariantList> QyOrmRemoteDatabase::fromJsonArrayArray(const QJsonDocument & document)
{
    QList<QVariantList> result;
    QJsonArray array = document.array();
    for(int row = 0; row < array.size(); ++row){
        result.append(array.at(row).toArray().toVariantList());
    }
    return result;
}

QVariantList QyOrmRemoteDatabase::fromJsonArray(const QJsonDocument & document)
{
    return document.array().toVariantList();
}

QList<QVariantList> QyOrmRemoteDatabase::fromJsonArrayArrayDirect(const QByteArray & bytes)
{
    return fromJsonArrayArray(fromJson(bytes));
}

QVariantList QyOrmRemoteDatabase::fromJsonArrayDirect(const QByteArray & bytes)
{
    return fromJsonArray(fromJson(bytes));
}

QByteArray QyOrmRemoteDatabase::toJson(const QJsonDocument & document)
{
    return document.toJson(QJsonDocument::Compact);
}

QJsonDocument QyOrmRemoteDatabase::toJson(const QList<QVariantList> & variantListList)
{
    QJsonDocument document;
    QJsonArray array;
    for(const QVariantList & variantList : variantListList){
        array.append(QJsonArray::fromVariantList(variantList));
    }
    document.setArray(array);
    return document;
}

QJsonDocument QyOrmRemoteDatabase::toJson(const QVariantList & variantList)
{
    QJsonDocument document;
    document.setArray(QJsonArray::fromVariantList(variantList));
    return document;
}

QByteArray QyOrmRemoteDatabase::toJsonDirect(const QList<QVariantList> &v)
{
    return toJson(toJson(v));
}

QByteArray QyOrmRemoteDatabase::toJsonDirect(const QVariantList &v)
{
    return toJson(toJson(v));
}
