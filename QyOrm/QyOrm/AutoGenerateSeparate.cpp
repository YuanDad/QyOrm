#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QSqlError>
#include <QFile>
#include <QDebug>

/*
 * 1.连接数据库
 * 2.填写正确的QrQrmFilePath的绝对文件目录路径
 * 3.在downloadTableNames里填写要生成的表
 * 4.去每个表中把每个表的 PRIMARYKEY 改出来,因为QSqlField只能识别类型,是否为空
*/

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    Q_UNUSED(application)

    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("QySqlite.db");
    if(!db.open()){
        qCritical() << "数据库打不开";
        return 0;
    }
    QSqlQuery query(db);
    query.setForwardOnly(true);

    QString QrQrmFilePath = "/home/yinkui/Qy/Qy/Model/";

    QStringList downloadTableNames = {
//        "TableName",
    };

    for(const QString& tableName : downloadTableNames){
        QString tableNameUpper = tableName.toUpper();
        QString content = QString("#ifndef %1_H\n#define %2_H\n#include \"QyOrm/Table.h\"\n\n").arg(tableNameUpper).arg(tableNameUpper);

        if(query.exec("SELECT * FROM " + tableName)){
            QSqlRecord&& record = query.record();
            int fieldCount = record.count();

            content += QString("class %1 : public Qy::Table").arg(tableName);
            content += QString("\n{\n\tQ_OBJECT\npublic:\n\t");
            content += QString("TABLE(%1)").arg(tableName);
            content += QString("\n\n");

            for(int index = 0; index < fieldCount; ++index){
                QSqlField&& field = record.field(index);
                QString fieldName = field.name();
                QString fieldType;
                QVariant::Type type = field.type();
                if(type == QVariant::String){
                    fieldType = "Qy::StringField";
                }else if(type == QVariant::Int ||  type == QVariant::LongLong || type == QVariant::ULongLong){
                    fieldType = "Qy::IntegerField";
                }else if(type == QVariant::Double){
                    fieldType = "Qy::DoubleField";
                }else if(type == QVariant::DateTime){
                    fieldType = "Qy::DateTimeField";
                }else if(type == QVariant::ByteArray){
                    fieldType = "Qy::BlobField";
                }else if(type == QVariant::UInt){
                    fieldType = "Qy::BooleanField";
                }
                QString fieldAttribute = (field.requiredStatus() ? "NOTNULL" : "FIELD");

                content += QString("\t%1(%2, %3)\n").arg(fieldAttribute).arg(fieldType).arg(fieldName);
                qInfo() << index << fieldName << type << fieldAttribute << fieldType;
            }
            content += QString("\n};\n");
        }else{
            qCritical() << query.lastError().text();
        }
        content += QString("\n#endif // %1_H\n").arg(tableNameUpper);

        QFile file(QrQrmFilePath + '/' + tableName + ".h");
        if(!file.open(QFile::WriteOnly)){
            qCritical() << file.fileName() << "文件打不开";
        }
        file.write(content.toUtf8());
        file.close();
    }

    qInfo() << "自动生成结束.";
    return 0;
}
