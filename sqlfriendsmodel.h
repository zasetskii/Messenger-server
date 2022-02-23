#ifndef SQLFRIENDSMODEL_H
#define SQLFRIENDSMODEL_H

#include "mysqltablemodel.h"
#include <QObject>

class SqlFriendsModel : public MySqlTableModel
{
    Q_OBJECT
public:
    explicit SqlFriendsModel(QObject *parent = nullptr);

    void setUserFilter(const QString& user);
    void addFriend(const QString& username, const QString& new_friend);
};

#endif // SQLFRIENDSMODEL_H
