#include <QApplication>
#include <QFile>
#include <QSqlDatabase>
#include <QSqlQuery>
#include "test_qyorm.h"
#include "QyOrm/FormWidget.h"

static QStringList getSexStringList()
{
    static QStringList sexs = {"男", "女"};
    return sexs;
}
RegisterFieldValueListOnly(User, sex, getSexStringList);

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);


    Qy::Table::db = QSqlDatabase::addDatabase("QSQLITE");
    QSqlDatabase& db = Qy::Table::db;
    db.setDatabaseName("test.db");

    RegisterForeign(User);//外键需要注册一下

#if 0
//这里是创建表结构，和插入一些测试数据
    DF << "创建User表" << User().create();
    User user;
    DF << "创建Info表" << Info().setField("uid", reinterpret_cast<qintptr>(&user)).create();
#elif 01
//通过自动表单插入一条数据
    User user;
    user.tableLabel = "添加用户";
    Qy::FormWidget form;

    Qy::TableFormAgreement agreement(2);//2代表每行放两列
    agreement.setRightInputWidth(200);//设置输入框的长度为200
    agreement.setField(user);//设置表单的字段
    agreement.setHideRow(2);//设置默认隐藏第二行及以下
    form.setAgreement(&agreement);//表单界面应用该样式

    form.initialize(user);//初始化表单
    if(form.insert(&user)){
        user.insert();
    }

#elif 0
//这里是全部查询
    for(User* user : QyOrmSelect(User())){//查询
        DF << user->uid.get() << user->pwd.get() << user->name.get();
    }

#elif 0
//这里是条件查询
    for(User* user : QyOrmSelect(User().setField("uid", QString("002")))){//查询
        DF << user->uid.get() << user->pwd.get() << user->name.get();
    }

#elif 0
//这里是查询一条数据后修改
    User* user = QyOrmSelectOne(User().setField("uid", QString("001")));
    if(user){
//        DF << user->uid.get() << user->pwd.get() << user->name.get();
        DF << user->name.set("赵一");//修改并打印结果
        DF << user->isInstance();
//        delete user;//可以delete，也可以不delete，留在hash表里用于QyOrmInstance搜索
    }

//这里是查询一条实例
    User* userInstance = QyOrmInstance(User, "001");
    if(userInstance){
        DF << userInstance->uid.get() << userInstance->pwd.get() << userInstance->name.get();
    }

#elif 0
//这里是插入一条数据
    User* user = new User;
    user->uid = "005";
    user->pwd = "005";
    user->name = "孙五";
    DF << user->insert();//插入

#elif 0
//这里是删除一个记录
    User* user = QyOrmSelectOne(User().setField("uid", QString("005")));
    if(user){
        DF << user->uid.get() << user->pwd.get() << user->name.get();
        DF << user->remove();//删除
    }

#elif 0
//这里是查询一条数据和插入一条有外键表的数据
    User* user = QyOrmSelectOne(User().setField("uid", QString("002")));
    if(user){
        DF << user->uid.get() << user->pwd.get() << user->name.get();
        Info* info = new Info;
        info->iid = "1";
        info->uid = user;
        info->date = "20210524";
        DF << info->insert();
    }

#elif 0
//这里是查询一条有外键表的数据并且保存json
    Info* info = QyOrmSelectOne(Info().setField("iid", QString("1")));
    if(info){
        User* user = info->uid.get<User>();
        DF << info->iid.get() << user->uid.get() << info->date.get();
        QFile file("test.json");
        file.open(QFile::WriteOnly);
        file.write(info->toJson().toUtf8());
        file.close();
    }

#elif 0
//这里是删除所有new出来没有删除的table对象实例
    User::destoryAllInstance();

#endif

    DF << "end.";
    return 0;
}
