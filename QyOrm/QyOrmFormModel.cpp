#include "QyOrmFormModel.h"
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QTableWidget>
#include <QTableWidgetItem>
#include <QAbstractButton>
#include <QSettings>
#include <QDebug>

#include "QyOrmTable.h"
#include "QyOrmDateTimeEdit.h"
#include "QyOrmComboBoxDataMap.h"
#include "QyOrmFormWidget.h"

void QyOrmFormModel::connect(QWidget *inputWidget, const QString &fieldName, QyOrmInputWidgetType type)
{
    if(__privated->hashInputWidget.contains(fieldName)) return;
    __privated->listFieldName.append(fieldName);
    __privated->hashInputWidget.insert(fieldName, inputWidget);
    inputWidget->setProperty("QyOrmInputWidgetType", type);
    if(__privated->isRecordInputChanged){
        inputWidget->setProperty("QyOrmFieldName", fieldName);
        connectChangedSignal(inputWidget, type, [inputWidget, this]{
            __privated->inputWidgetChangedFieldName.insert(inputWidget->property("QyOrmFieldName").toString());
        });
    }
}

void QyOrmFormModel::unconnect()
{
    __privated->hashInputWidget.clear();
    __privated->listFieldName.clear();
}

void QyOrmFormModel::connect(QTableWidget *table, QTableWidgetItem *item, const QString &fieldName)
{
    if(__privated->hashTableItem.contains(fieldName)) return;
    connect(table, fieldName, QyOrmInputWidgetType::QTableWidgetItemType);
    __privated->hashTableItem.insert(fieldName, item);
    if(__privated->isRecordInputChanged){
        if(table->property("RecordCHanged").toBool()) return;
        table->setProperty("RecordCHanged", true);
        QObject::connect(table, &QTableWidget::itemChanged, [this](QTableWidgetItem *item){
            for(auto iter = __privated->hashTableItem.begin();
                 iter != __privated->hashTableItem.end(); ++iter){
                if(__privated->hashTableItem.value(iter.key()) == item){
                    __privated->inputWidgetChangedFieldName.insert(iter.key());
                    return;
                }
            }
        });
    }
}

void QyOrmFormModel::connect(QTableWidget *table, QTableWidgetItem *item, QyOrmField &field)
{
    connect(table, item, field.fieldName());
}

void QyOrmFormModel::connectNewTableItem(QTableWidget *table, int row, const QStringList &fieldNames,
                                    int columnStart, QList<int> customColumn)
{
    if(customColumn.size() && customColumn.size() != fieldNames.size()) return;
    for(int i = 0; i < fieldNames.size(); ++i){
        int col = (customColumn.size() ? customColumn.at(i) : columnStart + i);
        QTableWidgetItem* item = new QTableWidgetItem;
        table->setItem(row, col, item);
        connect(table, item, fieldNames.at(i));
    }
}

void QyOrmFormModel::__getData(const QString &fieldName, QVariant &data,
                               QWidget *inputWidget, QyOrmInputWidgetType type)
{
    if(__privated->hashGetFilterSingle.contains(fieldName)){
        data = __privated->hashGetFilterSingle.value(fieldName)(data, inputWidget);
    }
    if(__hashGetFilter.contains(type)){
        data = __hashGetFilter.value(type)(data, inputWidget);
    }
}

bool QyOrmFormModel::__getData(const QString &fieldName, bool directChange, const QVariant &data)
{
    if(data.type() == QVariant::UserType) return false;
    QyOrmField* field = __privated->table->field(fieldName);
    if(!field) return false;
    if(data.isNull() && field->isNull()) return false;
    QVariant oldData = field->answer();
    if(directChange || data.isNull() || data != oldData){
        field->query(data);
        if(__privated->isRecordChanged){
            __privated->changedFields.append(field);
        }
        return true;
    }
    return false;
}

