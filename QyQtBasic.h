#ifndef QYQTBASIC_H
#define QYQTBASIC_H

#include <QFile>

namespace Qy {
static QByteArray loadFile(const QString& filePath){
    QByteArray data;
    QFile file(filePath);
    if(file.open(QFile::ReadOnly)){
        data = file.readAll();
        file.close();
    }
    return data;
}
}
#endif // QYQTBASIC_H
