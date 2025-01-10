#include "QyOrmFormWidget.h"
#include "QyOrmTable.h"
#include "QyOrmFormModel.h"
#include <QLabel>
#include <QLineEdit>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QDateTimeEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPushButton>
#include <QCheckBox>
#include <QLayout>
#include <QDebug>

class QyOrmFormLabel: public QLabel
{
public:
    inline explicit QyOrmFormLabel(QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags())
        :QLabel(parent,f){}
    inline explicit QyOrmFormLabel(const QString &text, QWidget *parent=nullptr, Qt::WindowFlags f=Qt::WindowFlags())
        :QLabel(text, parent,f){}
    inline void mouseReleaseEvent(QMouseEvent *ev) override{
        if(buddy()) metaObject()->invokeMethod(buddy(), "setFocus", Qt::QueuedConnection);
        QLabel::mouseReleaseEvent(ev);
    }
    void copy(const QLabel& label){
        setTextFormat(label.textFormat());
        setPixmap(*label.pixmap());
        setScaledContents(label.hasScaledContents());
        setAlignment(label.alignment());
        setWordWrap(label.wordWrap());
        setMargin(label.margin());
        setIndent(label.indent());
        setOpenExternalLinks(label.openExternalLinks());
        setTextInteractionFlags(label.textInteractionFlags());
    }
};

class QyOrmFormLayoutPrivate
{
public:
    inline QyOrmFormLayoutPrivate(const QStringList &fieldNames):
        model(new QyOrmFormModel), fieldNames(fieldNames) {}
    QyOrmTable *table = nullptr;
    QyOrmFormModel* model = nullptr;
    QStringList fieldNames;
    QList<int> fieldColumns;

    int columnCount = 3;
    int rowHeight = 30;
    int multiRowHeight = 100;
    Qt::Alignment align = Qt::AlignRight | Qt::AlignVCenter;

    std::function<QyOrmFormInputWidgetInfo(QyOrmField *)> customCreateInputWidget;

    QList<QyOrmFormLabel*> labels;
    QWidgetList inputWidgets;

    QFont font;

//    char __padding[4];
};

QyOrmFormLayout::QyOrmFormLayout(const QStringList &fieldNames) :
    __privated(new QyOrmFormLayoutPrivate(fieldNames))
{
    __privated->customCreateInputWidget = [this](QyOrmField* field){return createInputWidget(field);};
}
QyOrmFormLayout::~QyOrmFormLayout(){delete __privated;}

QyOrmFormModel *QyOrmFormLayout::getModel()
{
    return __privated->model;
}

void QyOrmFormLayout::setTable(QyOrmTable *table)
{
    __privated->table = table;
    __privated->model->setTable(table);
}

void QyOrmFormLayout::setFieldNames(const QStringList &fieldNames)
{
    __privated->fieldNames = fieldNames;
}

void QyOrmFormLayout::setFieldColumns(const QList<int> &fieldColumns)
{
    __privated->fieldColumns = fieldColumns;
}

void QyOrmFormLayout::setColumnCount(int count)
{
    __privated->columnCount = count;
}

void QyOrmFormLayout::setRowHeight(int height)
{
    __privated->rowHeight = height;
}

void QyOrmFormLayout::setMultiRowHeight(int height)
{
    __privated->multiRowHeight = height;
}

void QyOrmFormLayout::setAlignment(Qt::Alignment align)
{
    __privated->align = align;
}

void QyOrmFormLayout::setCopyLabel(const QLabel &tLabel)
{
    for(QyOrmFormLabel* label : __privated->labels){
        label->copy(tLabel);
    }
}

QGridLayout *QyOrmFormLayout::createLayout()
{
    QGridLayout* layout = new QGridLayout;
    int currentRow = 0, currentColumn = 0, columnCount = __privated->columnCount;
    for(int index = 0; index < __privated->fieldNames.size(); ++index){
        const QString& fieldName = __privated->fieldNames.at(index);
        QyOrmField* field = __privated->table->field(fieldName);
        QyOrmFormInputWidgetInfo info = __privated->customCreateInputWidget(field);
        __privated->model->connect(info.first, fieldName, info.second);

        QyOrmFormLabel* label = new QyOrmFormLabel(field->label());
        label->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
        label->setBuddy(info.first);
        label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Fixed);
        __privated->labels.append(label);
        QWidget* container = new QWidget;
        if(     info.second == QyOrmInputWidgetType::QTextEditType ||
                info.second == QyOrmInputWidgetType::QPlainTextEditType){
            container->setFixedHeight(__privated->multiRowHeight);
        }else{
            container->setFixedHeight(__privated->rowHeight);
        }
        container->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        QHBoxLayout* hlayout = new QHBoxLayout;
        hlayout->addWidget(label);
        hlayout->addWidget(info.first);
        __privated->inputWidgets.append(info.first);
        hlayout->setMargin(0);
        container->setLayout(hlayout);

        int fieldColumn = (index < __privated->fieldColumns.size() ? __privated->fieldColumns.at(index) : 1);
        if(fieldColumn > columnCount) fieldColumn = columnCount;
        if(fieldColumn + currentColumn > columnCount){
            ++currentRow;
            currentColumn = 0;
        }
        layout->addWidget(container, currentRow, currentColumn, 1, fieldColumn);
        currentColumn += fieldColumn;
    }
    layout->addItem(new QSpacerItem(20, 20, QSizePolicy::Maximum, QSizePolicy::Expanding), currentRow + 1, 0);
    return layout;
}