QVariant QyOrmFormModel::__getInputData(QWidget *inputWidget, QyOrmInputWidgetType inputWidgetType,
                                      const QString &fieldName)
{
    switch (inputWidgetType) {
    case QyOrmInputWidgetType::QLineEditType: {
        QVariant data;
        QLineEdit* lineEdit = dynamic_cast<QLineEdit*>(inputWidget);
        QString stringData = lineEdit->text();
        if(!stringData.isEmpty() && (lineEdit->inputMask().isEmpty() ||
                                     stringData.size() == lineEdit->displayText().size())){
            data = stringData;
        }
        __getData(fieldName, data, inputWidget, inputWidgetType);
        return data;
    }
    case QyOrmInputWidgetType::QTextEditType: {
        QVariant data;
        QString stringData = dynamic_cast<QTextEdit*>(inputWidget)->toPlainText();
        if(!stringData.isEmpty()) data = stringData;
        __getData(fieldName, data, inputWidget, inputWidgetType);
        return data;
    }
    case QyOrmInputWidgetType::QPlainTextEditType: {
        QVariant data;
        QString stringData = dynamic_cast<QPlainTextEdit*>(inputWidget)->toPlainText();
        if(!stringData.isEmpty()) data = stringData;
        __getData(fieldName, data, inputWidget, inputWidgetType);
        return data;
    }
    case QyOrmInputWidgetType::QDateTimeEditType: {
        QVariant data;
        QDateTime datetime;
        if(inputWidget->inherits("QyOrmDateTimeEdit")){
            datetime = dynamic_cast<QyOrmDateTimeEdit*>(inputWidget)->dateTime();
        }else{
            datetime = dynamic_cast<QDateTimeEdit*>(inputWidget)->dateTime();
        }
        if(datetime.isValid()) data = datetime;
        __getData(fieldName, data, inputWidget, inputWidgetType);
        return data;
    }
    case QyOrmInputWidgetType::QComboBoxType: {
        QVariant data;
        QString currentText = dynamic_cast<QComboBox*>(inputWidget)->currentText();
        if(!currentText.isEmpty()){
            data = currentText;
        }
        __getData(fieldName, data, inputWidget, inputWidgetType);
        return data;
    }
    case QyOrmInputWidgetType::QComboBoxIndexType: {
        QVariant data;
        QComboBox* box = dynamic_cast<QComboBox*>(inputWidget);
        if(box->currentIndex() >= 0){
            if(!box->currentText().isEmpty() || box->itemText(box->currentIndex()).isEmpty()){
                data = dynamic_cast<QComboBox*>(inputWidget)->currentIndex();
            }
        }
        __getData(fieldName, data, inputWidget, inputWidgetType);
        return data;
    }
    case QyOrmInputWidgetType::QComboBoxDataType: {
        QVariant data;
        if(dynamic_cast<QComboBox*>(inputWidget)->currentIndex() >= 0){
            data = dynamic_cast<QComboBox*>(inputWidget)->currentData();
        }
        __getData(fieldName, data, inputWidget, inputWidgetType);
        return data;
    }
    case QyOrmInputWidgetType::QSpinBoxType: {
        QVariant data = dynamic_cast<QSpinBox*>(inputWidget)->value();
        __getData(fieldName, data, inputWidget, inputWidgetType);
        return data;
    }
    case QyOrmInputWidgetType::QDoubleSpinBoxType: {
        QVariant data = dynamic_cast<QDoubleSpinBox*>(inputWidget)->value();
        __getData(fieldName, data, inputWidget, inputWidgetType);
        return data;
    }
    case QyOrmInputWidgetType::QAbstractButtonType: {
        QVariant data = dynamic_cast<QAbstractButton*>(inputWidget)->isChecked();
        __getData(fieldName, data, inputWidget, inputWidgetType);
        return data;
    }
    case QyOrmInputWidgetType::BlobType: {
        QVariant data = reinterpret_cast<QyOrmBlobAbstractModel*>(inputWidget)->blob;
        __getData(fieldName, data, inputWidget, inputWidgetType);
        return data;
    }
    case QyOrmInputWidgetType::StringAbstractType: {
        QVariant data;
        QyOrmStringAbstractModel* model = reinterpret_cast<QyOrmStringAbstractModel*>(inputWidget);
        if(!model->string.isNull()) data = model->string;
        __getData(fieldName, data, inputWidget, inputWidgetType);
        return data;
    }
    case QyOrmInputWidgetType::QTableWidgetItemType: {
        QTableWidgetItem* item = __privated->hashTableItem.value(fieldName);
        QVariant data;
        QString stringData = item->text();
        if(!stringData.isEmpty()){
            data = stringData;
        }
        __getData(fieldName, data, reinterpret_cast<QWidget*>(item), inputWidgetType);
        return data;
    }
//    default:break;
    }
    return QVariant();
}

QWidget *QyOrmFormModel::getInputWidget(const QString &fieldName)
{
    return __privated->hashInputWidget.value(fieldName, nullptr);
}

