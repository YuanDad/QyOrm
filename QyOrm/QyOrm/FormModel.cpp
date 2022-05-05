#include "FormModel.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidgetItem>
#include <QAbstractButton>

#include "Table.h"

using namespace Qy;

FormModel::FormModel(Table* table, QObject *parent) : QObject(parent), table(table)
{
    if(table) table->setParent(this);
}

void FormModel::connect(QWidget *inputWidget, const QString &fieldName, FormModelType type, const QString& QDateTimeForceStringFormat)
{
    if(hashInputWidget.contains(fieldName)) return;
    listFieldName.append(fieldName);
    hashInputWidget.insert(fieldName, inputWidget);
    inputWidget->setProperty("FormModelType", type);
    if(!QDateTimeForceStringFormat.isEmpty()) inputWidget->setProperty("ForceString", QDateTimeForceStringFormat);
}

void FormModel::connect(QTableWidget *table, QTableWidgetItem *item, const QString &fieldName)
{
    if(hashTableItem.contains(fieldName)) return;
    connect(table, fieldName, QTableWidgetItemType);
    hashTableItem.insert(fieldName, item);
}

void FormModel::getData(const QString &fieldName, bool directChange)
{
    QWidget *inputWidget = hashInputWidget.value(fieldName);
    switch (inputWidget->property("FormModelType").toInt()) {
    case QLineEditType: {
        QString data = dynamic_cast<QLineEdit*>(inputWidget)->text();
        if(hashGetFilter.contains(QLineEditType)){
            data = hashGetFilter.value(QLineEditType)(data, inputWidget).toString();
        }
        if(directChange || data != table->getFieldValue(fieldName).toString()){
            table->setFieldValue(fieldName, data);
        }
    }break;
    case QTextEditType: {
        QString data = dynamic_cast<QTextEdit*>(inputWidget)->toPlainText();
        if(hashGetFilter.contains(QTextEditType)){
            data = hashGetFilter.value(QTextEditType)(data, inputWidget).toString();
        }
        if(directChange || data != table->getFieldValue(fieldName).toString()){
            table->setFieldValue(fieldName, data);
        }
    }break;
    case QPlainTextEditType: {
        QString data = dynamic_cast<QPlainTextEdit*>(inputWidget)->toPlainText();
        if(hashGetFilter.contains(QPlainTextEditType)){
            data = hashGetFilter.value(QPlainTextEditType)(data, inputWidget).toString();
        }
        if(directChange || data != table->getFieldValue(fieldName).toString()){
            table->setFieldValue(fieldName, data);
        }
    }break;
    case QDateTimeEditType: {
        QString QDateTimeForceStringFormat = inputWidget->property("ForceString").toString();
        QDateTimeEdit* dateTimeEdit = dynamic_cast<QDateTimeEdit*>(inputWidget);
        QDateTime data = dateTimeEdit->dateTime();
        if(hashGetFilter.contains(QDateTimeEditType)){
            data = hashGetFilter.value(QDateTimeEditType)(data, inputWidget).toDateTime();
        }
        if(QDateTimeForceStringFormat.isEmpty()){
            if(directChange || dateTimeEdit->dateTime() != table->getFieldValue(fieldName).toDateTime()){
                table->setFieldValue(fieldName, data);
            }
        }else{
            QString tdata = (data == dateTimeEdit->minimumDateTime() ? "" :
                            data.toString(QDateTimeForceStringFormat));
            if(directChange || tdata != table->getFieldValue(fieldName).toString()){
                table->setFieldValue(fieldName, tdata);
            }
        }
    }break;
    case QComboBoxType: {
        QString data = dynamic_cast<QComboBox*>(inputWidget)->currentText();
        if(hashGetFilter.contains(QComboBoxType)){
            data = hashGetFilter.value(QComboBoxType)(data, inputWidget).toString();
        }
        if(directChange || data != table->getFieldValue(fieldName).toString()){
            table->setFieldValue(fieldName, data);
        }
    }break;
    case QSpinBoxType: {
        int data = dynamic_cast<QSpinBox*>(inputWidget)->value();
        if(hashGetFilter.contains(QSpinBoxType)){
            data = hashGetFilter.value(QSpinBoxType)(data, inputWidget).toInt();
        }
        if(directChange || data != table->getFieldValue(fieldName).toInt()){
            table->setFieldValue(fieldName, data);
        }
    }break;
    case QDoubleSpinBoxType: {
        double data = dynamic_cast<QDoubleSpinBox*>(inputWidget)->value();
        if(hashGetFilter.contains(QDoubleSpinBoxType)){
            data = hashGetFilter.value(QDoubleSpinBoxType)(data, inputWidget).toDouble();
        }
        if(directChange || qFuzzyCompare(data, table->getFieldValue(fieldName).toDouble())){
            table->setFieldValue(fieldName, data);
        }
    }break;
    case QAbstractButtonType: {
        bool data = dynamic_cast<QAbstractButton*>(inputWidget)->isChecked();
        if(hashGetFilter.contains(QAbstractButtonType)){
            data = hashGetFilter.value(QAbstractButtonType)(data, inputWidget).toBool();
        }
        if(directChange || data != table->getFieldValue(fieldName).toBool()){
            table->setFieldValue(fieldName, data);
        }
    }break;
    case BlobType: {
        QByteArray& data = dynamic_cast<BlobAbstractModel*>(inputWidget)->blob;
        if(hashGetFilter.contains(BlobType)){
            data = hashGetFilter.value(BlobType)(data, inputWidget).toByteArray();
        }
        if(directChange || dynamic_cast<BlobAbstractModel*>(inputWidget)->isChanged){
            table->setFieldValue(fieldName, data);
        }
    }break;
    case QTableWidgetItemType: {
        QTableWidgetItem* item = hashTableItem.value(fieldName);
        QString data = item->text();
        if(hashGetFilter.contains(QTableWidgetItemType)){
            data = hashGetFilter.value(QTableWidgetItemType)(data, reinterpret_cast<QWidget*>(item)).toString();
        }
        if(directChange || data != table->getFieldValue(fieldName).toString()){
            table->setFieldValue(fieldName, data);
        }
    }break;
    default:break;
    }
}

