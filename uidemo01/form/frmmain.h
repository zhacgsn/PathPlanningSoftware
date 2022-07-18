#ifndef FRMMAIN_H
#define FRMMAIN_H

#include <QWidget>
#include <QListWidgetItem>
#include <QDialog>
#include <QMainWindow>
#include <QPainter>
#include <QPoint>
#include <iostream>
#include <QMouseEvent>
#include <QLabel>
#include <QDebug>
#include <QPushButton>
#include <QRadioButton>
#include <cmath>
#include <QDebug>
#include <QIcon>
#include <QFile>
#include <QMessageBox>
#include <QDomDocument>
#include <QXmlStreamReader>
#include <QVector>
#include <QIODevice>
#include <QUuid>
#include <QtSql>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>
#include "astar.h"
#include "qtcpsocket.h"
#include "qextserialport.h"
#include "frmtcpserver.h"
#include "frmtcpclient.h"

extern int _MAP[30][20];

namespace Ui {
class frmMain;
}

class frmMain : public QDialog
{
    Q_OBJECT

public:
    explicit frmMain(QWidget *parent = 0);
    ~frmMain();

    QPushButton *b_set;
    QPushButton *agv_set;
    QPushButton *tg_set;
    QPushButton *assemble_set;
    QPushButton *findpath;
    QPushButton *refresh1;
    QPushButton *refresh2;
    QRadioButton *mode1;
    QRadioButton *mode2;
    class Astar *a;
    int mode;
    int fpathmode=0;
//    void paintEvent(QPaintEvent *);
    void buildMap();
    void mousePressEvent(QMouseEvent* event);
    int _starx, _stary, _endx, _endy, _assx, _assy;
    bool setstar=false;
    bool settg=false;
    bool setassemble=false;
    bool getAGVSpot = false;

    void draw(Node *current);
    frmTcpClient tcpClient;
    frmTcpServer tcpServer;


protected:
    bool eventFilter(QObject *watched, QEvent *event);

private:
    Ui::frmMain *ui;
    bool comOk;                 //串口是否打开
    QextSerialPort *com;        //串口通信对象
    QTimer *timerRead;          //定时读取串口数据
    QTimer *timerSend;          //定时发送串口数据
    QTimer *timerSave;          //定时保存串口数据

    int sleepTime;              //接收延时时间
    int sendCount;              //发送数据计数
    int receiveCount;           //接收数据计数
    bool isShow;                //是否显示数据

    bool tcpOk;                 //网络是否正常
    QTcpSocket *socket;         //网络连接对象
    QTimer *timerConnect;       //定时器重连
//    日志记录
    int count;
    QTimer *timer;

private slots:
    void initForm();
    void initConfig();
    void saveConfig();

    void buttonClick();
    void shift_to_b_set_mode();
    void shift_to_agv_set_mode();
    void shift_to_tg_set_mode();
    void shift_to_assemble_set_mode();
    void _findpath();

    void send_path_serial();
    void send_path_tcp();

    void ref1();
    void ref2();
    void _mode1();
    void _mode2();

    void readData();            //读取串口数据
    void sendData();            //发送串口数据
    void sendData(QString data);//发送串口数据带参数
    void saveData();            //保存串口数据

    void changeEnable(bool b);  //改变状态
    void append(int type, const QString &data, bool clear = false);
    void append1(const QString &flag = QString());

    void connectNet();
    void readDataNet();
    void readErrorNet();

    void sendAGVSpot();

private slots:
    void on_btnMenu_Min_clicked();
    void on_btnMenu_Max_clicked();
    void on_btnMenu_Close_clicked();

    void on_btnOpen1_clicked();
    void on_btnStopShow1_clicked();
    void on_btnSendCount1_clicked();
    void on_btnReceiveCount1_clicked();
    void on_btnClear1_clicked();
    void on_btnData1_clicked();
    void on_btnStart1_clicked();
    void on_ckAutoSend1_stateChanged(int arg1);
    void on_ckAutoSave1_stateChanged(int arg1);

    void on_btnLog_clicked();
    void on_ckTimer_stateChanged(int arg1);
//    void on_ckNet_stateChanged(int arg1);
    void on_ckSave_stateChanged(int arg1);
//    日志记录
    void on_cboxSize_currentIndexChanged(int index);
    void on_cboxRow_currentIndexChanged(int index);
    void on_listType_itemPressed(QListWidgetItem *item);
};

#endif // UIDEMO01_H