QVariant QyOrmFormModel::getInputData(const QString &fieldName, bool force)
{
    QWidget *inputWidget = __privated->hashInputWidget.value(fieldName);
    if(!force && !inputWidget->isEnabled()) return QVariant(QVariant::UserType);
    QyOrmInputWidgetType inputWidgetType = static_cast<QyOrmInputWidgetType>
            (inputWidget->property("QyOrmInputWidgetType").toInt());
    return __getInputData(inputWidget, inputWidgetType, fieldName);
}

void QyOrmFormModel::getData(const QString &fieldName, bool directChange, bool force)
{
    QWidget *inputWidget = __privated->hashInputWidget.value(fieldName);
    if(!force && !inputWidget->isEnabled()) return;
    QyOrmInputWidgetType inputWidgetType = static_cast<QyOrmInputWidgetType>
            (inputWidget->property("QyOrmInputWidgetType").toInt());
    QVariant data = __getInputData(inputWidget, inputWidgetType, fieldName);
    if(inputWidgetType == QyOrmInputWidgetType::BlobType){
        if(directChange || reinterpret_cast<QyOrmBlobAbstractModel*>(inputWidget)->isChanged){
            __privated->table->setFieldValue(fieldName, data);
        }
    }else if(inputWidgetType == QyOrmInputWidgetType::StringAbstractType){
        if(directChange || reinterpret_cast<QyOrmStringAbstractModel*>(inputWidget)->isChanged){
            __privated->table->setFieldValue(fieldName, data);
        }
    }else{
        __getData(fieldName, directChange, data);
    }
}

void QyOrmFormModel::getData(bool force)
{
    __privated->table->clearRecordChanged();
    foreach(QString fieldName, __privated->listFieldName){
        getData(fieldName, true, force);
    }
}

void QyOrmFormModel::getChangedData()
{
    __privated->table->clearRecordChanged();
    foreach(QString fieldName, __privated->listFieldName){
        getData(fieldName, false);
    }
}

void QyOrmFormModel::getInputData()
{
    foreach(QString fieldName, __privated->inputWidgetChangedFieldName){
        getData(fieldName, true);
    }
    __privated->inputWidgetChangedFieldName.clear();
}

void QyOrmFormModel::getInputChangedData()
{
    foreach(QString fieldName, __privated->inputWidgetChangedFieldName){
        getData(fieldName, false);
    }
    __privated->inputWidgetChangedFieldName.clear();
}

void QyOrmFormModel::__setData(const QString &fieldName, QVariant &data,
                               QWidget *inputWidget, QyOrmInputWidgetType inputWidgetType)
{
    if(__privated->hashSetFilterSingle.contains(fieldName)){
        data = __privated->hashSetFilterSingle.value(fieldName)(data, inputWidget);
    }
    if(__hashSetFilter.contains(inputWidgetType)){
        data = __hashSetFilter.value(inputWidgetType)(data, inputWidget);
    }
}

