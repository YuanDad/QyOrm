#include "QyOrmPurviewTreeWidget.h"
#include "QyQtBasic.h"
#include "QyUuid.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include <QTreeWidget>
#include <QDebug>

class QyOrmPurviewTreeWidgetprivate
{
public:
    QTreeWidget *treeWidget;
    QHash<QString, QTreeWidgetItem*> mapItem;//QyOrmPurviewModule.qyid <-> 树项目
    QHash<QString, QyOrmPurviewModule*> mapModule;
    QHash<QString, QyOrmPurviewInfo*> mapInfo;//QyOrmPurviewModule.qyid <-> 选中状态
};

//当出现选中和取消选中的动作，递归更改其父项目和子项目的选中状态
void QyOrmPurviewTreeWidget_setCheckState(QTreeWidgetItem *item, Qt::CheckState state)
{
    for(int row = 0; row < item->childCount(); ++row){
        QTreeWidgetItem *childItem = item->child(row);
        childItem->setCheckState(0, state);
        QyOrmPurviewTreeWidget_setCheckState(childItem, state);
    }
}
QyOrmPurviewTreeWidget::QyOrmPurviewTreeWidget(QTreeWidget *treeWidget) : QObject(treeWidget)
{
    __privated = new QyOrmPurviewTreeWidgetprivate;
    if(treeWidget) setTreeWidget(treeWidget);
}

QyOrmPurviewTreeWidget::~QyOrmPurviewTreeWidget()
{
    delete __privated;
    if(modules.size()) qDeleteAll(modules);
    if(infos.size()) qDeleteAll(infos);
}

void QyOrmPurviewTreeWidget::setTreeWidget(QTreeWidget *treeWidget)
{
    __privated->treeWidget = treeWidget;
    //当出现选中和取消选中的动作，更改其父项目和子项目的选中状态
    treeWidget->connect(
        treeWidget, &QTreeWidget::itemChanged, treeWidget,
        [treeWidget](QTreeWidgetItem *item, int){
            QSignalBlocker blocker(treeWidget);
            Qt::CheckState state = item->checkState(0);
            if(item->childCount() && state != Qt::PartiallyChecked){
                QyOrmPurviewTreeWidget_setCheckState(item, state);
            }
            if(state != Qt::Unchecked){
                forever{
                    QTreeWidgetItem* parentItem = item->parent();
                    if(parentItem){
                        if(parentItem->checkState(0) == Qt::Unchecked){
                            item = parentItem;
                            parentItem->setCheckState(0, Qt::PartiallyChecked);
                        }else break;
                    }else break;
                }
            }
        });
}

