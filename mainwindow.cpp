#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    /********************************* NET CLIENT *************************************/
    clinetToConn = new QAbstractSocket(QAbstractSocket::TcpSocket,parent);

    connect(clinetToConn, SIGNAL(connected()), this, SLOT(connectedSlot()));
    connect(clinetToConn, SIGNAL(disconnected()), this, SLOT(disconnectedSlot()));
    connect(clinetToConn, SIGNAL(error(QAbstractSocket::SocketError)), this,
        SLOT(errorSlot(QAbstractSocket::SocketError)));
    connect(clinetToConn, SIGNAL(readyRead()), this, SLOT(readyReadSlot()));
    /********************************* NET CLIENT *************************************/


    /********************************* SERIAL PORT *************************************/
    comInfo = new QSerialPortInfo();
    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        ui->comboBox_Ser_Num->addItem(info.portName());
    }

    comPort = new QSerialPort();
    /********************************* SERIAL PORT *************************************/


    /********************************* NET SERVER *************************************/
    netServer = new myTcpServer();

    ui->label_Server_IP->setText(QNetworkInterface::allAddresses().at(2).toString());
    connect(netServer,SIGNAL(ClientConnect(int,QString,int)),
                this,SLOT(Server_ClientConnect(int,QString,int)));
    connect(netServer,SIGNAL(ClientDisConnect(int,QString,int)),
                this,SLOT(Server_ClientDisConnect(int,QString,int)));
    connect(netServer,SIGNAL(ClientReadData(int,QString,int,QByteArray)),
                this,SLOT(Server_ClientReadData(int,QString,int,QByteArray)));

    /********************************* NET SERVER *************************************/


    /********************************* CHART *************************************/

    initChartView();
    timer = new QTimer();
    connect(timer, SIGNAL(timeout()), this, SLOT(timerSlot()));
    timer->start(100);
    cnt=0;

    /********************************* CHART *************************************/
}

MainWindow::~MainWindow()
{
    delete ui;
    delete clinetToConn;
    delete comInfo;
    delete comPort;
    delete netServer;
}

void MainWindow::on_pushButton_Send_clicked()
{
    if(ui->pushButton_Connect->text()=="断开连接")
    {
        QMessageBox::information(nullptr, "警告", "未建立连接！");
    }
    else
    {
        clinetToConn->write(ui->lineEdit_SendMsg->text().toLatin1().data(),ui->lineEdit_SendMsg->text().toLatin1().length());
    }
}

/********************************* NET CLIENT *************************************/

void MainWindow::on_pushButton_Connect_clicked()
{
    //ui->lineEdit_SendMsg->setText("hello");
    if(ui->pushButton_Connect->text()=="断开连接")
    {
        clinetToConn->disconnectFromHost();
    }
    else
    {
        clinetToConn->connectToHost(ui->lineEdit_IP->text(),ui->lineEdit_Port->text().toUShort(),QIODevice::ReadWrite);
    }
}


void MainWindow::connectedSlot()
{
    ui->pushButton_Connect->setText("断开连接");
}

void MainWindow::disconnectedSlot()
{
    ui->pushButton_Connect->setText("连接");
}

void MainWindow::errorSlot(QAbstractSocket::SocketError)
{
    QMessageBox::information(nullptr, "警告", "连接错误！");
}

void MainWindow::readyReadSlot()
{
    ui->plainTextEdit_Msg->setPlainText(ui->plainTextEdit_Msg->toPlainText()+clinetToConn->readAll());
}

void MainWindow::on_pushButton_clicked()
{
    ui->plainTextEdit_Msg->setPlainText("");
}
/********************************* NET CLIENT *************************************/



/******************************** SERIAL PORT ****************************************/

void MainWindow::on_pushButton_Ser_Open_clicked()
{
    if(ui->pushButton_Ser_Open->text()=="关闭")
    {
        comPort->close();
        ui->pushButton_Ser_Open->setText("打开");
        ui->comboBox_Ser_Num->setDisabled(false);
        return;
    }

    foreach (const QSerialPortInfo &info, QSerialPortInfo::availablePorts())
    {
        if(info.portName()==ui->comboBox_Ser_Num->currentText())
        {
           comPort->setPort(info);
        }
    }
    if(comPort->open(QIODevice::ReadWrite))
    {
            //qDebug() << "m_reader.open(QIODevice::ReadWrite)";
            //comPort->setBaudRate(QSerialPort::Baud9600);
            comPort->setBaudRate(ui->comboBox_Ser_Baud->currentText().toInt());
            comPort->setParity(QSerialPort::NoParity);
            comPort->setDataBits(QSerialPort::Data8);
            comPort->setStopBits(QSerialPort::OneStop);
            comPort->setFlowControl(QSerialPort::NoFlowControl);

            comPort->clearError();
            comPort->clear();
            connect(comPort, SIGNAL(readyRead()), this, SLOT(readyReadSlot_Ser()));

            ui->pushButton_Ser_Open->setText("关闭");
            ui->comboBox_Ser_Num->setDisabled(true);
    }
}