void QyOrmFormModel::__setInputData(QWidget *inputWidget, QyOrmInputWidgetType inputWidgetType,
                                    const QVariant &data, const QString &fieldName)
{
    QSignalBlocker blocker(inputWidget);
    switch (inputWidgetType) {
    case QyOrmInputWidgetType::QLineEditType: {
        dynamic_cast<QLineEdit*>(inputWidget)->setText(data.toString());
    }break;
    case QyOrmInputWidgetType::QTextEditType: {
        dynamic_cast<QTextEdit*>(inputWidget)->setText(data.toString());
    }break;
    case QyOrmInputWidgetType::QPlainTextEditType: {
        dynamic_cast<QPlainTextEdit*>(inputWidget)->setPlainText(data.toString());
    }break;
    case QyOrmInputWidgetType::QDateTimeEditType: {
        inputWidget->metaObject()->invokeMethod(inputWidget, "setDateTime",
                                                Q_ARG(QDateTime, data.toDateTime()));
    }break;
    case QyOrmInputWidgetType::QComboBoxType: {
        if(data.isNull()) dynamic_cast<QComboBox*>(inputWidget)->setCurrentIndex(-1);
        else{
            QyOrmComboBoxDataMap* dataMap = QyOrmComboBoxDataMap::dataMap(inputWidget);
            if(dataMap){
                dynamic_cast<QComboBox*>(inputWidget)->setCurrentIndex(dataMap->findText(data.toString()));
            }else{
                dynamic_cast<QComboBox*>(inputWidget)->setCurrentText(data.toString());
            }
        }
    }break;
    case QyOrmInputWidgetType::QComboBoxIndexType: {
        if(data.isNull()) dynamic_cast<QComboBox*>(inputWidget)->setCurrentIndex(-1);
        else dynamic_cast<QComboBox*>(inputWidget)->setCurrentIndex(data.toInt());
    }break;
    case QyOrmInputWidgetType::QComboBoxDataType: {
        if(data.isNull()) dynamic_cast<QComboBox*>(inputWidget)->setCurrentIndex(-1);
        else{
            QyOrmComboBoxDataMap* dataMap = QyOrmComboBoxDataMap::dataMap(inputWidget);
            if(dataMap){
                dynamic_cast<QComboBox*>(inputWidget)->setCurrentIndex(dataMap->findData(data.toString()));
            }else{
                dynamic_cast<QComboBox*>(inputWidget)->setCurrentIndex(
                            dynamic_cast<QComboBox*>(inputWidget)->findData(data));
            }
        }
    }break;
    case QyOrmInputWidgetType::QSpinBoxType: {
        dynamic_cast<QSpinBox*>(inputWidget)->setValue(data.toInt());
    }break;
    case QyOrmInputWidgetType::QDoubleSpinBoxType: {
        dynamic_cast<QDoubleSpinBox*>(inputWidget)->setValue(data.toDouble());
    }break;
    case QyOrmInputWidgetType::QAbstractButtonType: {
        dynamic_cast<QAbstractButton*>(inputWidget)->setChecked(data.toBool());
    }break;
    case QyOrmInputWidgetType::BlobType: {
        reinterpret_cast<QyOrmBlobAbstractModel*>(inputWidget)->blob = data.toByteArray();
    }break;
    case QyOrmInputWidgetType::StringAbstractType: {
        reinterpret_cast<QyOrmStringAbstractModel*>(inputWidget)->setData(data.toString());
    }break;
    case QyOrmInputWidgetType::QTableWidgetItemType: {
        QTableWidgetItem* item = __privated->hashTableItem.value(fieldName);
        if(item) item->setText(data.toString());
    }break;
//    default:break;
    }
}

void QyOrmFormModel::setInputData(const QString &fieldName, const QVariant &data)
{
    QWidget *inputWidget = __privated->hashInputWidget.value(fieldName);
    if(!inputWidget) return;
    __setInputData(inputWidget, static_cast<QyOrmInputWidgetType>(
                       inputWidget->property("QyOrmInputWidgetType").toInt()), data, fieldName);
}

void QyOrmFormModel::setData(const QString &fieldName, QWidget *inputWidget, bool directChange)
{
    QyOrmField* field = __privated->table->field(fieldName);
    if(!field || (!directChange && field->isNull())) return;
    QyOrmInputWidgetType inputWidgetType = static_cast<QyOrmInputWidgetType>
            (inputWidget->property("QyOrmInputWidgetType").toInt());
    QVariant data = field->answer();
    if(inputWidgetType == QyOrmInputWidgetType::QTableWidgetItemType) {
        QTableWidgetItem* item = __privated->hashTableItem.value(fieldName);
        __setData(fieldName, data, reinterpret_cast<QWidget*>(item), inputWidgetType);
    }else{
        __setData(fieldName, data, inputWidget, inputWidgetType);
    }
    if(data.type() == QVariant::UserType) return;
    __setInputData(inputWidget, inputWidgetType, data, fieldName);
}

void QyOrmFormModel::setData(const QString &fieldName, bool directChange)
{
    setData(fieldName, __privated->hashInputWidget.value(fieldName), directChange);
}

void QyOrmFormModel::setData(bool directChange)
{
    foreach(const QString& fieldName, __privated->listFieldName){
        QWidget *inputWidget = __privated->hashInputWidget.value(fieldName);
        setData(fieldName, inputWidget, directChange);
    }
}