//递归销毁不存在的项目
void destoryItem(QyOrmPurviewTreeWidget* that, QTreeWidgetItem* item,
                 QHash<QString, QTreeWidgetItem*>& mapItem)
{
    QyOrmPurviewModule* module = toQyOrmPurviewModule(item->data(0, QyOrmItemRole::Data));
    mapItem.remove(module->qyid);
    that->modules.removeOne(module);
    that->nullModules.append(module);
    foreach(QTreeWidgetItem* child, item->takeChildren()){
        destoryItem(that, child, mapItem);
    }
    delete item;
}
//通过modules去生成所有树项目、mapItem、nullModules
void QyOrmPurviewTreeWidget::refreshTree()
{
    if(!__privated->treeWidget) return;
    QSignalBlocker blocker(__privated->treeWidget);
    __privated->treeWidget->clear();
    __privated->mapItem.clear();
    qDeleteAll(nullModules);
    nullModules.clear();

    //通过modules去生成所有树项目、mapItem
    foreach(QyOrmPurviewModule* module, modules){
        QTreeWidgetItem* item = new QTreeWidgetItem;
        item->setText(0, module->name);
        item->setData(0, QyOrmItemRole::Data, module->toULongLong());
        item->setCheckState(0, Qt::Unchecked);
        __privated->mapItem.insert(module->qyid, item);
    }

    //将所有子项目和父项目关联起来，错误的子项目筛选出来
    QList<QTreeWidgetItem*> nullItems;
    for(int row = modules.size() - 1; row >= 0; --row){
        QyOrmPurviewModule* module = modules.at(row);
        QTreeWidgetItem* item = __privated->mapItem.value(module->qyid);
        if(module->parentId.isNull()){
            __privated->treeWidget->insertTopLevelItem(0, item);
        }else{
            QTreeWidgetItem* parentItem = __privated->mapItem.value(module->parentId);
            if(parentItem){
                parentItem->insertChild(0, item);
            }else{
                nullItems.prepend(item);
            }
        }
    }
    //销毁不存在父项目的子项目
    foreach(QTreeWidgetItem* item, nullItems) destoryItem(this, item, __privated->mapItem);
    //所有父项目设置为三种可选状态，叶子节点项目设置为两种可选状态
    for(auto iter = __privated->mapItem.begin(); iter != __privated->mapItem.end(); ++iter){
        QTreeWidgetItem* item = iter.value();
        if(item->childCount()){
            item->setFlags(Qt::ItemIsUserTristate | Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        }else{
            item->setFlags(Qt::ItemIsEnabled | Qt::ItemIsUserCheckable);
        }
    }
}
//通过infos更新所有项目的选中状态，生成mapInfo、nullInfos
void QyOrmPurviewTreeWidget::refreshData()
{
    if(!__privated->treeWidget) return;
    __privated->mapInfo.clear();
    for(int row = 0; row < __privated->treeWidget->topLevelItemCount(); ++row){
        QTreeWidgetItem* item = __privated->treeWidget->topLevelItem(row);
        if(item->checkState(0) != Qt::Unchecked){
            item->setCheckState(0, Qt::Unchecked);
        }
    }
    for(int row = infos.size() - 1; row >= 0; --row){
        const QString& moduleId = infos.at(row)->moduleId;
        __privated->mapInfo.insert(moduleId, infos.at(row));
        QTreeWidgetItem* item = __privated->mapItem.value(moduleId);
        if(item){
            if(item->childCount()){
                item->setCheckState(0, Qt::PartiallyChecked);
            }else{
                item->setCheckState(0, Qt::Checked);
            }
        }else{
            nullInfos.prepend(infos.takeAt(row));
        }
    }
}

void QyOrmPurviewTreeWidget::deleteNull()
{
    foreach(QyOrmPurviewModule* module, nullModules){
        module->sqlDelete();
    }
    qDeleteAll(nullModules);
    nullModules.clear();

    foreach(QyOrmPurviewInfo* info, nullInfos){
        info->sqlDelete();
    }
    qDeleteAll(nullInfos);
    nullInfos.clear();
}

void QyOrmPurviewTreeWidgetDfs(QyOrmPurviewTreeWidget* that, QTreeWidgetItem* item){
    if(item->checkState(0) != Qt::Unchecked){
        that->insertInfoModules.prepend(toQyOrmPurviewModule(item->data(0, QyOrmItemRole::Data)));
        for(int row = 0; row < item->childCount(); ++row){
            QyOrmPurviewTreeWidgetDfs(that, item->child(row));
        }
    }
}
//筛选出insertInfoModules和deleteInfos
void QyOrmPurviewTreeWidget::extract()
{
    if(!__privated->treeWidget) return;
    insertInfoModules.clear();
    deleteInfos.clear();
    //递归将所有选中状态的module放到insertInfoModules中
    for(int row = 0; row < __privated->treeWidget->topLevelItemCount(); ++row){
        QyOrmPurviewTreeWidgetDfs(this, __privated->treeWidget->topLevelItem(row));
    }
    //将所有info的选中状态置为false
    foreach(QyOrmPurviewInfo* info, infos){
        info->__isChecked = false;
    }
    //遍历insertInfoModules,将存在对应info的module去除(存在对应info代表module本来就是选中状态),并将info的选中状态置为true
    for(int row = insertInfoModules.size() - 1; row >= 0; --row){
        QyOrmPurviewInfo* info = __privated->mapInfo.value(insertInfoModules.at(row)->qyid, nullptr);
        if(info){
            info->__isChecked = true;
            insertInfoModules.removeAt(row);
        }
    }
    //info的选中状态没有置为true的，代表之前是选中的，现在没有选中了,加入到deleteInfos中
    foreach(QyOrmPurviewInfo* info, infos){
        if(!info->__isChecked){
            deleteInfos.prepend(info);
        }
    }
}

void QyOrmPurviewTreeWidget::doInsertInfos(QyOrmPurviewInfo &info, const QString &belong)
{
    if(insertInfoModules.isEmpty()) return;
    foreach(QyOrmPurviewModule* insertModule, insertInfoModules){
        auto insertInfo = info.constructor()->dynamicCast<QyOrmPurviewInfo>();
        insertInfo->belong = belong;
        insertInfo->qyid = QyUuid::uuidMsecOrder();
        insertInfo->moduleId = insertModule->qyid;
        insertInfo->sqlInsert();
        infos.append(insertInfo);
    }
    insertInfoModules.clear();
}

void QyOrmPurviewTreeWidget::doDeleteInfos()
{
    if(deleteInfos.isEmpty()) return;
    foreach(QyOrmPurviewInfo* deleteInfo, deleteInfos){
        infos.removeOne(deleteInfo);
        deleteInfo->sqlDelete();
    }
    qDeleteAll(deleteInfos);
    deleteInfos.clear();
}

void QyOrmPurviewTreeWidget::checkAll(bool is)
{
    if(!__privated->treeWidget) return;
    for(int row = 0; row < __privated->treeWidget->topLevelItemCount(); ++row){
        __privated->treeWidget->topLevelItem(row)->setCheckState(
            0, is ? Qt::Checked : Qt::Unchecked);
    }
}

void QyOrmPurviewTreeWidget_insertFromJson(
    QyOrmPurviewModule &module, const QJsonValueRef& value, const QString& parentId)
{
    if(value.isArray()){
        QJsonArray array = value.toArray();
        for(int row = 0; row < array.size(); ++row){
            QyOrmPurviewTreeWidget_insertFromJson(module, array[row], parentId);
        }
    }else if(value.isObject()){
        QJsonObject object = value.toObject();
        for(auto iter = object.begin(); iter != object.end(); ++iter){
            module.qyid = QyUuid::uuidMsecOrder();
            module.name = iter.key();
            if(parentId.isEmpty()) module.parentId.setNull();
            else module.parentId = parentId;
            module.sqlInsert();
            QyOrmPurviewTreeWidget_insertFromJson(module, iter.value(), module.qyid);
        }
    }else if(value.isString()){
        module.qyid = QyUuid::uuidMsecOrder();
        module.name = value.toString();
        if(parentId.isEmpty()) module.parentId.setNull();
        else module.parentId = parentId;
        module.sqlInsert();
    }
}
void QyOrmPurviewTreeWidget::insertFromJson(QyOrmPurviewModule &module, const QString &jsonPath)
{
    QJsonDocument json = QJsonDocument::fromJson(Qy::loadFile(jsonPath));
    QJsonArray array = json.array();
    for(int row = 0; row < array.size(); ++row){
        QyOrmPurviewTreeWidget_insertFromJson(module, array[row], QString());
    }
}

void QyOrmPurviewTreeWidget_deleteFromJson(QyOrmPurviewModule &module)
{
    foreach(QyOrmPurviewModule* module, module.childrenModule){
        QyOrmPurviewTreeWidget_deleteFromJson(*module);
    }
    module.sqlDelete();
}
void QyOrmPurviewTreeWidget_updateFromJson(QyOrmPurviewModule &module, const QJsonValueRef& value,
                                           const QString& parentId, QHash<QString, QyOrmPurviewModule*>& hash)
{
    if(value.isArray()){
        QJsonArray array = value.toArray();
        for(int row = 0; row < array.size(); ++row){
            QyOrmPurviewTreeWidget_updateFromJson(module, array[row], parentId, hash);
        }
        for(auto iter = hash.begin(); iter != hash.end(); ++iter){
            if(!iter.value()->isEnabled){
                QyOrmPurviewTreeWidget_deleteFromJson(*iter.value());
            }
        }
    }else if(value.isObject()){
        QJsonObject object = value.toObject();
        for(auto iter = object.begin(); iter != object.end(); ++iter){
            if(hash.contains(iter.key())){
                QyOrmPurviewModule* vmodule = hash.value(iter.key());
                vmodule->isEnabled = true;
                QyOrmPurviewTreeWidget_updateFromJson(module, iter.value(), vmodule->qyid, vmodule->childrenModule);
            }else{
                module.qyid = QyUuid::uuidMsecOrder();
                module.name = iter.key();
                if(parentId.isEmpty()) module.parentId.setNull();
                else module.parentId = parentId;
                module.sqlInsert();
                QyOrmPurviewTreeWidget_insertFromJson(module, iter.value(), module.qyid);
            }
        }
    }else if(value.isString()){
        module.name = value.toString();
        if(hash.contains(module.name)){
            hash.value(module.name)->isEnabled = true;
        }else{
            module.qyid = QyUuid::uuidMsecOrder();
            if(parentId.isEmpty()) module.parentId.setNull();
            else module.parentId = parentId;
            module.sqlInsert();
        }
    }
}
void QyOrmPurviewTreeWidget::updateFromJson(QyOrmPurviewModule &module, const QString &jsonPath)
{
    QyOrmPurviewTreeWidget container;
    module.sqlSelectAll(container.modules);
    container.parsePurview();
    foreach(QyOrmPurviewModule* module, container.modules){
        module->isEnabled = false;
    }

    QJsonDocument json = QJsonDocument::fromJson(Qy::loadFile(jsonPath));
    QJsonArray array = json.array();
    for(int row = 0; row < array.size(); ++row){
        QyOrmPurviewTreeWidget_updateFromJson(module, array[row], QString(), container.topLevelModules);
    }
    for(auto iter = container.topLevelModules.begin();
         iter != container.topLevelModules.end(); ++iter){
        if(!iter.value()->isEnabled){
            QyOrmPurviewTreeWidget_deleteFromJson(*iter.value());
        }
    }
}

void QyOrmPurviewTreeWidget::parsePurview()
{
    if(modules.isEmpty()) return;
    foreach(QyOrmPurviewModule* module, modules){
        __privated->mapModule.insert(module->qyid, module);
    }
    foreach(QyOrmPurviewModule* module, modules){
        if(module->parentId.isNull()){
            topLevelModules.insert(module->name, module);
        }else{
            QyOrmPurviewModule* parentModule =
                __privated->mapModule.value(module->parentId, nullptr);
            if(parentModule){
                parentModule->childrenModule.insert(module->name, module);
                module->parentModule = parentModule;
            }
        }
    }
}
