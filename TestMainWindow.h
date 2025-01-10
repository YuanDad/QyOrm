#ifndef TESTMAINWINDOW_H
#define TESTMAINWINDOW_H

#include <QWidget>

namespace Ui {
class TestMainWindow;
}

class QyOrmFormModel;
class User;
class TestMainWindow : public QWidget
{
    Q_OBJECT

public:
    explicit TestMainWindow(QWidget *parent = nullptr);
    ~TestMainWindow();

public slots:
    void btnAdd();
    void btnMod();
    void btnDel();
    void btnQuery();

private:
    void addRow(int row = -1);

private:
    Ui::TestMainWindow *ui;
    QList<QyOrmFormModel*> models;
    QList<User*> users;
    QStringList formFieldNames;
};

#endif // TESTMAINWINDOW_H