void FormModel::getData()
{
    for(const QString& fieldName : listFieldName){
        getData(fieldName, true);
    }
}

void FormModel::getChangedData()
{
    for(const QString& fieldName : listFieldName){
        getData(fieldName, false);
    }
}

void FormModel::setData(const QString &fieldName, QWidget *inputWidget)
{
    switch (inputWidget->property("FormModelType").toInt()) {
    case QLineEditType: {
        QVariant data = table->getFieldValue(fieldName);
        if(hashSetFilter.contains(QLineEditType)){
            data = hashSetFilter.value(QLineEditType)(data, inputWidget).toString();
        }
        dynamic_cast<QLineEdit*>(inputWidget)->setText(data.toString());
    }break;
    case QTextEditType: {
        QVariant data = table->getFieldValue(fieldName);
        if(hashSetFilter.contains(QTextEditType)){
            data = hashSetFilter.value(QTextEditType)(data, inputWidget).toString();
        }
        dynamic_cast<QTextEdit*>(inputWidget)->setText(data.toString());
    }break;
    case QPlainTextEditType: {
        QVariant data = table->getFieldValue(fieldName);
        if(hashSetFilter.contains(QPlainTextEditType)){
            data = hashSetFilter.value(QPlainTextEditType)(data, inputWidget).toString();
        }
        dynamic_cast<QPlainTextEdit*>(inputWidget)->setPlainText(data.toString());
    }break;
    case QDateTimeEditType: {
        QString QDateTimeForceStringFormat = inputWidget->property("ForceString").toString();
        QDateTimeEdit* dateTimeEdit = dynamic_cast<QDateTimeEdit*>(inputWidget);
        if(QDateTimeForceStringFormat.isEmpty()){
            QDateTime data = table->getFieldValue(fieldName).toDateTime();
            if(hashSetFilter.contains(QDateTimeEditType)){
                data = hashSetFilter.value(QDateTimeEditType)(data, inputWidget).toDateTime();
            }
            dateTimeEdit->setDateTime(data);
        }else{
            QString dateTimeString = table->getFieldValue(fieldName).toString();
            if(dateTimeString.isEmpty()){
                dateTimeEdit->setDateTime(dateTimeEdit->minimumDateTime());
            }else{
                QDateTime dateTime = QDateTime::fromString(dateTimeString, QDateTimeForceStringFormat);
                if(hashSetFilter.contains(QDateTimeEditType)){
                    dateTime = hashSetFilter.value(QDateTimeEditType)(dateTime, inputWidget).toDateTime();
                }
                dateTimeEdit->setDateTime(dateTime);
            }
        }
    }break;
    case QComboBoxType: {
        QVariant data = table->getFieldValue(fieldName);
        if(hashSetFilter.contains(QComboBoxType)){
            data = hashSetFilter.value(QComboBoxType)(data, inputWidget).toString();
        }
        dynamic_cast<QComboBox*>(inputWidget)->setCurrentText(data.toString());
    }break;
    case QSpinBoxType: {
        int data = table->getFieldValue(fieldName).toInt();
        if(hashSetFilter.contains(QSpinBoxType)){
            data = hashSetFilter.value(QSpinBoxType)(data, inputWidget).toInt();
        }
        dynamic_cast<QSpinBox*>(inputWidget)->setValue(data);
    }break;
    case QDoubleSpinBoxType: {
        double data = table->getFieldValue(fieldName).toDouble();
        if(hashSetFilter.contains(QDoubleSpinBoxType)){
            data = hashSetFilter.value(QDoubleSpinBoxType)(data, inputWidget).toDouble();
        }
        dynamic_cast<QDoubleSpinBox*>(inputWidget)->setValue(data);
    }break;
    case QAbstractButtonType: {
        bool data = table->getFieldValue(fieldName).toBool();
        if(hashSetFilter.contains(QAbstractButtonType)){
            data = hashSetFilter.value(QAbstractButtonType)(data, inputWidget).toBool();
        }
        dynamic_cast<QAbstractButton*>(inputWidget)->setChecked(data);
    }break;
    case BlobType: {
        QByteArray data = table->getFieldValue(fieldName).toByteArray();
        if(hashSetFilter.contains(BlobType)){
            data = hashSetFilter.value(BlobType)(data, inputWidget).toByteArray();
        }
        dynamic_cast<BlobAbstractModel*>(inputWidget)->blob = data;
    }break;
    case QTableWidgetItemType: {
        QTableWidgetItem* item = hashTableItem.value(fieldName);
        QString data = table->getFieldValue(fieldName).toString();
        if(hashSetFilter.contains(QTableWidgetItemType)){
            data = hashSetFilter.value(QTableWidgetItemType)(data, reinterpret_cast<QWidget*>(item)).toString();
        }
        item->setText(data);
    }break;
    default:break;
    }
}

