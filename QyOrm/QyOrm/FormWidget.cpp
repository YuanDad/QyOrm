#include "FormWidget.h"
#include "Table.h"
#include "FormModel.h"
#include <QFormLayout>
#include <QLabel>
#include <QComboBox>
#include <QLineEdit>
#include <QPlainTextEdit>
#include <QDateEdit>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QCheckBox>
#include <QFileDialog>
#include <QPushButton>
#include <QMessageBox>
#include <QEventLoop>
#include <QTimer>
#include <QPainter>
#include <limits>

using namespace Qy;

class QyFormWidgetFileButton: public QPushButton, public BlobAbstractModel
{
public:
    explicit QyFormWidgetFileButton(const QByteArray& bytes){
        if(bytes.isEmpty()){
            setText("点击选择文件");
        }else{
            setText("...");
        }
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
                setText("...");
            }else{
                QMessageBox::about(nullptr, "错误", filename + "文件打不开.");
            }
        });
    }
};

class QyFormWidgetSpinBox: public QSpinBox
{
public:
    inline explicit QyFormWidgetSpinBox(QWidget* parent = nullptr): QSpinBox(parent){}
    QLineEdit* getLineEdit() { return lineEdit(); }
};

FormWidget& FormWidget::initialize(const Table &table, QList<Table*> foreigns)
{
    if(!labels.isEmpty()) return *this;

    this->foreigns = foreigns;
    model = new FormModel(nullptr, this);
    QFormLayout* formLayout = nullptr;
    if(!agreement) formLayout = new QFormLayout;
    for(Field* field: table.listField){
        QLabel* label = new QLabel(field->getLabel());
        QWidget* inputWidget = nullptr;
        labels.append(label);
        if(field->hasValueList()){
            QComboBox* comboBox = new QComboBox;
            inputWidget = comboBox;
            comboBox->addItems(field->fieldValueList());
            model->connect(inputWidget, field->fieldName, FormModelType::QComboBoxType);
            inputWidget->setProperty("type", "valueList");
            inputWidget->setProperty("field", reinterpret_cast<qulonglong>(field));
        }else{
            switch(field->fieldType){
            case FieldType::Foreign: {
                if(foreigns.isEmpty()) break;
                QComboBox* comboBox = new QComboBox;
                qulonglong ptrForeign = field->answer().toULongLong();
                int index = 0, finded = -1;
                for(Table* foreign : foreigns){
                    if(finded == -1 && ptrForeign == reinterpret_cast<qulonglong>(foreign)){
                        finded = index;
                    }
                    comboBox->addItem(foreign->primaryKey->answer().toString());
                    ++index;
                }
                if(finded != -1) comboBox->setCurrentIndex(finded);
                inputWidget = comboBox;
                model->connect(inputWidget, field->fieldName, FormModelType::QComboBoxType);
                inputWidget->setProperty("type", "foreign");
                inputWidget->setProperty("field", reinterpret_cast<qulonglong>(field));
            }break;
            case FieldType::String: {
                QLineEdit* edit = new QLineEdit(field->answer().toString());
                if(!primaryKeyEdit && field->primaryKey){
                    primaryKeyEdit = edit;
                }
                inputWidget = edit;
                model->connect(inputWidget, field->fieldName, FormModelType::QLineEditType);
            }break;
            case FieldType::Integer: {
                QyFormWidgetSpinBox* spinBox = new QyFormWidgetSpinBox;
                spinBox->setRange(std::numeric_limits<int>::min(), std::numeric_limits<int>::max());
                spinBox->setValue(field->answer().toInt());
                inputWidget = spinBox;
                if(!primaryKeyEdit && field->primaryKey){
                    primaryKeyEdit = spinBox->getLineEdit();
                }
                model->connect(inputWidget, field->fieldName, FormModelType::QSpinBoxType);
            }break;
            case FieldType::Double: {
                QDoubleSpinBox* spinBox = new QDoubleSpinBox;
                spinBox->setRange(std::numeric_limits<double>::min(), std::numeric_limits<double>::max());
                spinBox->setValue(field->answer().toDouble());
                inputWidget = spinBox;
                model->connect(inputWidget, field->fieldName, FormModelType::QDoubleSpinBoxType);
            }break;
            case FieldType::DateTime: {
                inputWidget = new QDateTimeEdit(field->answer().toDateTime());
                model->connect(inputWidget, field->fieldName, FormModelType::QDateTimeEditType, dynamic_cast<DateTimeField*>(field)->getValidFormat());
            }break;
            case FieldType::Boolean: {
                QCheckBox* edit = new QCheckBox;
                edit->setChecked(field->answer().toBool());
                inputWidget = edit;
                model->connect(inputWidget, field->fieldName, FormModelType::QAbstractButtonType);
            }break;
            case FieldType::Text: {
                QPlainTextEdit* edit = new QPlainTextEdit;
                edit->setPlainText(field->answer().toString());
                inputWidget = edit;
                model->connect(inputWidget, field->fieldName, FormModelType::QPlainTextEditType);
            }break;
            case FieldType::Blob: {
                inputWidget = new QyFormWidgetFileButton(field->answer().toByteArray());
                model->connect(inputWidget, field->fieldName, FormModelType::BlobType);
            }break;
            }
        }
        model->appendSetFilter(FormModelType::QComboBoxType, [](const QVariant& data, QWidget*inputWidget)->QVariant{
            if(inputWidget->property("type") == "foreign"){
                return reinterpret_cast<ForeignField*>(inputWidget->property("field").toULongLong())->get()->primaryKey->answer();
            }else{
                Field* field = reinterpret_cast<Field*>(inputWidget->property("field").toULongLong());
                if(field->canHandleValueList()){
                    return QVariant(field->setFieldValueList(data));
                }else{
                    return data;
                }
            }
        });
        model->appendGetFilter(FormModelType::QComboBoxType, [this](const QVariant& data, QWidget*inputWidget)->QVariant{
            if(inputWidget->property("type") == "foreign"){
                return QVariant(reinterpret_cast<qulonglong>(
                                    this->foreigns.at(dynamic_cast<QComboBox*>(inputWidget)->currentIndex())));
            }else{
                Field* field = reinterpret_cast<Field*>(inputWidget->property("field").toULongLong());
                if(field->canHandleValueList()){
                    return QVariant(field->getFieldValueList(dynamic_cast<QComboBox*>(inputWidget)->currentIndex()));
                }else{
                    return data;
                }
            }
        });
        if(agreement){
            agreement->setWidget(field->getLabel(), inputWidget);
        }else{
            formLayout->addRow(label, inputWidget);
        }
    }

    buttonLayout = new QHBoxLayout;
    buttonLayout->addSpacerItem(new QSpacerItem(10, 10));
    okButton = new QPushButton("确认");
    noButton = new QPushButton("取消");
    buttonLayout->addStretch(10);
    buttonLayout->addWidget(okButton);
    buttonLayout->addWidget(noButton);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    if(agreement){
        mainLayout->addLayout(agreement->layout());
    }else{
        mainLayout->addLayout(formLayout);
    }
    mainLayout->addSpacerItem(new QSpacerItem(10, 10));
    mainLayout->addLayout(buttonLayout);
    setLayout(mainLayout);

    setWindowTitle(table.tableLabel.isEmpty() ? table.tableName : table.tableLabel);

    return *this;
}