void QyOrmFormModel::clearData(const QString &fieldName, QWidget *inputWidget)
{
    QSignalBlocker blocker(inputWidget);
    switch (inputWidget->property("QyOrmInputWidgetType").toInt()) {
    case QyOrmInputWidgetType::QLineEditType: {
        dynamic_cast<QLineEdit*>(inputWidget)->setText(QString());
    }break;
    case QyOrmInputWidgetType::QTextEditType: {
        dynamic_cast<QTextEdit*>(inputWidget)->setText(QString());
    }break;
    case QyOrmInputWidgetType::QPlainTextEditType: {
        dynamic_cast<QPlainTextEdit*>(inputWidget)->setPlainText(QString());
    }break;
    case QyOrmInputWidgetType::QDateTimeEditType: {
        inputWidget->metaObject()->invokeMethod(inputWidget, "setDateTime", Q_ARG(QDateTime, QDateTime()));
    }break;
    case QyOrmInputWidgetType::QComboBoxType:
    case QyOrmInputWidgetType::QComboBoxIndexType:
    case QyOrmInputWidgetType::QComboBoxDataType: {
        dynamic_cast<QComboBox*>(inputWidget)->setCurrentIndex(-1);
    }break;
    case QyOrmInputWidgetType::QSpinBoxType: {
        dynamic_cast<QSpinBox*>(inputWidget)->setValue(0);
    }break;
    case QyOrmInputWidgetType::QDoubleSpinBoxType: {
        dynamic_cast<QDoubleSpinBox*>(inputWidget)->setValue(0.0);
    }break;
    case QyOrmInputWidgetType::QAbstractButtonType: {
        dynamic_cast<QAbstractButton*>(inputWidget)->setChecked(false);
    }break;
    case QyOrmInputWidgetType::BlobType: {
    }break;
    case QyOrmInputWidgetType::StringAbstractType: {
        reinterpret_cast<QyOrmStringAbstractModel*>(inputWidget)->clearData();
    }break;
    case QyOrmInputWidgetType::QTableWidgetItemType: {
        __privated->hashTableItem.value(fieldName)->setText(QString());
    }break;
    default:break;
    }
}

void QyOrmFormModel::clearData(const QString &fieldName)
{
    clearData(fieldName, __privated->hashInputWidget.value(fieldName));
}

void QyOrmFormModel::clearData()
{
    foreach(QString fieldName, __privated->listFieldName){
        QWidget *inputWidget = __privated->hashInputWidget.value(fieldName);
        clearData(fieldName, inputWidget);
    }
}

void QyOrmFormModel::clearChanged()
{
    __privated->table->clearRecordChanged();
}

bool QyOrmFormModel::hasChange()
{
    return __privated->table->recordChangedFields().size();
}

void QyOrmFormModel::setComboBoxReadOnly(QComboBox *combo, bool isReadOnly)
{
    const QString styleSheetReadOnly = "QComboBox{padding-right:10px}QComboBox::drop-down{width:0px;}";
    if(isReadOnly){
        if(!combo->isEditable()){
            combo->setProperty("QyOrmReadOnlyEditable", true);
            combo->setEditable(true);
        }else{
            combo->setProperty("QyOrmReadOnlyFocusPolicy", combo->lineEdit()->focusPolicy());
        }
        combo->lineEdit()->setFocusPolicy(Qt::NoFocus);
        combo->setProperty("QyOrmReadOnlyStyleSheet", combo->styleSheet());
        combo->setStyleSheet(styleSheetReadOnly);
    }else{
        if(combo->styleSheet() != styleSheetReadOnly) return;
        if(combo->property("QyOrmReadOnlyEditable").toBool()){
            combo->setEditable(false);
        }else{
            combo->lineEdit()->setFocusPolicy(static_cast<Qt::FocusPolicy>(
                                                  combo->property("QyOrmReadOnlyFocusPolicy").toInt()));
        }
        combo->setStyleSheet(combo->property("QyOrmReadOnlyStyleSheet").toString());
    }
}

