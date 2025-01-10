#include "TestMainWindow.h"
#include "ui_TestMainWindow.h"
#include "TestModel.h"

#include "QyOrmFormWidget.h"
#include "QyOrmFormModel.h"

#include <QMessageBox>
#include <QDebug>

TestMainWindow::TestMainWindow(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::TestMainWindow)
{
    ui->setupUi(this);

    connect(ui->btnAdd, &QPushButton::clicked, this, &TestMainWindow::btnAdd);
    connect(ui->btnMod, &QPushButton::clicked, this, &TestMainWindow::btnMod);
    connect(ui->btnDel, &QPushButton::clicked, this, &TestMainWindow::btnDel);
    connect(ui->btnQuery, &QPushButton::clicked, this, &TestMainWindow::btnQuery);

    formFieldNames = QStringList{"uid", "pwd", "name", "age", "bmi", "born", "sex"};
    btnQuery();
}

TestMainWindow::~TestMainWindow()
{
    delete ui;
}

void TestMainWindow::btnAdd()
{
    User* user = new User;
    QyOrmFormWidget form(formFieldNames);
    form.setTable(user);
    form.createContent();
    connect(form.submitButton, &QPushButton::clicked, &form, &QyOrmFormWidget::accept);
    form.setWindowTitle("新增");
    if(form.execute()){
        form.getModel()->getData();
        user->sqlInsert();
        users.append(user);
        addRow();
    }
}

void TestMainWindow::btnMod()
{
    int row = ui->tableWidget->currentRow();
    if(row < 0) return;
    User* user = users.at(row);

    QyOrmFormWidget form({"pwd", "name", "age", "bmi", "born", "sex"});
    connect(form.submitButton, &QPushButton::clicked, &form, &QyOrmFormWidget::accept);
    form.setTable(user);
    form.createContent();
    form.getModel()->setData();
    user->clearRecordChanged();
    form.setWindowTitle("修改");
    if(form.execute()){
        form.getModel()->getChangedData();
        if(user->hasRecordChanged()){
            user->sqlUpdateChanged();
            models.at(row)->setData();
        }
    }
}

void TestMainWindow::btnDel()
{
    int row = ui->tableWidget->currentRow();
    if(row < 0) return;
    models.takeAt(row)->deleteLater();
    ui->tableWidget->removeRow(row);
    User* user = users.takeAt(row);
    user->sqlDelete();
    delete user;
}

void TestMainWindow::btnQuery()
{
    User user;
    QString content = ui->lineEdit->text();
    if(content.size()){
        QyOrmStringField* field = user.field(formFieldNames.at(ui->comboBox->currentIndex()))->toStringField();
        field->set('%' + content + '%');
        field->setExecuteOperatorLike();
    }
    user.sqlSelectAll(users);
    QyOrmFormModel::refreshModelTableWidget(ui->tableWidget, models, users.size(),
        [this](int index){return users.at(index);},
        [this](int row){addRow(row);}, 0);
}


void TestMainWindow::addRow(int row)
{
    if(row == -1) {
        row = users.size() - 1;
        ui->tableWidget->setRowCount(users.size());
    }
    QyOrmFormModel* model = new QyOrmFormModel(users.at(row));
    model->connectNewTableItem(ui->tableWidget, row, formFieldNames);
    model->appendSetFilter("born", [](const QVariant& data)->QVariant{
        return data.toDateTime().toString("yyyy-MM-dd hh:mm");
    });
    model->appendSetFilter("sex", [](const QVariant& data)->QVariant{
        return data.toBool() ? "男" : "女";
    });
    model->setData();
    models.append(model);
}
