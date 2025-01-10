#ifndef QYORMEXTRA_H
#define QYORMEXTRA_H

#include <QList>
#include <functional>

class QTreeWidget;
class QTableWidget;
class QStringList;
class QyOrmTable;

class QyOrmFormModel;
class QyOrmShownTableWidgetprivate;
class QyOrmShownTableWidget
{
public:
    QyOrmShownTableWidget();
    ~QyOrmShownTableWidget();
    void set(QTableWidget* tableWidget, const QStringList& fieldNames);
    void setFilter(std::function<void(QyOrmFormModel*, int)> function);
    void refresh();
    void refreshOrder();//对datas进行的排序,需要重新绑定model和刷新视图
    void refreshRow(int row);
    void appendData(QyOrmTable* data);
    void removeRow(int row);
    void clear();

    int getCurrentRow();
    void updateCurrentRow(std::function<bool(QyOrmTable*)> func, bool sqlUpdateChanged = false);
    template <class Type> inline void updateCurrentRow(std::function<bool(Type*)> func, bool sqlUpdate = false)
    {updateCurrentRow([func, sqlUpdate](QyOrmTable*data){return func(dynamic_cast<Type*>(data));}, sqlUpdate);}
    void deleteCurrentRow(std::function<bool()> func, bool sqlDelete = false);
    void deleteCurrentRow(std::function<bool(QyOrmTable*)> func, bool sqlDelete = false);
    template <class Type> inline void deleteCurrentRow(std::function<bool(Type*)> func, bool sqlDelete = false)
    {deleteCurrentRow([func, sqlDelete](QyOrmTable*data){return func(dynamic_cast<Type*>(data));}, sqlDelete);}

public:
    void select(QyOrmTable &data);
    inline void select(QyOrmTable *data) { select(*data); }
    int getLastOrderNumber(const QString& fieldName = "orderNumber");
    QStringList getFieldDatas(const QString& fieldName);

public:
    QList<QyOrmTable*> datas;
    inline QyOrmTable* getData(int row) { if(row < 0 || row >= datas.size()) return nullptr; return datas.at(row); }
    template <class Type> inline Type* getData(int row) { return dynamic_cast<Type*>(getData(row)); }

private:
    void addRow(int row = -1);
    QyOrmShownTableWidgetprivate* __privated;
};

#endif // QYORMEXTRA_H