bool QyOrmFormModel::setReadOnly(const QString &fieldName, bool isReadOnly)
{
    QWidget *inputWidget = __privated->hashInputWidget.value(fieldName, nullptr);
    if(!inputWidget) return false;
    switch (inputWidget->property("QyOrmInputWidgetType").toInt()) {
    case QyOrmInputWidgetType::QLineEditType: {
        dynamic_cast<QLineEdit*>(inputWidget)->setReadOnly(isReadOnly);
    }break;
    case QyOrmInputWidgetType::QTextEditType: {
        dynamic_cast<QTextEdit*>(inputWidget)->setReadOnly(isReadOnly);
    }break;
    case QyOrmInputWidgetType::QPlainTextEditType: {
        dynamic_cast<QPlainTextEdit*>(inputWidget)->setReadOnly(isReadOnly);
    }break;
    case QyOrmInputWidgetType::QDateTimeEditType: {
        dynamic_cast<QDateTimeEdit*>(inputWidget)->setReadOnly(isReadOnly);
    }break;
    case QyOrmInputWidgetType::QComboBoxType:
    case QyOrmInputWidgetType::QComboBoxIndexType:
    case QyOrmInputWidgetType::QComboBoxDataType: {
        setComboBoxReadOnly(dynamic_cast<QComboBox*>(inputWidget), isReadOnly);
    }break;
    case QyOrmInputWidgetType::QSpinBoxType: {
        dynamic_cast<QSpinBox*>(inputWidget)->setReadOnly(isReadOnly);
    }break;
    case QyOrmInputWidgetType::QDoubleSpinBoxType: {
        dynamic_cast<QDoubleSpinBox*>(inputWidget)->setReadOnly(isReadOnly);
    }break;
    case QyOrmInputWidgetType::QAbstractButtonType: {
        dynamic_cast<QAbstractButton*>(inputWidget)->setCheckable(isReadOnly);
    }break;
    case QyOrmInputWidgetType::BlobType: {
        reinterpret_cast<QyOrmBlobAbstractModel*>(inputWidget)->isReadonly = isReadOnly;
    }break;
    case QyOrmInputWidgetType::StringAbstractType: {
        reinterpret_cast<QyOrmStringAbstractModel*>(inputWidget)->isReadonly = isReadOnly;
    }break;
    case QyOrmInputWidgetType::QTableWidgetItemType: {
        QTableWidgetItem* item = __privated->hashTableItem.value(fieldName);
        Qt::ItemFlags flags = item->flags();
        flags &= (isReadOnly ? Qt::ItemIsEditable : ~Qt::ItemIsEditable);
        item->setFlags(flags);
    }break;
    default:break;
    }
    return true;
}

void QyOrmFormModel::setReadOnly(bool isReadOnly)
{
    foreach(QString fieldName, __privated->listFieldName){
        setReadOnly(fieldName, isReadOnly);
    }
}

bool QyOrmFormModel::setEnabled(const QString &fieldName, bool isEnabled)
{
    QWidget *inputWidget = __privated->hashInputWidget.value(fieldName);
    if(!inputWidget) return false;
    switch (inputWidget->property("QyOrmInputWidgetType").toInt()) {
    case QyOrmInputWidgetType::QLineEditType: {
        dynamic_cast<QLineEdit*>(inputWidget)->setEnabled(isEnabled);
    }break;
    case QyOrmInputWidgetType::QTextEditType: {
        dynamic_cast<QTextEdit*>(inputWidget)->setEnabled(isEnabled);
    }break;
    case QyOrmInputWidgetType::QPlainTextEditType: {
        dynamic_cast<QPlainTextEdit*>(inputWidget)->setEnabled(isEnabled);
    }break;
    case QyOrmInputWidgetType::QDateTimeEditType: {
        dynamic_cast<QDateTimeEdit*>(inputWidget)->setEnabled(isEnabled);
    }break;
    case QyOrmInputWidgetType::QComboBoxType:
    case QyOrmInputWidgetType::QComboBoxIndexType:
    case QyOrmInputWidgetType::QComboBoxDataType: {
        dynamic_cast<QComboBox*>(inputWidget)->setEnabled(isEnabled);
    }break;
    case QyOrmInputWidgetType::QSpinBoxType: {
        dynamic_cast<QSpinBox*>(inputWidget)->setEnabled(isEnabled);
    }break;
    case QyOrmInputWidgetType::QDoubleSpinBoxType: {
        dynamic_cast<QDoubleSpinBox*>(inputWidget)->setEnabled(isEnabled);
    }break;
    case QyOrmInputWidgetType::QAbstractButtonType: {
        dynamic_cast<QAbstractButton*>(inputWidget)->setEnabled(isEnabled);
    }break;
    case QyOrmInputWidgetType::BlobType: {
        reinterpret_cast<QyOrmBlobAbstractModel*>(inputWidget)->isReadonly = !isEnabled;
    }break;
    case QyOrmInputWidgetType::StringAbstractType: {
        reinterpret_cast<QyOrmStringAbstractModel*>(inputWidget)->setEnabled(isEnabled);
    }break;
    case QyOrmInputWidgetType::QTableWidgetItemType: {
        QTableWidgetItem* item = __privated->hashTableItem.value(fieldName);
        Qt::ItemFlags flags = item->flags();
        if(isEnabled) flags |= Qt::ItemIsEditable;
        else {
            if(flags & Qt::ItemIsEditable) flags ^= Qt::ItemIsEditable;
        }
        item->setFlags(flags);
    }break;
    default:break;
    }
    return true;
}

void QyOrmFormModel::setEnabled(bool isEnabled)
{
    foreach(QString fieldName, __privated->listFieldName){
        setEnabled(fieldName, isEnabled);
    }
}

