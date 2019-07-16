#ifndef MYTCPSERVER_H
#define MYTCPSERVER_H

#include <QTcpSocket>
#include <QTcpServer>
#include "mytcpclient.h"


class myTcpServer : public QTcpServer
{
    Q_OBJECT    //只有加入了Q_OBJECT，你才能使用QT中的signal和slot机制
public:
    explicit myTcpServer(QObject *parent = Q_NULLPTR);
    void SendData(int clientID, QByteArray data);
    void SendDataCurrent(QByteArray data);
    void SendDataAll(QByteArray data);

    int ClientCount()const{return clientCount;}
    void CloseAllClient();


    QList<myTcpClient *> ClientList;
    QList<int> ClientID;
    myTcpClient *CurrentClient;

    int clientCount;
private:
protected:
    void incomingConnection(int handle);

signals:
    void ClientReadData(int clientID,QString IP,int Port,QByteArray data);
    void ClientConnect(int clientID,QString IP,int Port);
    void ClientDisConnect(int clientID,QString IP,int Port);

private slots:
    void DisConnect(int clientID,QString IP,int Port);

public slots:

};

#endif // MYTCPSERVER_H
