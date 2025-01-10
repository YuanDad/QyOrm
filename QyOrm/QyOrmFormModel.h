#ifndef QYORMFORMMODEL_H
#define QYORMFORMMODEL_H

#include <QObject>
#include <QHash>
#include <QSet>
#include <QVariant>
#include <functional>
#include "QyOrmTableDefine.h"

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
class QyOrmStringAbstractModel;
class QSettings;

class QyOrmTable;
class QyOrmField;

typedef std::function<QVariant(const QVariant&, QWidget*)> QyOrmFormModelFilter;
typedef std::function<QVariant(const QVariant&)> QyOrmFormModelFilterSingle;
#define VoidQVariant QVariant::UserType
#define QVariantVoid QVariant::UserType
#define UserTypeQVariant QVariant::UserType
#define QVariantUserType QVariant::UserType

class QyOrmBlobAbstractModel
{
public:
    QByteArray blob;
    bool isChanged = false;
    bool isReadonly = false;
    std::function<void()> changedLambda;
};

class QyOrmFormModelPrivate{
public:

    inline QyOrmFormModelPrivate() {}
    inline QyOrmFormModelPrivate(QyOrmTable* table): table(table) {}
    QyOrmTable* table = nullptr;
    bool isRecordChanged = false;
    bool isRecordInputChanged = false;
    char __padding[6];
    QList<QyOrmField*> changedFields;
    QStringList listFieldName;
    QSet<QString> inputWidgetChangedFieldName;
    QHash<QString, QWidget*> hashInputWidget;
    QHash<QString, QTableWidgetItem*> hashTableItem;
    QHash<QString, QyOrmFormModelFilter> hashGetFilterSingle;
    QHash<QString, QyOrmFormModelFilter> hashSetFilterSingle;

    QStringList listFieldNameView;
    QHash<QString, QList<QWidget*>> hashInputWidgetView;
    QHash<QString, QList<QTableWidgetItem*>> hashTableItemView;
};

class QyOrmFormModel : public QObject
{
    Q_OBJECT
public:
signals:
public slots:

public:
    explicit inline QyOrmFormModel(QObject *parent = nullptr):
        QObject(parent), __privated(new QyOrmFormModelPrivate){}
    explicit inline QyOrmFormModel(QyOrmTable* table, QObject *parent = nullptr):
        QObject(parent), __privated(new QyOrmFormModelPrivate(table)){}
    inline void setTable(QyOrmTable* tTable){ __privated->table = tTable; }
    inline QyOrmTable* getTable() { return __privated->table; }
    inline void takeOver() { reinterpret_cast<QObject*>(__privated->table)->setParent(this); }
    inline QyOrmTable* take() { reinterpret_cast<QObject*>(__privated->table)->setParent(nullptr);
                                return __privated->table; }

public:
    void connect(QWidget* inputWidget, const QString& fieldName, QyOrmInputWidgetType type);
    void unconnect();
    void connect(QTableWidget* table, QTableWidgetItem* item, const QString &fieldName);
    void connect(QTableWidget* table, QTableWidgetItem* item, QyOrmField& field);
    void connectNewTableItem(QTableWidget* table, int row, const QStringList &fieldNames,
                             int columnStart = 0, QList<int> customColumn = QList<int>());

public:
    void __getData(const QString &fieldName, QVariant& data, QWidget *inputWidget, QyOrmInputWidgetType type);
    bool __getData(const QString &fieldName, bool directChange, const QVariant &data);
    inline void enableRecordChanged(bool is) { __privated->isRecordChanged = is; }
    QVariant __getInputData(QWidget *inputWidget, QyOrmInputWidgetType inputWidgetType, const QString &fieldName);
    QWidget* getInputWidget(const QString &fieldName);
    QVariant getInputData(const QString &fieldName, bool force = false);
    void getData(const QString &fieldName, bool directChange, bool force = false);
    void getData(bool force = false);
    void getChangedData();
    inline void enableRecordInputChanged(bool is) { __privated->isRecordInputChanged = is; }
    void getInputData();
    void getInputChangedData();
    inline QList<QyOrmField*> getChangedFields() { return __privated->changedFields; }
    inline void clearChangedFields() { __privated->changedFields.clear(); }
    void __setData(const QString &fieldName, QVariant& data, QWidget *inputWidget, QyOrmInputWidgetType type);
    void __setInputData(QWidget *inputWidget, QyOrmInputWidgetType inputWidgetType,
                        const QVariant& data, const QString& fieldName = QString());
    void setInputData(const QString& fieldName, const QVariant& data);
    void setData(const QString &fieldName, QWidget* inputWidget, bool directChange = true);
    void setData(const QString &fieldName, bool directChange = true);
    inline void setData(const char* fieldName, QWidget* inputWidget, bool directChange = true)
    {setData(QString(fieldName), inputWidget, directChange);}
    inline void setData(const char* fieldName, bool directChange = true)
    {setData(QString(fieldName), directChange);}
    void setData(bool directChange = true);//false:如果是null则不改变
    void clearData(const QString &fieldName, QWidget* inputWidget);
    void clearData(const QString &fieldName);
    void clearData();

    void clearChanged();
    bool hasChange();

    static void setComboBoxReadOnly(QComboBox* combo, bool isReadOnly = true);
    bool setReadOnly(const QString& fieldName, bool isReadOnly = true);
    void setReadOnly(bool isReadOnly = true);
    bool setEnabled(const QString& fieldName, bool isEnabled);
    void setEnabled(bool isEnabled);
    inline void setEditabled(bool isEditabled) { setEnabled(isEditabled); }
    void connectChangedSignal(std::function<void()> _lambda);
    void connectChangedSignal(QWidget *inputWidget, QyOrmInputWidgetType type, std::function<void()> _lambda);