void QyOrmFormModel::connectChangedSignal(std::function<void()> _lambda)
{
    foreach(const QString& fieldName, __privated->listFieldName){
        QWidget *inputWidget = __privated->hashInputWidget.value(fieldName);
        connectChangedSignal(inputWidget, static_cast<QyOrmInputWidgetType>(
                                 inputWidget->property("QyOrmInputWidgetType").toInt()), _lambda);
    }
}

void QyOrmFormModel::connectChangedSignal(QWidget *inputWidget, QyOrmInputWidgetType type,
                                          std::function<void ()> _lambda)
{
    switch (type) {
    case QyOrmInputWidgetType::QLineEditType: {
        QObject::connect(dynamic_cast<QLineEdit*>(inputWidget), &QLineEdit::textChanged, _lambda);
    }break;
    case QyOrmInputWidgetType::QTextEditType: {
        QObject::connect(dynamic_cast<QTextEdit*>(inputWidget), &QTextEdit::textChanged, _lambda);
    }break;
    case QyOrmInputWidgetType::QPlainTextEditType: {
        QObject::connect(dynamic_cast<QPlainTextEdit*>(inputWidget), &QPlainTextEdit::textChanged, _lambda);
    }break;
    case QyOrmInputWidgetType::QDateTimeEditType: {
        QObject::connect(dynamic_cast<QDateTimeEdit*>(inputWidget), &QDateTimeEdit::dateTimeChanged, _lambda);
    }break;
    case QyOrmInputWidgetType::QComboBoxType:
    case QyOrmInputWidgetType::QComboBoxIndexType:
    case QyOrmInputWidgetType::QComboBoxDataType: {
        QObject::connect(dynamic_cast<QComboBox*>(inputWidget), &QComboBox::currentTextChanged, _lambda);
    }break;
    case QyOrmInputWidgetType::QSpinBoxType: {
        void (QSpinBox::*convertedSignal)(int)=&QSpinBox::valueChanged;
        QObject::connect(dynamic_cast<QSpinBox*>(inputWidget), convertedSignal, _lambda);
    }break;
    case QyOrmInputWidgetType::QDoubleSpinBoxType: {
        void (QDoubleSpinBox::*convertedSignal)(double)=&QDoubleSpinBox::valueChanged;
        QObject::connect(dynamic_cast<QDoubleSpinBox*>(inputWidget), convertedSignal, _lambda);
    }break;
    case QyOrmInputWidgetType::QAbstractButtonType: {
        QObject::connect(dynamic_cast<QAbstractButton*>(inputWidget), &QAbstractButton::toggled, _lambda);
    }break;
    case QyOrmInputWidgetType::BlobType: {
        reinterpret_cast<QyOrmBlobAbstractModel*>(inputWidget)->changedLambda = _lambda;
    }break;
    case QyOrmInputWidgetType::StringAbstractType: {
        reinterpret_cast<QyOrmStringAbstractModel*>(inputWidget)->changedLambda = _lambda;
    }break;
    default:break;
    }
}

void QyOrmFormModel::connect(QComboBox *inputWidget, const QString &fieldName)
{
    if(__privated->table){
        QyOrmField* field = __privated->table->field(fieldName);
        if(field){
            connect(inputWidget, *field);
        }
    }else{
        connect(inputWidget, fieldName, QyOrmInputWidgetType::QComboBoxType);
    }
}

