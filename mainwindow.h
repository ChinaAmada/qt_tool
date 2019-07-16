#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QtNetwork>
#include <QTcpSocket>
#include <QTcpServer>
#include <QMessageBox>
#include <QSerialPort>
#include <QSerialPortInfo>

#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtCharts/QLogValueAxis>
#include <QtCharts/QValueAxis>

#include <math.h>

#include <mytcpserver.h>
#include <qcustomplot.h>

QT_CHARTS_USE_NAMESPACE

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();


private slots:
    void on_pushButton_Send_clicked();
    void on_pushButton_Connect_clicked();
    void connectedSlot();
    void disconnectedSlot();
    void errorSlot(QAbstractSocket::SocketError);
    void readyReadSlot();
    void on_pushButton_clicked();

    void on_pushButton_Ser_Open_clicked();
    void readyReadSlot_Ser();
    void on_pushButton_Ser_Send_clicked();
    void on_checkBox_Send_Hex_stateChanged(int arg1);
    void on_pushButton_Ser_ClearRev_clicked();
    void on_checkBox_Rcv_Hex_stateChanged(int arg1);
    void on_pushButton_Server_Listen_clicked();

    void Server_ClientConnect(int clientID,QString IP,int Port);
    void Server_ClientDisConnect(int clientID,QString IP,int Port);
    void Server_ClientReadData(int clientID,QString IP,int Port,QByteArray data);
    void on_pushButton_Server_Send_clicked();
    void on_comboBox_Server_Session_currentIndexChanged(const QString &arg1);

    void initChartView();
    void timerSlot(void);
private:
    Ui::MainWindow *ui;
    QAbstractSocket *clinetToConn;
    QSerialPortInfo *comInfo;
    QSerialPort *comPort;
    myTcpServer  *netServer;

    QTimer *timer;
    double cnt;
};

#endif // MAINWINDOW_H
