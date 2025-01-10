#ifndef QYORMFORMWIDGET_H
#define QYORMFORMWIDGET_H

#include <QDialog>
#include "QyOrmTableDefine.h"
#include <functional>

class QLabel;
class QGridLayout;
class QHBoxLayout;
class QPushButton;
typedef QPair<QWidget*, QyOrmInputWidgetType> QyOrmFormInputWidgetInfo;

class QyOrmTable;
class QyOrmField;
class QyOrmFormModel;

class QyOrmFormLayoutPrivate;
class QyOrmFormLayout
{
public:
    explicit QyOrmFormLayout(const QStringList& fieldNames = QStringList());
    virtual ~QyOrmFormLayout();

    QyOrmFormModel* getModel();
    void setTable(QyOrmTable* table);
    void setFieldNames(const QStringList &fieldNames);
    void setFieldColumns(const QList<int> &fieldColumns);//每个字段占几列

    void setColumnCount(int count);//表格总共几列
    void setRowHeight(int height);//每行最低高度
    void setMultiRowHeight(int height);//多行输入最低高度
    void setAlignment(Qt::Alignment align);//标签的字对齐
    void setCopyLabel(const QLabel& tLabel);//用于批量属性拷贝设置

    QGridLayout* createLayout();//生成布局
public:
    void alignLabels();//把所有label对齐
    void setAx();//A+ 增大字体
    void setA_();//A- 增大字体
    void setFontSize(int pointSize);
    int getFontSize();
    std::function<bool(QWidget*, QFont)> setWidgetFont;

public:
    QyOrmFormInputWidgetInfo createInputWidget(QyOrmField* field);
    void setCustomCreateInputWidget(std::function<QyOrmFormInputWidgetInfo(QyOrmField*)>);

private:
    friend class QyOrmFormLayoutPrivate;
    QyOrmFormLayoutPrivate* __privated;
};

class QyOrmFormDialog : public QDialog
{
    Q_OBJECT
public:
    explicit QyOrmFormDialog(QWidget* parent = nullptr);
public:
    QyOrmFormDialog& setContentWidget(QWidget* contentWidget);
    QyOrmFormDialog& setContentLayout(QLayout* contentLayout);
    void shown();
    inline void display() { open(); }
    inline int execute() { return exec(); }
public:
    QHBoxLayout* buttonContainer = nullptr;
    QPushButton* submitButton = nullptr;
    QPushButton* cancelButton = nullptr;
};

class QyOrmFormWidget : public QyOrmFormDialog, public QyOrmFormLayout
{
    Q_OBJECT
public:
    explicit QyOrmFormWidget(QWidget* parent = nullptr): QyOrmFormDialog(parent), QyOrmFormLayout() {}
    explicit QyOrmFormWidget(const QStringList& fieldNames, QWidget* parent = nullptr):
        QyOrmFormDialog(parent), QyOrmFormLayout(fieldNames) {}

    inline void createContent() { setContentLayout(reinterpret_cast<QLayout*>(createLayout())); }
};

#include <QWidget>
class QyOrmStringAbstractModel: public QWidget
{
public:
    QString string;
    bool isChanged = false;
    bool isReadonly = false;
    std::function<void()> changedLambda;
public:
    inline QyOrmStringAbstractModel(QWidget* parent = nullptr): QWidget(parent){}
    virtual ~QyOrmStringAbstractModel() {}
    virtual void setData(const QString&) {}
    virtual void clearData(){}
};

#endif // QYORMFORMWIDGET_H
