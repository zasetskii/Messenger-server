#include "tcpsqlserver.h"

TcpSqlServer::TcpSqlServer(int port_num, QWidget *parent)
    : QWidget(parent), log (new QTextEdit)
{
    tcp_server = new QTcpServer(this);
    if (!tcp_server->listen(QHostAddress::Any, port_num))
    {
        QMessageBox::critical(this, "Server Error", "Unable to start the server: " + tcp_server->errorString());
        tcp_server->close();
        return;
    }

    connect(tcp_server, &QTcpServer::newConnection, this, &TcpSqlServer::slotNewConnection);

    log->setReadOnly(true);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(new QLabel("<H1>Server</H1>"));
    layout->addWidget(log);

    //Инициализируем табличную модель сообщений
    m_message_model = new SqlMessageModel;
    m_message_model->setTable("message");
    //Получаем имена столбцов
    QSqlRecord field_names = m_message_model->record();
    //Создаём экземпляры колонок с именами из таблицы
    for (int i = 0; i < m_message_model->columnCount(); ++i)
    {
        m_message_model->registerColumn(new SimpleColumn(field_names.fieldName(i)));
    }
    m_message_model->select();

    //Инициализируем модель таблицы пользователей
    m_users_model = new MySqlTableModel;
    m_users_model->setTable("user");
    field_names.clear();
    field_names = m_users_model->record();
    for (int i = 0; i < m_users_model->columnCount(); ++i)
    {
        m_users_model->registerColumn(new SimpleColumn(field_names.fieldName(i)));
    }
    m_users_model->select();

    //Инициализируем модель таблицы друзей
    m_friends_model = new SqlFriendsModel;
    m_friends_model->setTable("friends");
    field_names.clear();
    field_names = m_friends_model->record();
    for (int i = 0; i < m_friends_model->columnCount(); ++i)
    {
        m_friends_model->registerColumn(new SimpleColumn(field_names.fieldName(i)));
    }
    m_friends_model->select();
}

void TcpSqlServer::sendCommand(const ClientCommand &command, QTcpSocket *client_socket) const
{
    QByteArray block_command;
    QDataStream out_command(&block_command, QIODevice::WriteOnly);
    out_command << 0 << command;
    out_command.device()->seek(0);
    out_command << block_command.size() - sizeof(int);
    client_socket->write(block_command);
}

void TcpSqlServer::sendMessages(const QPair<QString, QString>& sender_receiver_pair, QTcpSocket* client_socket)
{
    //обрабатываем запрос: фильтруем, сортируем, отправляем данные
    m_message_model->setSenderReceiverFilter(sender_receiver_pair.first, sender_receiver_pair.second);
    m_message_model->sortByTimeDate();
    m_message_model->select();
    //Получаем все строки QSqlRecord, конвертируем их в QVariantMap
    int count_items = m_message_model->rowCount();
    for (int i = 0; i < count_items; ++i)
    {
        QSqlRecord record = m_message_model->record(i);
        QVariantMap variant_map_record;
        variant_map_record.insert("id", record.value(0)); //отправляем messege_id
        for (int col = 1; col < m_message_model->columnCount(); ++col) //передаём все остальные столбцы
        {
            variant_map_record.insert(record.fieldName(col), record.value(col));
        }

        sendCommand(MESSAGE, client_socket);

        QByteArray block_record;
        QDataStream out(&block_record, QIODevice::WriteOnly);
        out << 0 << variant_map_record;
        out.device()->seek(0);
        out << block_record.size() - sizeof(int);
        client_socket->write(block_record);
    }
    //Заносим новое соединение в активные соединения
    m_active_connections.insert(client_socket, sender_receiver_pair);
}

void TcpSqlServer::checkUpdate(const QVariantMap &message) const
{
    //Ищем среди активных соединений собеседника (sender и receiver поменяны местами)
    QString sender = message.value("sender").toString();
    QString receiver = message.value("receiver").toString();
    QPair<QString, QString> sender_receiver(receiver, sender);
    for (QHash<QTcpSocket*, QPair<QString, QString>>::const_iterator it = m_active_connections.constBegin();
         it != m_active_connections.constEnd(); ++it)
    {
        //Если sender и receiver совпадают, отправляем сообщение по этому сокету
        if (it.value() == QPair<QString, QString>(sender, receiver) ||
            it.value() == QPair<QString, QString>(receiver, sender))
            sendUpdate(message, it.key());
    }
}

