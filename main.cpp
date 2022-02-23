#include "tcpsqlserver.h"
#include "sqlconnection.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    if (!createConnection())
        return -1;

    TcpSqlServer server(2323);
    server.show();
    return a.exec();
}
