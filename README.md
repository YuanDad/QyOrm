# QyOrm

#### 介绍
支持功能：
1. AutoGenerate根据数据库表自动生成Model代码
2. 外键
3. 实例化
4. 联合查询、特殊查询
5. Json的读取和保存
6. QT GUI 常见input widget 的双向绑定
7. 自动表单

#### 软件架构
强依赖QT的QSql模块，站在巨人的肩膀上使劲儿摩擦 :joy: 
目前还在不断完善中，在实际项目中不断锤炼...

#### 使用说明

1.   表定义宏介绍

| 说明   | 语法                     |
|--------|------------------------|
| 普通字段   | FIELD(类型,字段名)          |
| 非空字段   | NOTNULL(类型,字段名)        |
| 唯一字段   | UNIQUE(类型,字段名)         |
| 唯一非空字段 | UNIQUE_NULL(类型,字段名)    |
| 主键字段   | PRIMARYKEY(类型,字段名)     |
| 自增主键字段 | AUTOPRIMARYKEY(类型,字段名) |

带默认值的定义,在上诉语法基础上加_DEFAULT,可以实现在实例化时,自动赋初值

| 说明   | 语法                     |
|--------|------------------------|
| 普通字段   | FIELD_DEFAULT(类型,字段名,默认值)          |
| 非空字段   | NOTNULL_DEFAULT(类型,字段名,默认值)        |
| 唯一字段   | UNIQUE_DEFAULT(类型,字段名,默认值)         |
| 唯一非空字段 | UNIQUE_NULL_DEFAULT(类型,字段名,默认值)    |
| 主键字段   | PRIMARYKEY_DEFAULT(类型,字段名,默认值)     |

PRIMARYKEY_DEFAULT_UUID(类型,字段名)
自动用QUuid作为默认值

带字段描述,在上诉语法基础上加_LABEL,自动为字段的fieldLabel变量进行赋值，也可用于FormWidget自动生成表单时作为QLabel的内容

| 说明   | 语法                     |
|--------|------------------------|
| 普通字段   | FIELD_LABEL(类型,字段名,字段描述)          |
| 非空字段   | NOTNULL_LABEL(类型,字段名,字段描述)        |
| 唯一字段   | UNIQUE_LABEL(类型,字段名,字段描述)         |
| 唯一非空字段 | UNIQUE_NULL_LABEL(类型,字段名,字段描述)    |
| 主键字段   | PRIMARYKEY_LABEL(类型,字段名,字段描述)     |
| 自增主键字段 | AUTOPRIMARYKEY_LABEL(类型,字段名,字段描述) |

默认值和字段描述可以同时,只需如此_DEFAULT_LABEL

示例：FIELD_DEFAULT_LABEL(类型,字段名,默认值,字段描述)

TABLE(表名) 定义表

TABLE_FUNC(表名, 直接替换在初始化函数内执行的函数体) 定义表，因为初始化函数不支持自定义，所以需要初始化函数内执行的操作就写在这里，曲线救国嘛

FOREIGN(类型, 字段名, 外键表名) 定义外键

RegisterForeign(外键表名) 在cpp中注册外键

FIELD_VALUE_LIST(字段名) 定义字段为下拉框类型,仅用于FormWidget自动生成表单

RegisterFieldValueListOnly(表名, 字段名, std::function<QStringList()>类型的函数) 在cpp中注册字段下拉框的addItems的赋值,仅用于FormWidget自动生成表单

RegisterFieldValueList(表名, 字段名, std::function<QStringList()>类型的函数, std::function<QVariant(QVariant)>类型的函数, std::function<QVariant(int)>类型的函数) 在cpp中注册字段下拉框的addItems的赋值，设置显示值，取值,仅用于FormWidget自动生成表单


2.   test_qyorm.h写了表定义,Test_QyOrm_Main.cpp写了所有支持的功能的例子
示例:

```
class User : public Qy::Table
{
    Q_OBJECT
public:
    TABLE(User)

    PRIMARYKEY(Qy::StringField,uid)
    UNIQUE(Qy::StringField,pwd)
    NOTNULL(Qy::StringField,name)
};
```

```
    for(User* user : QyOrmSelect(User())){// 全部查询 <=> select * from User
    //for(User* user : QyOrmSelect(User().setField("uid", QString("001")))){// 条件查询 <=> select * from User where uid='001'
        qDebug() << user->uid.get() << user->pwd.get() << user->name.get();    //打印值    
        user->name.set("赵一"); // 修改一条记录 <=> update User set name='赵一' where uid='001'
        user->remove(); // 删除一条记录 <=> delete from User where uid='001'
    }

    User user;
    user.uid = "005";
    user.pwd = "005";
    user.name = "孙五";
    user.insert(); // 插入一条记录 <=> insert into User values('005', '005', '孙五')
```

3.  自动表单
```
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
```

#### 安装教程

1.  将QyOrm子目录下载下来
2.  在自己的pro里添加include($$PWD/QyOrm/QyOrm.pri)


#### 演示
[Test_QyOrm_Main.cpp的运行演示](http://www.install.love/img/gif/QyOrm.gif)
(Gitee最大上传GIF大小为2MB)

#### 参与贡献

1.  俺...

