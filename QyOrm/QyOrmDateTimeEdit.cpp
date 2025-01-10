#include "QyOrmDateTimeEdit.h"
#include <QCalendarWidget>
#include <QLineEdit>
#include <QLayout>
#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QAction>

class QyOrmCalendarWidget: public QCalendarWidget
{
public:
    explicit QyOrmCalendarWidget(QyOrmDateEdit *_edit): QCalendarWidget(nullptr), edit(_edit){
        connect(this, &QCalendarWidget::currentPageChanged, [this](int year, int month){
            QDate date = selectedDate();
            if(date.isValid()){
                int day = date.day();
                if(day > 28){
                    while (!QDate(year, month, day).isValid()) --day;
                }
                setSelectedDate(QDate(year, month, day));
            }
        });
        QyOrmDateTimeEdit::addTodayAndClearButton(this, edit);
    }
    QyOrmDateEdit *edit = nullptr;
};

void QyOrmDateTimeEdit::addTodayAndClearButton(QCalendarWidget *widget, QDateTimeEdit *edit)
{
    QWidget* navBarBackground = nullptr;
    for(QObject* object : widget->children()){
        if(object->objectName() == "qt_calendar_navigationbar"){
            navBarBackground = dynamic_cast<QWidget*>(object);
            break;
        }
    }
    if(navBarBackground){
        QHBoxLayout* headerLayout = dynamic_cast<QHBoxLayout*>(navBarBackground->layout());

        QToolButton* btn1 = new QToolButton;
        btn1->setAutoRaise(true);
        btn1->setToolTip("今天");
        btn1->setIcon(qApp->style()->standardIcon(QStyle::StandardPixmap::SP_BrowserReload));
        headerLayout->insertWidget(headerLayout->count() - 1, btn1);
        connect(btn1, &QToolButton::clicked, [widget, edit]{
            widget->parentWidget()->close();
            edit->metaObject()->invokeMethod(edit, "setDate", Q_ARG(QDate, QDate::currentDate()));
        });


        QToolButton* btn2 = new QToolButton;
        btn2->setAutoRaise(true);
        btn2->setToolTip("清空");
        btn2->setIcon(qApp->style()->standardIcon(QStyle::StandardPixmap::SP_LineEditClearButton));
        headerLayout->insertWidget(headerLayout->count() - 1, btn2);
        connect(btn2, &QToolButton::clicked, [widget, edit]{
            widget->parentWidget()->close();
            edit->metaObject()->invokeMethod(edit, "setDate", Q_ARG(QDate, QDate()));
        });
    }
}

void QyOrmDateTimeEdit::setClearButtonEnabled(bool enable)
{
    if(enable){
        if(__clearAction) return;
        __clearAction = new QAction(qApp->style()->standardIcon(QStyle::StandardPixmap::
                                                                SP_LineEditClearButton), QString(), this);
        lineEdit()->addAction(__clearAction, QLineEdit::TrailingPosition);
        connect(__clearAction, &QAction::triggered, [this]{clearDateTime();});
        connect(this, &QyOrmDateTimeEdit::dateTimeChanged, [this]{
            __clearAction->setVisible(__isValid);
        });
        __clearAction->setVisible(__isValid);
    }else{
        if(!__clearAction) return;
        lineEdit()->removeAction(__clearAction);
        __clearAction->deleteLater();
        __clearAction = nullptr;
    }
}

void QyOrmDateTimeEdit::setDateTime(const QDateTime &dateTime)
{
    if((__isValid = dateTime.isValid())){
        QDateTimeEdit::setDateTime(dateTime);
    }else{
        QDateTimeEdit::setDateTime(QDateTime::currentDateTime());
    }
    lineEdit()->setReadOnly(!__isValid);
}

void QyOrmDateTimeEdit::setDate(const QDate &date)
{
    setDateTime(date.isValid() ? QDateTime(date) : QDateTime());
}

void QyOrmDateTimeEdit::setTime(const QTime &time)
{
    setDateTime(time.isValid() ? QDateTime(QDate::currentDate(), time) : QDateTime());
}

QDateTime QyOrmDateTimeEdit::dateTime() const
{
    return (__isValid ? QDateTimeEdit::dateTime() : QDateTime());
}

QDate QyOrmDateTimeEdit::date() const
{
    return (__isValid ? QDateTimeEdit::date() : QDate());
}

QTime QyOrmDateTimeEdit::time() const
{
    return (__isValid ? QDateTimeEdit::time() : QTime());
}

QString QyOrmDateTimeEdit::textFromDateTime(const QDateTime &dt) const
{
    if(__isValid) return QDateTimeEdit::textFromDateTime(dt);
    return QString();
}

void QyOrmDateTimeEdit::stepBy(int steps)
{
    if(!__isValid){
        setNowDateTime();
    }
    QDateTimeEdit::stepBy(steps);
}

void QyOrmDateEdit::setCalendarPopup(bool enable)
{
    QDateTimeEdit::setCalendarPopup(enable);
    if(enable){
        setCalendarWidget(new QyOrmCalendarWidget(this));
    }
}