void FormModel::setData()
{
    for(const QString& fieldName : listFieldName){
        QWidget *inputWidget = hashInputWidget.value(fieldName);
        setData(fieldName, inputWidget);
    }
}

void FormModel::clearChanged()
{
    table->clearChanged();
}

bool FormModel::hasChange()
{
    return table->fieldChanged.size();
}

void FormModel::setReadOnly(bool isReadOnly)
{
    for(const QString& fieldName : listFieldName){
        QWidget *inputWidget = hashInputWidget.value(fieldName);
        switch (inputWidget->property("FormModelType").toInt()) {
        case QLineEditType: {
            dynamic_cast<QLineEdit*>(inputWidget)->setReadOnly(isReadOnly);
        }break;
        case QTextEditType: {
            dynamic_cast<QTextEdit*>(inputWidget)->setReadOnly(isReadOnly);
        }break;
        case QPlainTextEditType: {
            dynamic_cast<QPlainTextEdit*>(inputWidget)->setReadOnly(isReadOnly);
        }break;
        case QDateTimeEditType: {
            dynamic_cast<QDateTimeEdit*>(inputWidget)->setReadOnly(isReadOnly);
        }break;
        case QComboBoxType: {
            dynamic_cast<QComboBox*>(inputWidget)->setEnabled(isReadOnly);
        }break;
        case QSpinBoxType: {
            dynamic_cast<QSpinBox*>(inputWidget)->setReadOnly(isReadOnly);
        }break;
        case QDoubleSpinBoxType: {
            dynamic_cast<QDoubleSpinBox*>(inputWidget)->setReadOnly(isReadOnly);
        }break;
        case QAbstractButtonType: {
            dynamic_cast<QAbstractButton*>(inputWidget)->setCheckable(isReadOnly);
        }break;
        case BlobType: {
            dynamic_cast<BlobAbstractModel*>(inputWidget)->isReadonly = isReadOnly;
        }break;
        case QTableWidgetItemType: {
            QTableWidgetItem* item = hashTableItem.value(fieldName);
            Qt::ItemFlags flags = item->flags();
            flags.setFlag(Qt::ItemIsEditable, isReadOnly);
            item->setFlags(flags);
        }break;
        default:break;
        }
    }
}

