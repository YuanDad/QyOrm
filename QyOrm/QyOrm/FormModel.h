#ifndef FORMMODEL_H
#define FORMMODEL_H

#include <QObject>
#include <QHash>
#include <functional>

class QLineEdit;
class QTextEdit;
class QPlainTextEdit;
class QDateTimeEdit;
class QComboBox;
class QSpinBox;
class QDoubleSpinBox;
class QTableWidget;
class QTableWidgetItem;
class QAbstractButton;

namespace Qy{
class Table;

typedef std::function<QVariant(const QVariant&, QWidget*)> FormModelFilter;

class BlobAbstractModel
{
public:
    QByteArray blob;
    bool isChanged = false;
    bool isReadonly = false;
    std::function<void()> changedLambda;
};

enum FormModelType{
    QLineEditType = 0, QTextEditType, QPlainTextEditType, QDateTimeEditType, QComboBoxType,
    QSpinBoxType, QDoubleSpinBoxType, QAbstractButtonType, QTableWidgetItemType, BlobType
};
class FormModel : public QObject
{
    Q_OBJECT

public:
    explicit FormModel(Table* table, QObject *parent = nullptr);
    void connect(QWidget* inputWidget, const QString& fieldName, FormModelType type, const QString& QDateTimeForceStringFormat = "");
    void connect(QTableWidget* table, QTableWidgetItem* item, const QString &fieldName);
    void getData(const QString &fieldName, bool directChange);
    void getData();
    void getChangedData();
    void setData(const QString &fieldName, QWidget* inputWidget);
    void setData();

    void clearChanged();
    bool hasChange();

    void setReadOnly(bool isReadOnly = true);
    void setEnabled(bool isEnabled);
    inline void setEditabled(bool isEditabled) { setEnabled(isEditabled); }
    void connectChangedSignal(std::function<void()> _lambda);

    template<class Type>inline Type* getTable(){
        static_assert(std::is_base_of<Table, Type>::value, "T must base of Table!");
        return dynamic_cast<Type*>(table);
    }

signals:

public slots:

public:
    Table* table;
    QStringList listFieldName;
    QHash<QString, QWidget*> hashInputWidget;
    QHash<QString, QTableWidgetItem*> hashTableItem;
    QHash<FormModelType, FormModelFilter> hashGetFilter;
    QHash<FormModelType, FormModelFilter> hashSetFilter;
    inline void appendGetFilter(FormModelType type, FormModelFilter filter) { hashGetFilter.insert(type, filter); }
    inline void appendSetFilter(FormModelType type, FormModelFilter filter) { hashSetFilter.insert(type, filter); }

public:
    void clearView();
    void setViewData();
    void connectView(QWidget* inputWidget, const QString& fieldName, FormModelType type, const QString& QDateTimeForceStringFormat = "");
    void connectView(QTableWidget* table, QTableWidgetItem* item, const QString &fieldName);
    QStringList listFieldNameView;
    QHash<QString, QList<QWidget*>> hashInputWidgetView;
    QHash<QString, QList<QTableWidgetItem*>> hashTableItemView;
};

}
#endif // FORMMODEL_H
