#ifndef SQLCONNECTION_H
#define SQLCONNECTION_H

#include <QDebug>
#include <QSqlQuery>
#include <QSqlError>

void createTable(QSqlDatabase* database)
{
    if (!database->tables().contains(QString("message")))
    {
        QSqlQuery query;
        if (!query.exec("CREATE TABLE message ("
                        "message_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "text VARCHAR(200),"
                        "time VARCHAR(5),"
                        "sender VARCHAR(30),"
                        "receiver VARCHAR(30),"
                        "date VARCHAR(10));"))
        {
            qDebug() << "Unable to create a table 'message'";
        }
        else
        {
            qDebug() << "Table 'message' created";
        }
    }

    if (!database->tables().contains(QString("user")))
    {
        QSqlQuery query;
        if (!query.exec("CREATE TABLE user ("
                        "user_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "name VARCHAR(30));"))
        {
            qDebug() << "Unable to create a table 'user'";
        }
        else
        {
            qDebug() << "Table 'user' created";
        }
    }

    if (!database->tables().contains(QString("friends")))
    {
        QSqlQuery query;
        if (!query.exec("CREATE TABLE friends ("
                        "friends_id INTEGER PRIMARY KEY AUTOINCREMENT,"
                        "user_name VARCHAR(30),"
                        "friend_name VARCHAR(30));"))
        {
            qDebug() << "Unable to create a table 'friends'";
        }
        else
        {
            qDebug() << "Table 'friends' created";
        }
    }
}

bool createConnection()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE");
    db.setDatabaseName("mydb");
    db.setUserName("user");
    db.setHostName("localhost");
    db.setPassword("password");

    if (!db.open())
    {
        qDebug() << "Cannot open database: " << db.lastError();
        return false;
    }
    createTable(&db);
    return true;
}

#endif // SQLCONNECTION_H