void MainWindow::readyReadSlot_Ser()
{
    QByteArray data = comPort->readAll();
    if(ui->checkBox_Rcv_Hex->isChecked())
    {
        QString dataString;

        for (int i = 0; i < data.size(); i++)
        {
             dataString += QString("%1 ").arg(static_cast<quint8>(data.at(i)), 2, 16, QChar('0'));
        }

       // msg += dataString + "\n";
        ui->plainTextEdit_Ser_Msg->setPlainText(ui->plainTextEdit_Ser_Msg->toPlainText()+dataString);
    }
    else
    {
        ui->plainTextEdit_Ser_Msg->setPlainText(ui->plainTextEdit_Ser_Msg->toPlainText()+data);
    }
}
void MainWindow::on_pushButton_Ser_Send_clicked()
{
    QString allData = ui->lineEdit_Ser_Send->text();
    QByteArray data;
    QStringList list = allData.split(" ");

    //if(ui->pushButton_Ser_Open->text()=="打开")
    if(comPort->isOpen()==false)
    {
        QMessageBox::information(nullptr, "警告", "未打开串口！");
        return;
    }


    if(ui->checkBox_Send_Hex->isChecked())
    {
            for (const QString &hex : list) {
                data.append(static_cast<char>(hex.toInt(Q_NULLPTR, 16)));
            }
            if(ui->checkBox_Send_NewLine->isChecked())
            {
                data.append("\r\n");
            }

            comPort->write(data,data.length());  //发送
    }else {
        comPort->write(ui->lineEdit_Ser_Send->text().toLatin1().data(),ui->lineEdit_Ser_Send->text().toLatin1().length());
        if(ui->checkBox_Send_NewLine->isChecked())
        {
             comPort->write("\r\n",strlen("\r\n"));
        }
    }
}



void MainWindow::on_pushButton_Ser_ClearRev_clicked()
{
    ui->plainTextEdit_Ser_Msg->setPlainText("");
}

void MainWindow::on_checkBox_Rcv_Hex_stateChanged(int arg1)
{
    arg1=arg1+0;

    QByteArray data=ui->plainTextEdit_Ser_Msg->toPlainText().toLatin1();
    QString dataString;
    QStringList list = ui->plainTextEdit_Ser_Msg->toPlainText().split(" ");

    if(ui->checkBox_Rcv_Hex->isChecked())
    {


        for (int i = 0; i < data.size(); i++)
        {
             dataString += QString("%1 ").arg(static_cast<quint8>(data.at(i)), 2, 16, QChar('0'));
        }

       // msg += dataString + "\n";

    }
    else
    {
        bool ok;
        for (const QString &hex : list) {
            if(hex.toInt(&ok,16)!=0)
                dataString.append(hex.toInt(&ok,16));
        }

    }
     ui->plainTextEdit_Ser_Msg->setPlainText(dataString);
}

void MainWindow::on_checkBox_Send_Hex_stateChanged(int arg1)
{
    arg1=arg1+0;

    /*
        checked:    arg1=2
        unchecked:  arg1=0
    */
    //ui->lineEdit_Ser_Send->setText(QString::number(arg1));
}

/******************************** SERIAL PORT ****************************************/




/********************************* NET SERVER *************************************/

void MainWindow::on_pushButton_Server_Listen_clicked()
{
    bool ok;

    if(netServer->isListening())
    {
        netServer->CloseAllClient();
        netServer->close();
        ui->comboBox_Server_Session->clear();
        ui->plainTextEdit_Server_Recv_Msg->appendPlainText("监听关闭");
        ui->pushButton_Server_Listen->setText("开始监听");
    }
   else{
        if(netServer->listen(QHostAddress::Any,ui->lineEdit_Server_ListenPort->text().toUShort(&ok,10)))
        {
            ui->plainTextEdit_Server_Recv_Msg->appendPlainText("监听成功");
            ui->pushButton_Server_Listen->setText("停止监听");
        }
    }
}



/**********会话连接槽函数***********/
void MainWindow::Server_ClientConnect(int clientID,QString IP,int Port)
{
    //*
    ui->plainTextEdit_Server_Recv_Msg->appendPlainText("A client connected!");
    ui->plainTextEdit_Server_Recv_Msg->appendPlainText("clientID:" + QString::number(clientID,10));
    ui->plainTextEdit_Server_Recv_Msg->appendPlainText("IP:" + IP);
    ui->plainTextEdit_Server_Recv_Msg->appendPlainText("Port:" + QString::number(Port));
    //*/

    //ui->comboBox_Server_Session->addItem("session " + QString::number(clientID,10));
    ui->comboBox_Server_Session->addItem(QString::number(clientID,10)+":"+IP+"," + QString::number(Port,10));
}

