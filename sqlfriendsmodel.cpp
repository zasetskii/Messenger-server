#include "sqlfriendsmodel.h"

SqlFriendsModel::SqlFriendsModel(QObject *parent)
    : MySqlTableModel{parent}
{

}

void SqlFriendsModel::setUserFilter(const QString &user)
{
    QString filter = QString("user_name = '%1'").arg(user);
    qDebug() << "Current friends filter: " << filter;
    setFilter(filter);
}

void SqlFriendsModel::addFriend(const QString& username, const QString& new_friend)
{
    QSqlRecord new_record = record();
    new_record.setValue("user_name", username);
    new_record.setValue("friend_name", new_friend);
    insertRecord(-1, new_record);
    submitAll();
    select();
}
