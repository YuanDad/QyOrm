#ifndef QYUUID_H
#define QYUUID_H

#include <QUuid>

class QyUuid
{
public:
    enum l16_enum{ l16 };
    static inline QString uuidSec(QyUuid::l16_enum) { return QUuid::createUuid().toString(); }
    static inline QString uuidSec() { return QUuid::createUuid().toString(); }
    static inline QString uuidMsecOrder() { return QUuid::createUuid().toString(); }
};
#endif // QYUUID_H
