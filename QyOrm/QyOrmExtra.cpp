#include "QyOrmExtra.h"
#include "QyOrmTable.h"
#include "QyOrmFormModel.h"
#include <QTableWidget>
#include <QTreeWidget>

class QyOrmShownTableWidgetprivate
{
public:
    QTableWidget *tableWidget;
    QStringList fieldNames;
    std::function<void (QyOrmFormModel *, int)> function;
    QList<QyOrmFormModel*> models;
};

QyOrmShownTableWidget::QyOrmShownTableWidget()
{
    __privated = new QyOrmShownTableWidgetprivate;
}

QyOrmShownTableWidget::~QyOrmShownTableWidget()
{
    qDeleteAll(datas);
    qDeleteAll(__privated->models);
    delete __privated;
}

void QyOrmShownTableWidget::set(QTableWidget *tableWidget, const QStringList &fieldNames)
{
    __privated->tableWidget = tableWidget;
    __privated->fieldNames = fieldNames;
}

void QyOrmShownTableWidget::setFilter(std::function<void (QyOrmFormModel *, int)> function)
{
    __privated->function = function;
}

void QyOrmShownTableWidget::refresh()
{
    QyOrmFormModel::refreshModelTableWidget(
        __privated->tableWidget, __privated->models, datas.size(),
        [this](int index){return datas.at(index);},
        [this](int row){addRow(row);});
}

void QyOrmShownTableWidget::refreshOrder()
{
    for(int row = 0; row < datas.size() && row < __privated->models.size(); ++row){
        __privated->models.at(row)->setTable(datas.at(row));
        __privated->models.at(row)->setData();
    }
}

void QyOrmShownTableWidget::refreshRow(int row)
{
    __privated->models.at(row)->setData();
}

void QyOrmShownTableWidget::appendData(QyOrmTable *data)
{
    datas.append(data);
    __privated->tableWidget->setRowCount(datas.count());
    addRow();
}

void QyOrmShownTableWidget::removeRow(int row)
{
    __privated->tableWidget->removeRow(row);
    datas.takeAt(row)->deleteLater();
    __privated->models.takeAt(row)->deleteLater();
}

void QyOrmShownTableWidget::clear()
{
    if(datas.isEmpty()) return;
    qDeleteAll(datas);
    datas.clear();
    refresh();
}

int QyOrmShownTableWidget::getCurrentRow()
{
    return __privated->tableWidget->currentRow();
}

void QyOrmShownTableWidget::updateCurrentRow(std::function<bool (QyOrmTable *)> func, bool sqlUpdateChanged)
{
    int row = __privated->tableWidget->currentRow();
    if(row < 0) return;
    if(func(datas.at(row))){
        if(sqlUpdateChanged) datas.at(row)->sqlUpdateChanged();
        __privated->models.at(row)->setData();
    }
}

void QyOrmShownTableWidget::deleteCurrentRow(std::function<bool ()> func, bool sqlDelete)
{
    int row = __privated->tableWidget->currentRow();
    if(row < 0) return;
    if(func()){
        if(sqlDelete) datas.at(row)->sqlDelete();
        removeRow(row);
    }
}

void QyOrmShownTableWidget::deleteCurrentRow(std::function<bool (QyOrmTable *)> func, bool sqlDelete)
{
    int row = __privated->tableWidget->currentRow();
    if(row < 0) return;
    if(func(datas.at(row))){
        if(sqlDelete) datas.at(row)->sqlDelete();
        removeRow(row);
    }
}

void QyOrmShownTableWidget::select(QyOrmTable &data)
{
    data.sqlSelectAll(datas);
    refresh();
    if(datas.size()) __privated->tableWidget->setCurrentCell(0, 0);
}

int QyOrmShownTableWidget::getLastOrderNumber(const QString &fieldName)
{
    if(datas.isEmpty()) return 1;
    else return datas.last()->getFieldValue(fieldName).toInt();
}

QStringList QyOrmShownTableWidget::getFieldDatas(const QString &fieldName)
{
    QStringList result;
    for(int row = 0; row < datas.size(); ++row){
        result.append(datas.at(row)->getFieldValue(fieldName).toString());
    }
    return result;
}

void QyOrmShownTableWidget::addRow(int row)
{
    if(row == -1) row = datas.size() - 1;
    QyOrmFormModel* model = new QyOrmFormModel(datas.at(row));
    model->connectNewTableItem(__privated->tableWidget, row, __privated->fieldNames);
    if(__privated->function) __privated->function(model, row);
    model->setData();
    __privated->models.append(model);
}
