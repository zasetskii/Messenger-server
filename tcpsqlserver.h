#ifndef TCPSQLSERVER_H
#define TCPSQLSERVER_H

#include <QWidget>
#include <QTextEdit>
#include <QTcpServer>
#include <QTcpSocket>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QLabel>
#include <QPair>
#include <QHash>
#include <QImage>
#include <QDir>
#include "mysqltablemodel.h"
#include "sqlmessagemodel.h"
#include "sqlfriendsmodel.h"
#include "commands.h"

class TcpSqlServer : public QWidget
{
    Q_OBJECT

    QTcpServer* tcp_server;
    QTextEdit* log;

public:
    TcpSqlServer(int port_num, QWidget *parent = nullptr);
    ~TcpSqlServer();

private:
    void sendCommand(const ClientCommand& command, QTcpSocket* client_socket) const;
    void sendMessages(const QPair<QString, QString>& sender_receiver_pair, QTcpSocket* client_socket);
    void checkUpdate(const QVariantMap& message) const;
    void sendUpdate(const QVariantMap& message, QTcpSocket* client_socket) const;
    void sendFriends(const QString& user, QTcpSocket* client_socket) const;
    void sendUsers(QTcpSocket* client_socket);
    void addUser(const QString& user);
    void saveAvatar(const QString& username, const QImage& avatar);
    void sendAvatar(const QString& username, QTcpSocket *client_socket);
    void sendFriendAvatar(const QString& friend_name, QTcpSocket *client_socket);

public slots:
    virtual void slotNewConnection();
    void slotReadClient();
    void onDisconnected();

private:
    SqlMessageModel* m_message_model;
    MySqlTableModel* m_users_model;
    SqlFriendsModel* m_friends_model;
    QHash<QTcpSocket*, QPair<QString, QString>> m_active_connections; // "socket - {sender, receiver}"

    ServerCommand m_command = NO_COMMAND_SERVER;
    int m_next_block_size = 0;

};
#endif // TCPSQLSERVER_H
