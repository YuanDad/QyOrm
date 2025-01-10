#ifndef QYORMCOMBOBOXDATAMAP_H
#define QYORMCOMBOBOXDATAMAP_H

class QObject;
class QComboBox;
class QString;
class QStringList;

class QyOrmComboBoxDataMapPrivated;
class QyOrmComboBoxDataMap
{
public:
    explicit QyOrmComboBoxDataMap(QComboBox* box, QObject* parent);
    void addComboBox(QComboBox *box);
    explicit QyOrmComboBoxDataMap(QComboBox* parent);
    void release();
    static QyOrmComboBoxDataMap* dataMap(QObject* box);
    static QyOrmComboBoxDataMap* setDataMap(QComboBox* box);

    void setFromComboBox();
    void set(const QStringList& texts, const QStringList& datas);

    void append(const QString& text, const QString& data);
    void insert(int index, const QString& text, const QString& data);

    int findText(const QString& text);
    int findData(const QString& data);

    QString textToData(const QString& text);
    QString dataToText(const QString& data);

private:
    friend class QyOrmComboBoxDataMapPrivated;
    QyOrmComboBoxDataMapPrivated* __privated = nullptr;
};

#endif // QYORMCOMBOBOXDATAMAP_H
