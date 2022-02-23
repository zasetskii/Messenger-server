#include "mysqltablemodel.h"

MySqlTableModel::MySqlTableModel(QObject *parent)
    : QSqlTableModel(parent)
{

}

MySqlTableModel::~MySqlTableModel()
{

}

void MySqlTableModel::registerColumn(SimpleColumn *column)
{
    m_columns.append(column);
}

QHash<int, QByteArray> MySqlTableModel::roleNames() const
{
    QHash<int, QByteArray> roles = QAbstractItemModel::roleNames();
    for (int role = Qt::UserRole; role < m_columns.count(); ++role)
    {
        roles.insert(role, m_columns.at(role - Qt::UserRole)->name().toUtf8());
    }
    return roles;
}

QVariant MySqlTableModel::data(const QModelIndex &index, int role) const
{
    if (role < Qt::UserRole)
    {
        return QSqlQueryModel::data(index, role);
    }

    QSqlRecord cur_record = this->record(index.row());
    const int column_number = role - Qt::UserRole;
    return m_columns.at(column_number)->colData(cur_record);
}


SimpleColumn::SimpleColumn(QString name) : m_name(name)
{

}

QVariant SimpleColumn::colData(const QSqlRecord& record, int role)
{
    if (role != Qt::DisplayRole)
        return QVariant();
    return record.value(this->name());
}

