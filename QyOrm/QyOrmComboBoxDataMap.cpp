#include "QyOrmComboBoxDataMap.h"
#include <QPointer>
#include <QComboBox>
#include <QHash>

class QyOrmComboBoxDataMapPrivated
{
public:
    explicit QyOrmComboBoxDataMapPrivated(QComboBox* box): box(box){}
public:
    QMetaObject::Connection connection;
    QComboBox* box;
    QStringList texts;
    QStringList datas;
    QHash<QString, int> mapText;
    QHash<QString, int> mapData;
    bool canAddItem = true;
    char __padding[7];
};

QyOrmComboBoxDataMap::QyOrmComboBoxDataMap(QComboBox *box, QObject *parent):
    __privated(new QyOrmComboBoxDataMapPrivated(box))
{
    __privated->connection = parent->connect(parent, &QObject::destroyed,
                                             [this]{ delete __privated; delete this; });
    box->setProperty("QyOrmComboBoxDataMap", reinterpret_cast<qulonglong>(this));
}

void QyOrmComboBoxDataMap::addComboBox(QComboBox *box)
{
    box->setProperty("QyOrmComboBoxDataMap", reinterpret_cast<qulonglong>(this));
}

QyOrmComboBoxDataMap::QyOrmComboBoxDataMap(QComboBox *parent):
    __privated(new QyOrmComboBoxDataMapPrivated(parent))
{
    __privated->connection = parent->connect(parent, &QComboBox::destroyed, [this]{ delete __privated; delete this; });
    parent->setProperty("QyOrmComboBoxDataMap", reinterpret_cast<qulonglong>(this));
}

void QyOrmComboBoxDataMap::release()
{
    __privated->box->setProperty("QyOrmComboBoxDataMap", 0);
    __privated->box->disconnect(__privated->connection);
    delete __privated;
    delete this;
}

QyOrmComboBoxDataMap *QyOrmComboBoxDataMap::dataMap(QObject *box)
{
    qulonglong ptr = box->property("QyOrmComboBoxDataMap").toULongLong();
    if(ptr) return reinterpret_cast<QyOrmComboBoxDataMap*>(ptr);
    else return nullptr;
}

QyOrmComboBoxDataMap *QyOrmComboBoxDataMap::setDataMap(QComboBox *box)
{
    QyOrmComboBoxDataMap* map = new QyOrmComboBoxDataMap(box);
    map->setFromComboBox();
    return map;
}

void QyOrmComboBoxDataMap::setFromComboBox()
{
    __privated->canAddItem = false;
    for(int row = 0; row < __privated->box->count(); ++row){
        append(__privated->box->itemText(row), __privated->box->itemData(row).toString());
    }
    __privated->canAddItem = true;
}

void QyOrmComboBoxDataMap::set(const QStringList &texts, const QStringList &datas)
{
    for(int row = 0; row < texts.size(); ++row){
        append(texts.at(row), datas.at(row));
    }
}

void QyOrmComboBoxDataMap::append(const QString &text, const QString &data)
{
    int row = __privated->texts.size();
    if(__privated->canAddItem) __privated->box->addItem(text, data);
    __privated->texts.append(text);
    __privated->datas.append(data);
    __privated->mapText.insert(text, row);
    __privated->mapData.insert(data, row);
}

void QyOrmComboBoxDataMap::insert(int index, const QString &text, const QString &data)
{
    if(index < 0 || index > __privated->texts.size()) return;
    __privated->box->insertItem(index, text, data);
    __privated->texts.insert(index, text);
    __privated->datas.insert(index, data);
    for(int row = index + 1; row < __privated->texts.size(); ++row){
        __privated->mapText[__privated->texts.at(row)] = row;
        __privated->mapData[__privated->datas.at(row)] = row;
    }
}

int QyOrmComboBoxDataMap::findText(const QString &text)
{
    return __privated->mapText.value(text, -1);
}

int QyOrmComboBoxDataMap::findData(const QString &data)
{
    return __privated->mapData.value(data, -1);
}

QString QyOrmComboBoxDataMap::textToData(const QString &text)
{
    int index = __privated->mapText.value(text, -1);
    if(index > -1) return __privated->datas.at(index);
    return QString();
}

QString QyOrmComboBoxDataMap::dataToText(const QString &data)
{
    int index = __privated->mapData.value(data, -1);
    if(index > -1) return __privated->datas.at(index);
    return QString();
}
