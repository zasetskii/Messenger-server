#ifndef MYSQLTABLEMODEL_H
#define MYSQLTABLEMODEL_H

#include <QSqlTableModel>
#include <QObject>
#include <QSqlDatabase>
#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>
#include <QSqlRecord>

class SimpleColumn
{
public:
    SimpleColumn(QString name);
    QString name() { return m_name; }
    QVariant colData(const QSqlRecord& record, int role = Qt::DisplayRole); //извлечение данных
private:
    QString m_name;
};


class MySqlTableModel : public QSqlTableModel
{
    Q_OBJECT

public:
    MySqlTableModel(QObject *parent = nullptr);
    ~MySqlTableModel();
    void registerColumn(SimpleColumn* column);

    // QAbstractItemModel interface
public:
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QHash<int, QByteArray> roleNames() const;

private:
    QList<SimpleColumn*> m_columns;
};

#endif // MYSQLTABLEMODEL_H
