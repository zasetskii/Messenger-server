#ifndef SQLMESSAGEMODEL_H
#define SQLMESSAGEMODEL_H

#include "mysqltablemodel.h"
#include <QObject>

class SqlMessageModel : public MySqlTableModel
{
    Q_OBJECT

public:
    SqlMessageModel(QObject *parent = nullptr);
    ~SqlMessageModel();

public slots: //заменить на паблик функции
    void setSenderReceiverFilter(QString sender, QString receiver);
    void resetFilter();
    void sortByTimeDate();
    void addMessage(const QVariantMap& message);
    void removeMessage(const int id);

};

#endif // SQLMESSAGEMODEL_H
