#include <QCoreApplication>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include <QSqlField>
#include <QFile>
#include <QDebug>

/*
 * 1.连接数据库
 * 2.填写正确的QrQrmFilePath,QyOrm.h的绝对文件路径
 * 3.在downloadTableNames里填写要生成的表
 * 4.去QyOrm.h中把每个表的 PRIMARYKEY 改出来,因为QSqlField只能识别类型,
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

    QString QrQrmFilePath = "/home/yinkui/QyOrm/QyOrm.h";

    QStringList downloadTableNames = {
        "TableName"
    };

    QString content = "#ifndef QYORM_H\n#define QYORM_H\n#include \"Table.h\"\n\n";
    for(const QString& tableName : downloadTableNames){
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
            }
            content += QString("\n};\n");
        }
    }
    content += "\n#endif // QYORM_H\n";


    QFile file(QrQrmFilePath);
    if(!file.open(QFile::WriteOnly)){
        qCritical() << QrQrmFilePath << "文件打不开";
        return 0;
    }
    file.write(content.toUtf8());
    file.close();

    qInfo() << "自动生成结束.";
    return 0;
}