#define CONNECTWIDGETFIELD(T) void QyOrmFormModel::connect(T *inputWidget, QyOrmField& field)\
{connect(inputWidget, field.fieldName(), QyOrmInputWidgetType::T##Type);}
CONNECTWIDGETFIELD(QLineEdit)
CONNECTWIDGETFIELD(QTextEdit)
CONNECTWIDGETFIELD(QPlainTextEdit)
CONNECTWIDGETFIELD(QDateTimeEdit)
void QyOrmFormModel::connect(QComboBox *inputWidget, QyOrmField& field)
{
    if(field.type() == QyOrmFieldType::Integer){
        connect(inputWidget, field.fieldName(), QyOrmInputWidgetType::QComboBoxIndexType);
    }else{
        connect(inputWidget, field.fieldName(), QyOrmInputWidgetType::QComboBoxType);
    }
}
void QyOrmFormModel::connect(QComboBox *inputWidget, QyOrmField &field, QyOrmInputWidgetType type)
{
    connect(inputWidget, field.fieldName(), type);
}
CONNECTWIDGETFIELD(QSpinBox)
CONNECTWIDGETFIELD(QDoubleSpinBox)
CONNECTWIDGETFIELD(QAbstractButton)

void QyOrmFormModel::connect(QyOrmBlobAbstractModel *inputWidget, QyOrmField& field)
{
    connect(reinterpret_cast<QWidget*>(inputWidget), field.fieldName(), QyOrmInputWidgetType::BlobType);
}

void QyOrmFormModel::connect(QyOrmStringAbstractModel *inputWidget, QyOrmField &field)
{
    connect(reinterpret_cast<QWidget*>(inputWidget), field.fieldName(), QyOrmInputWidgetType::StringAbstractType);
}

void QyOrmFormModel::clearView()
{
    __privated->listFieldNameView.clear();
    __privated->hashInputWidgetView.clear();
    __privated->hashTableItemView.clear();
}

void QyOrmFormModel::setViewData(bool directChange)
{
    foreach(QString fieldName, __privated->listFieldNameView){
        foreach(QWidget *inputWidget, __privated->hashInputWidgetView.value(fieldName)){
            QyOrmInputWidgetType type = QyOrmInputWidgetType(
                        inputWidget->property("QyOrmInputWidgetType").toInt());
            if(type == QyOrmInputWidgetType::QTableWidgetItemType){
                QString data = __privated->table->getFieldValue(fieldName).toString();
                for(QTableWidgetItem* item : __privated->hashTableItemView.value(fieldName)){
                    if(item){
                        if(__hashSetFilter.contains(QyOrmInputWidgetType::QTableWidgetItemType)){
                            data = __hashSetFilter.value(QyOrmInputWidgetType::QTableWidgetItemType)
                                    (data, reinterpret_cast<QWidget*>(item)).toString();
                        }
                        item->setText(data);
                    }else{
                        __privated->hashTableItemView[fieldName].removeOne(item);
                    }
                }
            }else{
                if(inputWidget){
                    setData(fieldName, inputWidget, directChange);
                }else{
                    __privated->hashInputWidgetView[fieldName].removeOne(inputWidget);
                }
            }
        }
    }
}

void QyOrmFormModel::connectView(QWidget *inputWidget, const QString &fieldName, QyOrmInputWidgetType type)
{
    if(!__privated->hashInputWidgetView.contains(fieldName)){
        __privated->listFieldNameView.append(fieldName);
        __privated->hashInputWidgetView.insert(fieldName, QList<QWidget*>());
    }
    __privated->hashInputWidgetView[fieldName].append(inputWidget);

    inputWidget->setProperty("QyOrmInputWidgetType", type);
}

void QyOrmFormModel::connectView(QTableWidget *table, QTableWidgetItem *item, const QString &fieldName)
{
    if(!__privated->hashTableItemView.contains(fieldName)){
        __privated->hashTableItemView.insert(fieldName, QList<QTableWidgetItem*>());
        __privated->listFieldNameView.append(fieldName);
        __privated->hashInputWidgetView.insert(fieldName, QList<QWidget*>());
        __privated->table->setProperty("QyOrmInputWidgetType", QyOrmInputWidgetType::QTableWidgetItemType);
    }
    __privated->hashTableItemView[fieldName].append(item);
    __privated->hashInputWidgetView[fieldName].append(table);
}

void QyOrmFormModel::refreshModelTableWidget(QTableWidget *tableWidget, QList<QyOrmFormModel *> &models,
                                             int tableCount, std::function<QyOrmTable *(int)> at,
                                             std::function<void (int)> addRow, int startRow)
{
    QSignalBlocker blocker(tableWidget);
    QSignalBlocker blockerModel(tableWidget->selectionModel());
    for(int i = models.size() - 1; i >= tableCount; --i) models.takeAt(i)->deleteLater();
    int rowCount = tableWidget->rowCount();
    tableWidget->setRowCount(tableCount);
    for(int i = startRow; i < rowCount && i < models.size(); ++i){
        models.at(i)->__privated->table = at(i);
        models.at(i)->setData();
    }
    for(int i = rowCount; i < tableCount; ++i){
        addRow (i);
    }
}

void QyOrmFormModel::saveQSetting(QSettings &sets)
{
    foreach(QString fieldName, __privated->listFieldName){
        sets.setValue(fieldName, getInputData(fieldName, true));
    }
}

void QyOrmFormModel::loadQSetting(QSettings &sets)
{
    foreach(QString fieldName, __privated->listFieldName){
        setInputData(fieldName, sets.value(fieldName));
    }
}