    template<class Type>inline Type* getTable(){
        static_assert(std::is_base_of<QyOrmTable, Type>::value, "T must base of QyOrmTable!");
        return dynamic_cast<Type*>(__privated->table);
    }

public:
    inline void connect(QLineEdit* inputWidget, const QString& fieldName)
    {connect(reinterpret_cast<QWidget*>(inputWidget), fieldName, QyOrmInputWidgetType::QLineEditType);}
    inline void connect(QTextEdit* inputWidget, const QString& fieldName)
    {connect(reinterpret_cast<QWidget*>(inputWidget), fieldName, QyOrmInputWidgetType::QTextEditType);}
    inline void connect(QPlainTextEdit* inputWidget, const QString& fieldName)
    {connect(reinterpret_cast<QWidget*>(inputWidget), fieldName, QyOrmInputWidgetType::QPlainTextEditType);}
    inline void connect(QDateTimeEdit* inputWidget, const QString& fieldName)
    {connect(reinterpret_cast<QWidget*>(inputWidget), fieldName, QyOrmInputWidgetType::QDateTimeEditType);}
    void connect(QComboBox* inputWidget, const QString& fieldName);
    inline void connect(QSpinBox* inputWidget, const QString& fieldName)
    {connect(reinterpret_cast<QWidget*>(inputWidget), fieldName, QyOrmInputWidgetType::QSpinBoxType);}
    inline void connect(QDoubleSpinBox* inputWidget, const QString& fieldName)
    {connect(reinterpret_cast<QWidget*>(inputWidget), fieldName, QyOrmInputWidgetType::QDoubleSpinBoxType);}
    inline void connect(QAbstractButton* inputWidget, const QString& fieldName)
    {connect(reinterpret_cast<QWidget*>(inputWidget), fieldName, QyOrmInputWidgetType::QAbstractButtonType);}
    inline void connect(QyOrmBlobAbstractModel* inputWidget, const QString& fieldName)
    {connect(reinterpret_cast<QWidget*>(inputWidget), fieldName, QyOrmInputWidgetType::BlobType);}
    inline void connect(QyOrmStringAbstractModel* inputWidget, const QString& fieldName)
    {connect(reinterpret_cast<QWidget*>(inputWidget), fieldName, QyOrmInputWidgetType::StringAbstractType);}

    void connect(QLineEdit* inputWidget, QyOrmField& field);
    void connect(QTextEdit* inputWidget, QyOrmField& field);
    void connect(QPlainTextEdit* inputWidget, QyOrmField& field);
    void connect(QDateTimeEdit* inputWidget, QyOrmField& field);
    void connect(QComboBox* inputWidget, QyOrmField& field);
    void connect(QComboBox* inputWidget, QyOrmField& field, QyOrmInputWidgetType type);
    void connect(QSpinBox* inputWidget, QyOrmField& field);
    void connect(QDoubleSpinBox* inputWidget, QyOrmField& field);
    void connect(QAbstractButton* inputWidget, QyOrmField& field);
    void connect(QyOrmBlobAbstractModel* inputWidget, QyOrmField& field);
    void connect(QyOrmStringAbstractModel* inputWidget, QyOrmField& field);

public:
    QHash<QyOrmInputWidgetType, QyOrmFormModelFilter> __hashGetFilter;
    QHash<QyOrmInputWidgetType, QyOrmFormModelFilter> __hashSetFilter;
    inline void appendGetFilter(QyOrmInputWidgetType type, QyOrmFormModelFilter filter)
    { __hashGetFilter.insert(type, filter); }
    inline void appendSetFilter(QyOrmInputWidgetType type, QyOrmFormModelFilter filter)
    { __hashSetFilter.insert(type, filter); }
    inline void appendGetFilter(QyOrmInputWidgetType type, QyOrmFormModelFilterSingle filter)
    { __hashGetFilter.insert(type, [filter](const QVariant& variant, QWidget*){return filter(variant);}); }
    inline void appendSetFilter(QyOrmInputWidgetType type, QyOrmFormModelFilterSingle filter)
    { __hashSetFilter.insert(type, [filter](const QVariant& variant, QWidget*){return filter(variant);}); }

    inline void appendGetFilter(const QString &fieldName, QyOrmFormModelFilter filter)
    { __privated->hashGetFilterSingle.insert(fieldName, filter); }
    inline void appendSetFilter(const QString &fieldName, QyOrmFormModelFilter filter)
    { __privated->hashSetFilterSingle.insert(fieldName, filter); }
    inline void appendGetFilter(const QString &fieldName, QyOrmFormModelFilterSingle filter)
    { __privated->hashGetFilterSingle.insert(fieldName, [filter](const QVariant& variant, QWidget*){return filter(variant);}); }
    inline void appendSetFilter(const QString &fieldName, QyOrmFormModelFilterSingle filter)
    { __privated->hashSetFilterSingle.insert(fieldName, [filter](const QVariant& variant, QWidget*){return filter(variant);}); }

public:
    void clearView();
    void setViewData(bool directChange = true);
    void connectView(QWidget* inputWidget, const QString& fieldName, QyOrmInputWidgetType type);
    void connectView(QTableWidget* table, QTableWidgetItem* item, const QString &fieldName);

public:
    static void refreshModelTableWidget(QTableWidget* tableWidget, QList<QyOrmFormModel*>& models,
                                        int tableCount, std::function<QyOrmTable*(int)> at,
                                        std::function<void(int)> addRow, int startRow = 0);

public:
    void saveQSetting(QSettings& sets);
    void loadQSetting(QSettings& sets);

private:
    friend class QyOrmFormModelPrivate;
    QyOrmFormModelPrivate* __privated = nullptr;
};

#endif // QYORMFORMMODEL_H