void FormModel::setEnabled(bool isEnabled)
{
    for(const QString& fieldName : listFieldName){
        QWidget *inputWidget = hashInputWidget.value(fieldName);
        switch (inputWidget->property("FormModelType").toInt()) {
        case QLineEditType: {
            dynamic_cast<QLineEdit*>(inputWidget)->setEnabled(isEnabled);
        }break;
        case QTextEditType: {
            dynamic_cast<QTextEdit*>(inputWidget)->setEnabled(isEnabled);
        }break;
        case QPlainTextEditType: {
            dynamic_cast<QPlainTextEdit*>(inputWidget)->setEnabled(isEnabled);
        }break;
        case QDateTimeEditType: {
            dynamic_cast<QDateTimeEdit*>(inputWidget)->setEnabled(isEnabled);
        }break;
        case QComboBoxType: {
            dynamic_cast<QComboBox*>(inputWidget)->setEnabled(isEnabled);
        }break;
        case QSpinBoxType: {
            dynamic_cast<QSpinBox*>(inputWidget)->setEnabled(isEnabled);
        }break;
        case QDoubleSpinBoxType: {
            dynamic_cast<QDoubleSpinBox*>(inputWidget)->setEnabled(isEnabled);
        }break;
        case QAbstractButtonType: {
            dynamic_cast<QAbstractButton*>(inputWidget)->setEnabled(isEnabled);
        }break;
        case BlobType: {
            dynamic_cast<BlobAbstractModel*>(inputWidget)->isReadonly = !isEnabled;
        }break;
        case QTableWidgetItemType: {
            QTableWidgetItem* item = hashTableItem.value(fieldName);
            Qt::ItemFlags flags = item->flags();
            if(isEnabled) flags |= Qt::ItemIsEditable;
            else {
                if(flags & Qt::ItemIsEditable) flags ^= Qt::ItemIsEditable;
            }
            item->setFlags(flags);
        }break;
        default:break;
        }
    }
}