/**********会话结束槽函数***********/
void MainWindow::Server_ClientDisConnect(int clientID,QString IP,int Port)
{
    this->ui->plainTextEdit_Server_Recv_Msg->appendPlainText("A client closed!");
    this->ui->plainTextEdit_Server_Recv_Msg->appendPlainText("clientID:" + QString::number(clientID,10));
    this->ui->plainTextEdit_Server_Recv_Msg->appendPlainText("IP:" + IP);
    this->ui->plainTextEdit_Server_Recv_Msg->appendPlainText("Port:" + QString::number(Port));

}

/**********会话接收数据槽函数**********/
void MainWindow::Server_ClientReadData(int clientID,QString IP,int Port,QByteArray data)
{
    IP=IP+"";
    Port=Port+0;
    clientID=clientID+0;

    this->ui->plainTextEdit_Server_Recv_Msg->appendPlainText(IP.mid(IP.indexOf("192")) +":"+ QString::number(Port) + " " + " has new data:");
    this->ui->plainTextEdit_Server_Recv_Msg->appendPlainText(QString(data));
}


void MainWindow::on_pushButton_Server_Send_clicked()
{
    netServer->CurrentClient->write(ui->lineEdit_Server_Send_Msg->text().toLatin1(),ui->lineEdit_Server_Send_Msg->text().toLatin1().length());
}

void MainWindow::on_comboBox_Server_Session_currentIndexChanged(const QString &arg1)
{
    for (int i=0;i<netServer->clientCount;i++)
    {
       //ui->plainTextEdit_Server_Recv_Msg->appendPlainText(arg1);
       //ui->plainTextEdit_Server_Recv_Msg->appendPlainText(QString::number(netServer->ClientID[i]));
       QList<QString> str=arg1.split(":");
       if(str[0].toInt()==netServer->ClientID[i])
       {
           //ui->plainTextEdit_Server_Recv_Msg->appendPlainText(QString::number(str[0].toInt()));
            netServer->CurrentClient=netServer->ClientList[i];
       }
    }
}


/********************************* NET SERVER *************************************/




/********************************* CHART *************************************/

void MainWindow::initChartView()
{
    //定义两个可变数组存放绘图的坐标数据
    QVector<double> x(101),y(101);//分别存放x和y坐标的数据,101为数据长度
    //添加数据，我们这里演示y=x^3,为了正负对称，我们x从-10到+10
    for(int i=0;i<101;i++)
    {
        x[i] = i/5 - 10;
        y[i] = x[i] * x[i] * x[i];
    }
    //向绘图区域QCustomPlot(从widget提升来的)添加一条曲线
    //ui->qCustomPlot->addGraph();
    ui->widget->addGraph();
    ui->widget->addGraph();

    ui->widget->graph(0)->setPen(QPen(Qt::blue)); // line color blue for first graph
    ui->widget->graph(0)->setBrush(QBrush(QColor(0, 0, 255, 20))); // first graph will be filled with translucent blue

    ui->widget->graph(1)->setPen(QPen(Qt::red)); // line color blue for first graph
    ui->widget->graph(1)->setBrush(QBrush(QColor(255, 0, 0, 20))); // first graph will be filled with translucent blue


    //添加数据
    //ui->widget->graph(0)->setData(x,y);

    //设置坐标轴标签名称
    ui->widget->xAxis->setLabel("x");
    ui->widget->yAxis->setLabel("y");

    //设置坐标轴显示范围,否则我们只能看到默认的范围
    ui->widget->xAxis->setRange(0,20.0);
    ui->widget->yAxis->setRange(-2,2);

    QSharedPointer<QCPAxisTicker> intTicker (new QCPAxisTicker);
    QSharedPointer<QCPAxisTickerPi> piTicker(new QCPAxisTickerPi);
    QSharedPointer<QCPAxisTickerFixed> fixTicker(new QCPAxisTickerFixed);
    fixTicker->setTickStep(1);    //设置刻度

    ui->widget->xAxis->setTicker(fixTicker);
    ui->widget->xAxis->setTickLabelColor(QColor(255,0,0));

    //ui->widget->xAxis->setti


    //重绘，这里可以不用，官方例子有，执行setData函数后自动重绘
    //我认为应该用于动态显示或者是改变坐标轴范围之后的动态显示，我们以后探索
    //ui->qCustomPlot->replot();
}


void MainWindow::timerSlot(void)
{

    ui->widget->graph(0)->addData(cnt/10,cos(cnt/10));
    ui->widget->graph(1)->addData(cnt/10,sin(cnt/10));
    if(cnt/10>=ui->widget->xAxis->range().upper)
        ui->widget->xAxis->setRange(ui->widget->xAxis->range().lower, ui->widget->xAxis->range().upper+20);

    ui->widget->replot();
    cnt++;
    qDebug() << ("cnt="+QString::number(cnt));
}

/********************************* CHART *************************************/





