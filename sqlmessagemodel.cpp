#include "sqlmessagemodel.h"

SqlMessageModel::SqlMessageModel(QObject *parent)
    : MySqlTableModel(parent)
{

}

SqlMessageModel::~SqlMessageModel()
{

}

void SqlMessageModel::setSenderReceiverFilter(QString sender, QString receiver)
{
    QString filter = QString("(sender = '%1' AND receiver = '%2') OR (sender = '%2' AND receiver = '%1')").arg(sender).arg(receiver);
    qDebug() << "Current filter: " << filter;
    setFilter(filter);
}

void SqlMessageModel::resetFilter()
{
    setFilter("");
}

void SqlMessageModel::sortByTimeDate()
{
    setSort(5, Qt::AscendingOrder);
}

void SqlMessageModel::addMessage(const QVariantMap& message) //QString text, QString time, QString sender, QString receiver, QString date
{
    QSqlRecord new_record = record();
    for (int col = 1; col < this->columnCount(); ++col) //считываем данные во все столбцы, кроме message_id
    {
        QString field_name = new_record.fieldName(col);
        new_record.setValue(field_name, message.value(field_name));
    }
    insertRecord(-1, new_record);
    submitAll();
    select();
}

void SqlMessageModel::removeMessage(const int id)
{
    QString filter = QString("message_id = %1").arg(id);
    setFilter(filter);
    removeRow(0);
    submitAll();
    resetFilter();
}