FormWidget::~FormWidget()
{
    if(!foreigns.isEmpty()){
        qDeleteAll(foreigns);
    }
}

void FormWidget::setLabels(const QStringList &labelStrings)
{
    if(labelStrings.size() != labels.size()) return;
    for(int i = 0; i < labels.size(); ++i){
        labels.at(i)->setText(labelStrings.at(i));
    }
}

void FormWidget::embed(Table *table, bool isUpdate)
{
    model->table = table;
    if(isUpdate){
        model->setData();
        primaryKeyEdit->setReadOnly(true);
    }
}

bool FormWidget::insert(Table* table)
{
    m_embed = false;
    embed(table, false);
    return exec();
}

bool FormWidget::update(Table *table)
{
    m_embed = false;
    embed(table, true);
    return exec();
}

FormWidget& FormWidget::defaultNotNullEmpty()
{
    connect(this, &FormWidget::notNullEmpty, [](QList<Qy::Field*> fields){
        if(fields.isEmpty()) return;
        QString errorFieldLabels = fields.first()->getLabel();
        for(int i = 1; i < fields.size(); ++i){
            errorFieldLabels += "、" + fields.at(i)->getLabel();
        }
        QMessageBox::warning(nullptr, "提示", errorFieldLabels + " 不可为空.");
    });
    return *this;
}

bool FormWidget::exec()
{
    loop = new QEventLoop(this);
    connect(okButton, &QPushButton::clicked, [this]{
        model->getChangedData();
        QList<Qy::Field*> fields;
        for(Field* field : model->table->listField){
            if(field->isNull && field->notNull){
                fields.append(field);
            }
        }
        if(fields.isEmpty()){
            loop->exit(1);
        }else{
            emit notNullEmpty(fields);
        }
    });
    connect(noButton, &QPushButton::clicked, loop, &QEventLoop::quit);

    setWindowModality(Qt::WindowModal);
    show();
    if(loop->exec(QEventLoop::DialogExec)){
        close();
        return true;
    }
    close();
    return false;
}

