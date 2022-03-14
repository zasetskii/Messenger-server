# Messenger Server

## Краткое описание
Это приложение - сервер для [клиента мессенджера](https://github.com/zasetskii/Messenger). Он обрабатывает запросы клиента, работает с локальной базой данных. Работает в локальной сети.

## Хранение данных
Используются 3 SQL таблицы: message (хранение сообщений), user (хранение информации о пользователях), friends (хранение информации о друзьях).<br>
Таблица: message<br>
`message_id (INT PRIMARY KEY) | text (VARCHAR(200)) | time (VARCHAR(5)) | sender (VARCHAR(30)) | receiver (VARCHAR(30)) | date (VARCHAR(200))`<br>

Таблица: user<br>
`user_id (INT PRIMARY KEY) | name (VARCHAR(30)) | image_url (VARCHAR(150))`

Таблица: friends<br>
`friends_id (INT PRIMARY KEY) | user_name (VARCHAR(30)) | friend_name (VARCHAR(30))`

## Описание файлов/классов
[commands.h](https://github.com/zasetskii/Messenger-server/blob/main/commands.h) – содержит `enum` команд, используемых для взаимодействия приложений клиента и сервера.<br>
[sqlconnection.h](https://github.com/zasetskii/Messenger-server/blob/main/sqlconnection.h) – функции для установления соединения с базой данных.<br>
[SqlMessageModel](https://github.com/zasetskii/Messenger-server/blob/main/sqlmessagemodel.h), [MySqlTableModel](https://github.com/zasetskii/Messenger-server/blob/main/mysqltablemodel.h), [SqlFriendsModel](https://github.com/zasetskii/Messenger-server/blob/main/sqlfriendsmodel.h) – модели данных для работы с SQL таблицами.<br>
[TcpSqlServer](https://github.com/zasetskii/Messenger-server/blob/main/tcpsqlserver.h) – класс обработки команд от клиента. Содержит всю бизнес-логику.

## Логика работы
Информация передаётся блоками, при этом перед каждым блоком указывается размер блока в байтах.<br>
`<размер блока> <данные>`<br>
Перед отправкой данных отправляется соответствующая им команда.<br>
Принятые данные обрабатываются сервером в функции-слоте `TcpSqlServer::slotReadClient()` внутри бесконечного цикла `while(1)`. Условие выхода из цикла - размер доступных для чтения данных сокета меньше заявленного размера блока либо отсутствие данных для чтения.

## Скачать
[Приложение-клиент](https://disk.yandex.ru/d/_OELQFTO6egETg)<br>
[Приложение-сервер](https://disk.yandex.ru/d/AUzihxSNXG-FvQ)<br>
1. Распаковать<br>
2. Запустить сервер `TCP_SQL_Server.exe`<br>
3. Запустить клиент `messenger_client.exe`
