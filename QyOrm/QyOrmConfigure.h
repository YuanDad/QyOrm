#ifndef QYORMCONFIGURE_H
#define QYORMCONFIGURE_H

class QyOrmConfigure{
public:
    /*
     * 是否开启表名和字段名用"进行包裹
     * 默认值为false
    */
    static bool enabledBrace;//= false
    /*
     * 是否开启字段名用表名做前缀修饰
     * 默认值为false
    */
    static bool enabledBelong;//= false
    /*
     * 是否开启远程数据库
     * 默认值为false
    */
    static bool enabledRemoteDatabase;//= false
};

#endif // QYORMCONFIGURE_H