void QyOrmFormLayout::alignLabels()
{
    int maxWidth = 0;
    for(QyOrmFormLabel* label : __privated->labels){
        label->setMinimumWidth(0);
    }
    for(QyOrmFormLabel* label : __privated->labels){
        int width = label->minimumSizeHint().width();
        if(width > maxWidth) maxWidth = width;
    }
    for(QyOrmFormLabel* label : __privated->labels){
        label->setFixedWidth(maxWidth);
    }
}

void QyOrmFormLayout::setAx()
{
    int pointSize = __privated->font.pointSize() + 1;
    if(pointSize < 8) pointSize = 8; else if(pointSize > 64) pointSize = 64;
    setFontSize(pointSize);
}

void QyOrmFormLayout::setA_()
{
    int pointSize = __privated->font.pointSize() - 1;
    if(pointSize < 8) return;
    setFontSize(pointSize);
}

void QyOrmFormLayout::setFontSize(int pointSize)
{
    __privated->font.setPointSize(pointSize);
    foreach(QyOrmFormLabel* label, __privated->labels){
        label->setFont(__privated->font);
    }
    foreach(QWidget* w, __privated->inputWidgets){
        if(setWidgetFont && setWidgetFont(w, __privated->font)) continue;
        QComboBox* box = dynamic_cast<QComboBox*>(w);
        if(box && box->lineEdit()){
            box->lineEdit()->setFont(__privated->font);
            continue;
        }
        w->setFont(__privated->font);
    }
    alignLabels();
}

int QyOrmFormLayout::getFontSize()
{
    return __privated->font.pointSize();
}

#include <QFileDialog>
class QyOrmFormLayoutFileButton: public QPushButton, public QyOrmBlobAbstractModel
{
public:
    inline explicit QyOrmFormLayoutFileButton(QWidget *parent = nullptr):
        QPushButton("...", parent){initialize();}
    inline explicit QyOrmFormLayoutFileButton(const QString &text, QWidget *parent = nullptr):
        QPushButton(text, parent){initialize();}
    void initialize(){
        connect(this, &QPushButton::clicked, [this]{
            if(isReadonly) return;
            QString filename = QFileDialog::getOpenFileName();
            if(filename.isEmpty()) return;
            QFile file(filename);
            if(file.open(QFile::ReadOnly)){
                blob = file.readAll();
                isChanged = true;
                if(changedLambda){
                    changedLambda();
                }
                file.close();
            }
        });
    }
};
QyOrmFormInputWidgetInfo QyOrmFormLayout::createInputWidget(QyOrmField *field)
{
    switch (field->type()) {
    case QyOrmFieldType::String: return {new QLineEdit, QyOrmInputWidgetType::QLineEditType};
    case QyOrmFieldType::Integer: return {new QSpinBox, QyOrmInputWidgetType::QSpinBoxType};
    case QyOrmFieldType::Double: return {new QDoubleSpinBox, QyOrmInputWidgetType::QDoubleSpinBoxType};
    case QyOrmFieldType::DateTime: return {new QDateTimeEdit, QyOrmInputWidgetType::QDateTimeEditType};
    case QyOrmFieldType::Boolean: return {new QCheckBox, QyOrmInputWidgetType::QAbstractButtonType};
    case QyOrmFieldType::Text: return {new QTextEdit, QyOrmInputWidgetType::QTextEditType};
    case QyOrmFieldType::Blob: return {new QyOrmFormLayoutFileButton, QyOrmInputWidgetType::BlobType};
    }
    return {new QLineEdit, QyOrmInputWidgetType::QLineEditType};
}

void QyOrmFormLayout::setCustomCreateInputWidget(std::function<QyOrmFormInputWidgetInfo(QyOrmField *)> func)
{
    __privated->customCreateInputWidget = func;
}

QyOrmFormDialog::QyOrmFormDialog(QWidget *parent): QDialog(parent)
{
    buttonContainer = new QHBoxLayout;
    submitButton = new QPushButton("确认(&Y)");
    cancelButton = new QPushButton("取消(&C)");
    buttonContainer->addStretch();
    buttonContainer->addWidget(submitButton);
    buttonContainer->addWidget(cancelButton);
    connect(cancelButton, &QPushButton::clicked, this, &QDialog::close);
}

QyOrmFormDialog& QyOrmFormDialog::setContentWidget(QWidget *contentWidget)
{
    QLayout* lo = layout();
    if(lo) delete lo;
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(contentWidget);
    layout->addLayout(buttonContainer);
    setLayout(layout);
    return *this;
}

QyOrmFormDialog& QyOrmFormDialog::setContentLayout(QLayout *contentLayout)
{
    QLayout* lo = layout();
    if(lo) delete lo;
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(contentLayout);
    layout->addLayout(buttonContainer);
    setLayout(layout);
    return *this;
}

void QyOrmFormDialog::shown()
{
    setParent(parentWidget(), Qt::Sheet);
    setResult(0);
    show();
}