void TcpSqlServer::sendUpdate(const QVariantMap &message, QTcpSocket *client_socket) const
{
    sendCommand(MESSAGE, client_socket);

    QByteArray block_message;
    QDataStream out(&block_message, QIODevice::WriteOnly);
    out << 0 << message;
    out.device()->seek(0);
    out << block_message.size() - sizeof(int);
    client_socket->write(block_message);
}

void TcpSqlServer::sendUsers(QTcpSocket *client_socket)
{
    m_users_model->setFilter("");
    QStringList users;
    qDebug() << "Users count: " << m_users_model->rowCount();
    for (int i = 0; i < m_users_model->rowCount(); ++i)
    {
        QSqlRecord record = m_users_model->record(i);
        users.append(record.value("name").toString());
    }

    sendCommand(USERS, client_socket);
    //Отправляем список пользователей
    QByteArray block_message;
    QDataStream out(&block_message, QIODevice::WriteOnly);
    out << 0 << users;
    out.device()->seek(0);
    out << block_message.size() - sizeof(int);
    client_socket->write(block_message);
}

void TcpSqlServer::addUser(const QString &user)
{
    QSqlRecord new_record = m_users_model->record();
    new_record.setValue("name", user);
    m_users_model->insertRecord(-1, new_record);
    m_users_model->submitAll();
    m_users_model->select();
}

void TcpSqlServer::sendFriends(const QString& user, QTcpSocket *client_socket) const
{
    m_friends_model->setUserFilter(user);
    m_friends_model->select();
    int count_items = m_friends_model->rowCount();
    //Для каждого друга отправляем последнее сообщение из переписки
    for (int i = 0; i < count_items; ++i)
    {
        QSqlRecord friend_record = m_friends_model->record(i);
        QString friend_name = friend_record.value("friend_name").toString();

        m_message_model->setSenderReceiverFilter(user, friend_name);
        m_message_model->sortByTimeDate();
        m_message_model->select();
        //Составляем запись для отправки: text, time, sender, friend_name
        QSqlRecord last_record = m_message_model->record(m_message_model->rowCount() - 1);
        QVariantMap variant_map_record;
        for (int col = 1; col < 4; ++col)
        {
            variant_map_record.insert(last_record.fieldName(col), last_record.value(col));
        }
        variant_map_record.insert("friend_name", friend_name);
        sendCommand(FRIEND, client_socket);
        QByteArray block_message;
        QDataStream out(&block_message, QIODevice::WriteOnly);
        out << 0 << variant_map_record;
        out.device()->seek(0);
        out << block_message.size() - sizeof(int);
        client_socket->write(block_message);
    }
}

void TcpSqlServer::saveAvatar(const QString &username, const QImage &avatar)
{
    //Сохраняем файл аватара
    QString save_path = QDir::currentPath() + "/avatars/";
    QDir save_dir(save_path);
    if (!save_dir.exists())
        save_dir.mkpath(save_path);
    if (avatar.save(save_dir.absoluteFilePath(username), "JPG"))
        qInfo() << "Saved successfully";
    else
        qInfo() << "Save failed";
    //Записываем путь к аватару в БД
    QString filter = QString("name = '%1'").arg(username);
    m_users_model->setFilter(filter);
    qInfo() << m_users_model->rowCount();
    QSqlRecord record = m_users_model->record(0); //Должна быть только одна запись после фильтрации - под индексом 0
    record.setValue("image_url", save_path + username);
    m_users_model->setRecord(0, record);
    m_users_model->submitAll();
}

void TcpSqlServer::sendAvatar(const QString &username, QTcpSocket *client_socket)
{
    QString filter = QString("name = '%1'").arg(username);
    m_users_model->setFilter(filter);
    QSqlRecord record = m_users_model->record(0);
    QString image_path = record.value("image_url").toString();
    QImage avatar(image_path);

    sendCommand(AVATAR, client_socket);
    //Отправляем аватар
    QByteArray block_image;
    QDataStream out(&block_image, QIODevice::WriteOnly);
    out << 0 << avatar;
    out.device()->seek(0);
    out << block_image.size() - sizeof(int);
    client_socket->write(block_image);
}

void TcpSqlServer::sendFriendAvatar(const QString &friend_name, QTcpSocket *client_socket)
{
    QString filter = QString("name = '%1'").arg(friend_name);
    m_users_model->setFilter(filter);
    QSqlRecord record = m_users_model->record(0);
    QString image_path = record.value("image_url").toString();
    QImage avatar(image_path);

    sendCommand(FRIEND_AVATAR, client_socket);
    QByteArray block_image;
    QDataStream out(&block_image, QIODevice::WriteOnly);
    out << 0 << avatar;
    out.device()->seek(0);
    out << block_image.size() - sizeof(int);
    client_socket->write(block_image);
}

