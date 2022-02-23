#ifndef COMMANDS_H
#define COMMANDS_H
#include <QDataStream>

enum ClientCommand
{
    NO_COMMAND_CLIENT,
    MESSAGE,
    USERS,
    FRIEND,
    AVATAR,
    FRIEND_AVATAR
}; //команды клиенту

inline QDataStream& operator>>(QDataStream& stream, ClientCommand& cmd)
{
    qint64 val;
    stream >> val;
    cmd = static_cast<ClientCommand>(val);
    return stream;
}
inline QDataStream& operator<<(QDataStream& stream, const ClientCommand& cmd)
{
    stream << static_cast<qint64>(cmd);
    return stream;
}

enum ServerCommand
{
    NO_COMMAND_SERVER,
    MESSAGES_READ,
    MESSAGE_WRITE,
    MESSAGE_REMOVE,
    USERS_GET,
    USER_APPEND,
    FRIENDS_GET,
    FRIEND_APPEND,
    AVATAR_UPDATE,
    AVATAR_GET,
    FRIEND_AVATAR_GET
}; //команды серверу

inline QDataStream& operator>>(QDataStream& stream, ServerCommand& cmd)
{
    qint64 val;
    stream >> val;
    cmd = static_cast<ServerCommand>(val);
    return stream;
}
inline QDataStream& operator<<(QDataStream& stream, const ServerCommand& cmd)
{
    stream << static_cast<qint64>(cmd);
    return stream;
}

#endif // COMMANDS_H
