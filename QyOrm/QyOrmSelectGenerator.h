#ifndef QYORMSELECTGENERATOR_H
#define QYORMSELECTGENERATOR_H

#include <QStringList>
#include <QVariantList>
#include "QyOrmTable.h"

class QyOrmDatabaseSelectGeneratorPrivate;
class QyOrmDatabaseSelectGenerator
{
public:
    explicit QyOrmDatabaseSelectGenerator(const QString& sql);
    ~QyOrmDatabaseSelectGenerator();

    const QVariantList& fieldNames();
    int columnCount() const;
    const QString& error() const;
    const QString& sql() const;

    QVariantList yield() const;
    const QList<QVariantList>& yields(int count) const;
    const QList<QVariantList>& yieldAll() const;

private:
    QyOrmDatabaseSelectGeneratorPrivate* __privated;
    friend class QyOrmSelectGenerator;
};

class QyOrmTable;
class QyOrmSelectGeneratorPrivate;
class QyOrmSelectGenerator
{
public:
    explicit QyOrmSelectGenerator(QyOrmTable &selectObejct,
                                  const QString& sql = QString(),
                                  bool disorder = false);
    explicit QyOrmSelectGenerator(QyOrmTable* selectObejct, bool autoDelete = false,
                                  const QString& sql = QString(),
                                  bool disorder = false);
    ~QyOrmSelectGenerator();

    int rowCount() const;
    bool __yield(const QVariantList& rowData, QyOrmTable*) const;
    QyOrmTable* makeTable() const;
    QyOrmTable* __yield(const QVariantList& rowData) const;
    bool yield(QyOrmTable*) const;
    QyOrmTable* yield() const;
    QList<QyOrmTable*> yields(int count) const;
    QList<QyOrmTable*> yieldAll() const;
public:
    template<class Type>inline Type* makeTable() const {
        return makeTable()->dynamicCast<Type>();
    }
    template<class Type>inline Type* yield() const {
        QyOrmTable* table = yield();
        return table ? table->dynamicCast<Type>() : nullptr;
    }
public:
    const QyOrmDatabaseSelectGenerator* generator();
    const QyOrmTable *selectObejct();
private:
    QyOrmSelectGeneratorPrivate* __privated;

};

#endif // QYORMSELECTGENERATOR_H