void FormWidget::closeEvent(QCloseEvent *)
{
    loop->exit();
}



TableFormAgreement::~TableFormAgreement()
{
    for(const QString& name : names){
        delete items[name];
    }
}

void TableFormAgreement::setField(Table *table)
{
    for(Field* field : table->listField){
        setField(field->getLabel());
    }
}

static QList<QLabel*> lineFormLabels;
static QHBoxLayout* makeLineForm(const QString& labelString, QWidget* widget)
{
    QHBoxLayout* hbox = new QHBoxLayout;
    QLabel* label = new QLabel(labelString);
    label->setSizePolicy(QSizePolicy::Maximum, QSizePolicy::Maximum);
    hbox->addWidget(label);
    hbox->addWidget(widget);
    lineFormLabels.append(label);
    return hbox;
}

class CollapseButton: public QAbstractButton
{
public:
    QString collapseString = "▴▴▴";
    QString expandString = "▾▾▾";
    bool isCollapsed = false;
    char __padding[7];
    QWidgetList widgets;
    CollapseButton(){
        setText(expandString);
        connect(this, &QAbstractButton::clicked, [this]{
            isCollapsed = !isCollapsed;
            setText(isCollapsed ? collapseString : expandString);
            for(QWidget* widget : widgets){
                widget->setVisible(!isCollapsed);
            }
        });
    }
    inline void addWidget(QWidget* w) { widgets.append(w); }
    virtual QSize sizeHint() const override;
    virtual QSize minimumSizeHint() const override { return sizeHint(); }
    void paintEvent(QPaintEvent *) override{
        QPainter painter(this);
        painter.drawText(rect(), Qt::AlignCenter, text());
    }
};
QSize CollapseButton::sizeHint() const
{
    QString s(text());
    bool empty = s.isEmpty();
    if (empty)
        s = QStringLiteral("XX");
    return fontMetrics().size(Qt::TextShowMnemonic, s);
}

QLayout *TableFormAgreement::layout()
{
    lineFormLabels.clear();
    QGridLayout* grid = new QGridLayout;
    CollapseButton* collapse = nullptr;
    bool appendCollapse = false;
    if(hideRow > 0) collapse = new CollapseButton;
    if(row == 0){
        int x = 0, y = 0, maxX = 1;
        for(const QString& name : names){
            Item* item = items[name];
            if(!item->fieldWidget || item->colSpan > col) continue;
            if(y + item->colSpan > col){
                x += maxX;
                if(hideRow > 0 && x == hideRow){
                    ++x;
                    grid->addWidget(collapse, hideRow, 0);
                    appendCollapse = true;
                }
                y = 0;
                maxX = 1;
            }
            QHBoxLayout* hbox = makeLineForm(item->fieldLabel, item->fieldWidget);
            grid->addLayout(hbox, x, y, item->rowSpan, item->colSpan);
            if(appendCollapse) {
                collapse->addWidget(hbox->itemAt(0)->widget());
                collapse->addWidget(item->fieldWidget);
            }
            if(item->rowSpan > maxX) maxX = item->rowSpan;
            y += item->colSpan;
            if(y >= col){
                x += maxX;
                if(hideRow > 0 && x == hideRow){
                    ++x;
                    grid->addWidget(collapse, hideRow, 0);
                    appendCollapse = true;
                }
                y = 0;
                maxX = 1;
            }
        }
    }else{
        for(const QString& name : names){
            Item* item = items[name];
            QHBoxLayout* hbox = makeLineForm(item->fieldLabel, item->fieldWidget);
            grid->addLayout(hbox, item->row, item->col, item->rowSpan, item->colSpan);
            if(appendCollapse) {
                hbox->itemAt(0)->widget()->hide();
                item->fieldWidget->hide();
                collapse->addWidget(hbox->itemAt(0)->widget());
                collapse->addWidget(item->fieldWidget);
            }
        }
    }
    QTimer::singleShot(20, [this, grid, collapse]{
        int maxWidth = 0;
        for(QLabel* label : lineFormLabels){
            if(label->width() > maxWidth){
                maxWidth = label->width();
            }
        }
        while(!lineFormLabels.isEmpty()){
            lineFormLabels.takeLast()->setMinimumWidth(maxWidth);
        }
        grid->parentWidget()->setMinimumWidth((maxWidth + rightInputWidth) * col);
        grid->parentWidget()->setMinimumWidth(0);
        if(collapse) collapse->click();
    });
    return grid;
}