void TcpSqlServer::slotReadClient()
{
    QTcpSocket* client_socket = static_cast<QTcpSocket*>(sender()); //объект, пославший сигнал
    QDataStream in(client_socket);
    while(1)
    {
        if (m_next_block_size == 0)
        {
            if (client_socket->bytesAvailable() < sizeof(m_next_block_size))
                break;
            in >> m_next_block_size;
        }
        //Выходим из цикла, если не все данные текущего блока ещё подгрузились
        if (client_socket->bytesAvailable() < m_next_block_size)
            break;
        if (m_command == NO_COMMAND_SERVER)
        {
            in >> m_command;
            //Обработка команд без аргументов
            if (m_command == USERS_GET)
            {
                sendUsers(client_socket);
                m_command = NO_COMMAND_SERVER;
            }
            m_next_block_size = 0;
            continue;
        }
        //Обработка команд с аргументами
        else if (m_command == MESSAGES_READ)
        {
            static QPair<QString, QString> sender_receiver_pair = QPair<QString, QString>("", "");
            if (sender_receiver_pair.first == "")
            {
                in >> sender_receiver_pair.first;
            }
            else
            {
                in >> sender_receiver_pair.second;
                sendMessages(sender_receiver_pair, client_socket);
                sender_receiver_pair = QPair<QString, QString>("", "");
                m_command = NO_COMMAND_SERVER;
            }
        }
        else if (m_command == MESSAGE_WRITE)
        {
            QVariantMap new_message;
            in >> new_message;
            m_message_model->addMessage(new_message);
            int new_message_id = m_message_model->record(m_message_model->rowCount() - 1).value("message_id").toInt();
            new_message.insert("id", new_message_id);
            checkUpdate(new_message);
            m_command = NO_COMMAND_SERVER;
        }
        else if (m_command == MESSAGE_REMOVE)
        {
            int id;
            in >> id;
            m_message_model->removeMessage(id);
            m_command = NO_COMMAND_SERVER;
        }
        else if (m_command == USER_APPEND)
        {
            QString user;
            in >> user;
            addUser(user);
            m_command = NO_COMMAND_SERVER;
        }
        else if (m_command == FRIENDS_GET)
        {
            QString user;
            in >> user;
            sendFriends(user, client_socket);
            m_command = NO_COMMAND_SERVER;
        }
        else if (m_command == FRIEND_APPEND)
        {
            static QPair<QString, QString> user_friend_pair = QPair<QString, QString>("", "");
            if (user_friend_pair.first == "")
            {
                in >> user_friend_pair.first;
            }
            else
            {
                in >> user_friend_pair.second;
                m_friends_model->addFriend(user_friend_pair.first, user_friend_pair.second);
                user_friend_pair = QPair<QString, QString>("", "");
                m_command = NO_COMMAND_SERVER;
            }
        }
        else if (m_command == AVATAR_UPDATE)
        {
            static QPair<QString, QImage> user_avatar_pair = QPair<QString, QImage>("", QImage());
            if (user_avatar_pair.first == "")
            {
                in >> user_avatar_pair.first;
            }
            else
            {
                in >> user_avatar_pair.second;
                saveAvatar(user_avatar_pair.first, user_avatar_pair.second);
                user_avatar_pair = QPair<QString, QImage>("", QImage());
                m_command = NO_COMMAND_SERVER;
            }
        }
        else if (m_command == AVATAR_GET)
        {
            QString username;
            in >> username;
            sendAvatar(username, client_socket);
            m_command = NO_COMMAND_SERVER;
        }
        else if (m_command == FRIEND_AVATAR_GET)
        {
            QString friend_name;
            in >> friend_name;
            sendFriendAvatar(friend_name, client_socket);
            m_command = NO_COMMAND_SERVER;
        }
        m_next_block_size = 0;
    }
}

void TcpSqlServer::onDisconnected()
{
    QTcpSocket* client_socket = static_cast<QTcpSocket*>(sender());
    log->append("Disconnected: " + QString::number(client_socket->peerPort()));
    client_socket->deleteLater();
}

void TcpSqlServer::slotNewConnection()
{
    QTcpSocket* client_socket = tcp_server->nextPendingConnection();
    connect(client_socket, &QTcpSocket::disconnected, this, &TcpSqlServer::onDisconnected); //при отсоединении клиента удаляем его
    connect(client_socket, &QTcpSocket::readyRead, this, &TcpSqlServer::slotReadClient); //когда новые данные готовы, читаем их
    log->append("New connection: " + QString::number(client_socket->peerPort()));
}

TcpSqlServer::~TcpSqlServer()
{
}