void FormModel::connectChangedSignal(std::function<void()> _lambda)
{
    for(const QString& fieldName : listFieldName){
        QWidget *inputWidget = hashInputWidget.value(fieldName);
        switch (inputWidget->property("FormModelType").toInt()) {
        case QLineEditType: {
            QObject::connect(dynamic_cast<QLineEdit*>(inputWidget), &QLineEdit::editingFinished, _lambda);
        }break;
        case QTextEditType: {
            QObject::connect(dynamic_cast<QTextEdit*>(inputWidget), &QTextEdit::textChanged, _lambda);
        }break;
        case QPlainTextEditType: {
            QObject::connect(dynamic_cast<QPlainTextEdit*>(inputWidget), &QPlainTextEdit::textChanged, _lambda);
        }break;
        case QDateTimeEditType: {
            QObject::connect(dynamic_cast<QDateTimeEdit*>(inputWidget), &QDateTimeEdit::dateTimeChanged, _lambda);
        }break;
        case QComboBoxType: {
            QObject::connect(dynamic_cast<QComboBox*>(inputWidget), &QComboBox::currentTextChanged, _lambda);
        }break;
        case QSpinBoxType: {
            QObject::connect(dynamic_cast<QSpinBox*>(inputWidget), QOverload<int>::of(&QSpinBox::valueChanged), _lambda);
        }break;
        case QDoubleSpinBoxType: {
            QObject::connect(dynamic_cast<QDoubleSpinBox*>(inputWidget), QOverload<double>::of(&QDoubleSpinBox::valueChanged), _lambda);
        }break;
        case QAbstractButtonType: {
            QObject::connect(dynamic_cast<QAbstractButton*>(inputWidget), &QAbstractButton::toggled, _lambda);
        }break;
        case BlobType: {
            dynamic_cast<BlobAbstractModel*>(inputWidget)->changedLambda = _lambda;
        }break;
        default:break;
        }
    }
}

void FormModel::clearView()
{
    listFieldNameView.clear();
    hashInputWidgetView.clear();
    hashTableItemView.clear();
}

void FormModel::setViewData()
{
    for(const QString& fieldName : listFieldNameView){
        for(QWidget *inputWidget : hashInputWidgetView.value(fieldName)){
            FormModelType type = FormModelType(inputWidget->property("FormModelType").toInt());
            if(type == QTableWidgetItemType){
                QString data = table->getFieldValue(fieldName).toString();
                for(QTableWidgetItem* item : hashTableItemView.value(fieldName)){
                    if(item){
                        if(hashSetFilter.contains(QTableWidgetItemType)){
                            data = hashSetFilter.value(QTableWidgetItemType)(data, reinterpret_cast<QWidget*>(item)).toString();
                        }
                        item->setText(data);
                    }else{
                        hashTableItemView[fieldName].removeOne(item);
                    }
                }
            }else{
                if(inputWidget){
                    setData(fieldName, inputWidget);
                }else{
                    hashInputWidgetView[fieldName].removeOne(inputWidget);
                }
            }
        }
    }
}

void FormModel::connectView(QWidget *inputWidget, const QString &fieldName, FormModelType type, const QString &QDateTimeForceStringFormat)
{
    if(!hashInputWidgetView.contains(fieldName)){
        listFieldNameView.append(fieldName);
        hashInputWidgetView.insert(fieldName, QList<QWidget*>());
    }
    hashInputWidgetView[fieldName].append(inputWidget);

    inputWidget->setProperty("FormModelType", type);
    if(!QDateTimeForceStringFormat.isEmpty()) inputWidget->setProperty("ForceString", QDateTimeForceStringFormat);
}

void FormModel::connectView(QTableWidget *table, QTableWidgetItem *item, const QString &fieldName)
{
    if(!hashTableItemView.contains(fieldName)){
        hashTableItemView.insert(fieldName, QList<QTableWidgetItem*>());
        listFieldNameView.append(fieldName);
        hashInputWidgetView.insert(fieldName, QList<QWidget*>());
        table->setProperty("FormModelType", QTableWidgetItemType);
    }
    hashTableItemView[fieldName].append(item);
    hashInputWidgetView[fieldName].append(table);
}
