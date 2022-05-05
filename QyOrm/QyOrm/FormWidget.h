#ifndef FORMWIDGET_H
#define FORMWIDGET_H

#include <QWidget>

class QLabel;
class QLineEdit;
class QEventLoop;
class QPushButton;
class QHBoxLayout;
namespace Qy{
class Field;
class Table;
class FormModel;

class TableFormAgreement{
public:
    //row设置为0，则按照setField的添加顺序流式摆放Item，row则失效
    inline explicit TableFormAgreement(int col = 1, int row = 0): row(row), col(col) {}
    ~TableFormAgreement();
    class Item{
    public:
        inline Item(const QString& fieldLabel, int row, int col, int rowSpan = 1, int colSpan = 1):
            fieldLabel(fieldLabel), row(row), col(col), rowSpan(rowSpan), colSpan(colSpan) {}
        QString fieldLabel;
        int row;//第几行
        int col;//第几列
        int rowSpan = 1;//占据几行
        int colSpan = 1;//占据几列
        QWidget* fieldWidget = nullptr;
    };
    inline void setField(const QString& fieldLabel, int row = 0, int col = 0, int rowSpan = 1, int colSpan = 1){
        items.insert(fieldLabel, new Item(fieldLabel, row, col, rowSpan, colSpan));
        names.append(fieldLabel);
    }
    inline void setField(Item* item) { items.insert(item->fieldLabel, item); names.append(item->fieldLabel); }
    void setField(Table* table);
    inline void setField(Table& table) { setField(&table); }
    inline void setWidget(const QString& fieldLabel, QWidget* widget) { if(items.contains(fieldLabel)) items[fieldLabel]->fieldWidget = widget; }
    inline void setRightInputWidth(int width) { rightInputWidth = width; }
    inline void setHideRow(int row) { hideRow = row; }

    QLayout* layout();
public:
    int row;
    int col;
    int rightInputWidth = 100;
    int hideRow;//从这行开始默认隐藏
    QHash<QString, Item*> items;
    QStringList names;
};

class FormWidget : public QWidget
{
    Q_OBJECT
public:
    explicit inline FormWidget(QWidget *parent = nullptr): QWidget(parent){}
    ~FormWidget();
    FormWidget& initialize(const Table& table, QList<Table*> foreigns = QList<Table*>());

public:
    void setLabels(const QStringList& labelStrings);

    void embed(Table* table, bool isUpdate);
    bool insert(Table* table);
    bool update(Table* table);

    FormWidget& defaultNotNullEmpty();
    inline FormWidget& setAgreement(TableFormAgreement* _) { agreement = _; return *this; }

public:
    bool exec();

signals:
    void submit();
    void notNullEmpty(QList<Qy::Field*>);

public slots:

public:
    QEventLoop* loop = nullptr;
    QList<QLabel*> labels;
    QList<Table*> foreigns;
    QLineEdit* primaryKeyEdit = nullptr;
    FormModel* model;
    bool m_embed = true;
    QPushButton* okButton;
    QPushButton* noButton;
    QHBoxLayout* buttonLayout;
    TableFormAgreement* agreement = nullptr;

protected:
    void closeEvent(QCloseEvent*);
};
}

#endif // FORMWIDGET_H
