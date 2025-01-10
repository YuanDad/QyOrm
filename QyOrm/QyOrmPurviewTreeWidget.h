#ifndef QYORMPURVIEWTREEWIDGET_H
#define QYORMPURVIEWTREEWIDGET_H

#include <QStringList>
#include "QyOrmTable.h"

class QTreeWidget;
class QyOrmTable;

#define QYORM_TABLE_PURVIEWMODULE(TABLE) \
explicit inline TABLE(QObject *parent = nullptr) : QyOrmPurviewModule(parent){ __initialize(); } \
    QYORM_TABLE_INLINE_FUNCTION(TABLE)
class QyOrmPurviewModule: public QyOrmTable
{
    Q_OBJECT
public:
    explicit inline QyOrmPurviewModule(QObject *parent = nullptr) : QyOrmTable(parent){}

    //主键和字段名不可更改
    QYORM_FIELD(QyOrmStringField, qyid, QyOrmFieldConstraint::PrimaryKey);
    QYORM_FIELD(QyOrmStringField, parentId);
    QYORM_FIELD(QyOrmStringField, name);

public:
    inline QyOrmPurviewModule* child(const QString& childName,
                                     QyOrmPurviewModule* __default = nullptr)
    { return childrenModule.value(childName, __default); }
    inline bool contains(const QString& childName)
    { return childrenModule.contains(childName); }

public:
    QyOrmPurviewModule* parentModule = nullptr;
    QHash<QString, QyOrmPurviewModule*> childrenModule;
    bool isEnabled = false;//状态量,目前都可以用,因为是一次性的消耗品
};
QYORM_REINTERPRET_CAST_ULONGLONG(QyOrmPurviewModule)

#define QYORM_TABLE_PURVIEWINFO(TABLE) \
explicit inline TABLE(QObject *parent = nullptr) : QyOrmPurviewInfo(parent){__initialize();} \
    QYORM_TABLE_INLINE_FUNCTION(TABLE)
class QyOrmPurviewInfo: public QyOrmTable
{
    Q_OBJECT
public:
    explicit inline QyOrmPurviewInfo(QObject *parent = nullptr) : QyOrmTable(parent){}

    //主键和字段名不可更改
    QYORM_FIELD(QyOrmStringField, qyid, QyOrmFieldConstraint::PrimaryKey);
    QYORM_FIELD(QyOrmStringField, moduleId);
    QYORM_FIELD(QyOrmStringField, belong);

private:
    friend class QyOrmPurviewTreeWidget;
    bool __isChecked;
};

class QyOrmPurviewTreeWidgetprivate;
class QyOrmPurviewTreeWidget : public QObject
{
public:
    explicit QyOrmPurviewTreeWidget(QTreeWidget* treeWidget = nullptr);
    ~QyOrmPurviewTreeWidget();
    void setTreeWidget(QTreeWidget* treeWidget);

//前提都是先把modules、infos查询出来,才能依次使用以下功能

//设置权限的时候使用
    void refreshTree();//根据所有module生成所有项目,module代表项目
    void refreshData();//根据所有info更新选中状态,info代表选中状态
    void deleteNull();//将nullModules和nullInfos全部从数据库中删除

    void extract();//筛选出insertInfoModules和deleteInfos
    //遍历insertInfoModules创建对应的info
    void doInsertInfos(QyOrmPurviewInfo& info, const QString& belong);
    void doDeleteInfos();//删除所有取消选中的info

    //先筛选,再添加新选中的info,再删除取消选中的info
    inline void doExtract(QyOrmPurviewInfo& info, const QString& belong)
    {extract();doInsertInfos(info, belong);doDeleteInfos();}

    void checkAll(bool is);

//读取权限的时候用
    static void insertFromJson(QyOrmPurviewModule& module, const QString& jsonPath);
    static void updateFromJson(QyOrmPurviewModule& module, const QString& jsonPath);
    void parsePurview();//将info查出来的modules,子项目对应上父项目,供程序去读取权限

public:
    QList<QyOrmPurviewModule*> modules;
    QList<QyOrmPurviewModule*> nullModules;
    QHash<QString, QyOrmPurviewModule*> topLevelModules;
    QList<QyOrmPurviewInfo*> infos;
    QList<QyOrmPurviewInfo*> nullInfos;

    QList<QyOrmPurviewModule*> insertInfoModules;//新选中的项目
    QList<QyOrmPurviewInfo*> deleteInfos;//取消选中的项目

private:
    QyOrmPurviewTreeWidgetprivate* __privated;

};

#endif // QYORMPURVIEWTREEWIDGET_H
