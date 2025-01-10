#ifndef QYORMDATETIMEEDIT_H
#define QYORMDATETIMEEDIT_H

#include <QDateTimeEdit>

class QyOrmDateTimeEdit : public QDateTimeEdit
{
    Q_OBJECT
public:
    explicit inline QyOrmDateTimeEdit(QWidget *parent = nullptr):
        QDateTimeEdit(QDateTime::currentDateTime(), parent) {}
    explicit inline QyOrmDateTimeEdit(const QDateTime &datetime, QWidget *parent = nullptr):
        QDateTimeEdit(datetime, parent) {}
    static void addTodayAndClearButton(QCalendarWidget* widget, QDateTimeEdit *edit);

protected:
    inline QyOrmDateTimeEdit(const QVariant &datetime, QVariant::Type parserType, QWidget *parent = nullptr):
        QDateTimeEdit(datetime, parserType, parent) {}

public Q_SLOTS:
    void setClearButtonEnabled(bool enable = true);
    void setDateTime(const QDateTime &dateTime);
    void setDate(const QDate &date);
    void setTime(const QTime &time);

public:
    QDateTime dateTime() const;
    QDate date() const;
    QTime time() const;

    inline void clearDateTime() { setDateTime(QDateTime()); }
    inline void setNowDateTime() { setDateTime(QDateTime::currentDateTime()); }
    inline void setNowDate() { setDate(QDate::currentDate()); }
    inline void setNowTime() { setTime(QTime::currentTime()); }

protected:
    bool __isValid = true;
    QAction *__clearAction = nullptr;
    QString textFromDateTime(const QDateTime &dt) const override;
    void stepBy(int steps) override;
};

class QyOrmDateEdit : public QyOrmDateTimeEdit
{
    Q_OBJECT
public:
    explicit inline QyOrmDateEdit(QWidget *parent = nullptr):
        QyOrmDateTimeEdit(QDate::currentDate(), QVariant::Date, parent){}
    explicit inline QyOrmDateEdit(const QDate &date, QWidget *parent = nullptr):
        QyOrmDateTimeEdit(date, QVariant::Date, parent){}

    void setCalendarPopup(bool enable);
    inline void clearDate() { setDate(QDate()); }
};

class QyOrmTimeEdit : public QyOrmDateTimeEdit
{
    Q_OBJECT
public:
    explicit inline QyOrmTimeEdit(QWidget *parent = nullptr):
        QyOrmDateTimeEdit(QTime::currentTime(), QVariant::Time, parent){}
    explicit inline QyOrmTimeEdit(const QTime &time, QWidget *parent = nullptr):
        QyOrmDateTimeEdit(time, QVariant::Time, parent){}

    inline void clearTime() { setTime(QTime()); }
};

#endif // QYORMDATETIMEEDIT_H
