#include "frmmain.h"
#include "ui_frmmain.h"
#include "iconhelper.h"
#include "quihelper.h"
#include "quihelperdata.h"

#include "ui_frmtcpclient.h"
#include "ui_frmtcpserver.h"
#include "appconfig.h"
#include "appdata.h"

#include "savelog.h"
#include "qdatetime.h"
#include "qtimer.h"
#include "qdebug.h"

#include <QTime>
#include <sys/time.h>
#include <windows.h>
#include <math.h>

int N=30, M=20;
QString strUuid;
//折线路径像素点
QPoint *pathPoints = new QPoint[500];
//路径块下标点
QPoint *indexPoints = new QPoint[500];
int nodeCount = 0;

int agvSpot_x_int;
int agvSpot_y_int;

QVector<QVector<int>> obstacles_cordinates;

frmMain::frmMain(QWidget *parent) : QDialog(parent), ui(new Ui::frmMain)
{
    ui->setupUi(this);
//    buildMap();
    this->initForm();
    this->initConfig();

    ui->stackedWidget->installEventFilter(this);

    a = new Astar();

    connect(ui->agvSpot, SIGNAL(clicked()), this, SLOT(shift_to_agv_set_mode()));
//    ui->agvSpot->setIcon(QIcon(":/new/prefix1/agvSpot.jpg"));

    connect(ui->fetchSpot, SIGNAL(clicked()),this,SLOT(shift_to_tg_set_mode()));
//    ui->fetchSpot->setIcon(QIcon(":/new/prefix1/fetchSpot.jpg"));

    connect(ui->assembleSpot, SIGNAL(clicked()),this,SLOT(shift_to_assemble_set_mode()));
//    ui->assembleSpot->setIcon(QIcon(":/new/prefix1/assembleSpot.jpg"));

    connect(ui->pathFinding, SIGNAL(clicked()),this,SLOT(_findpath()));
    connect(ui->reset, SIGNAL(clicked()),this,SLOT(ref2()));

    connect(ui->sendPathSerial, SIGNAL(clicked()),this,SLOT(send_path_serial()));
    connect(ui->sendPathTcp, SIGNAL(clicked()),this,SLOT(send_path_tcp()));

    connect(tcpClient.ui->btnSend, SIGNAL(clicked()),this,SLOT(sendAGVSpot()));

//    地图文件选择
    connect(ui->chooseFile, &QToolButton::released, [=](){
//        文件对话框 -- 返回选中文件的文件路径
        QString path = QFileDialog::getOpenFileName(this,"浏览","C:/Users/10985/Desktop/Code/QT/ui/uidemo01");
//        将路径放入到 LineEdit 控件中
        ui->fileName->setText(path);
        QFile file(path);

        file.open(QIODevice::ReadOnly);
        //通过readAll函数可以将文件内容都给读出来
        //这个函数返回一个 QByteArray 类类型的值，因此可以去接收这个返回值
        QByteArray byteArray = file.readAll();

//        取消浏览时有闪退bug

        ui->fileOutput->setText(byteArray);
        file.close();

//        xml文件处理
        file.open(QIODevice::ReadOnly);
        QXmlStreamReader reader(&file);
//        坐标二维数组
//        QVector<QVector<int>> obstacles_cordinates;
        obstacles_cordinates.resize(28);

        for (int i = 0; i < obstacles_cordinates.size(); i++)
        {
            obstacles_cordinates[i].resize(2);
        }
        QVector<int> temp;

        while(!reader.atEnd())
        {
//            判断是否是节点的开始
            if(reader.isStartElement())
            {
                if(reader.name() == "MapSize")
                {
//                    qDebug() << QString("MapSize:%1").arg(reader.readElementText());
                    QString mapSize = reader.readElementText();

                }
//                将属性读出:例如 id="1"
                QXmlStreamAttributes attributes = reader.attributes();
//                判断是否存在属性"id"
                if(attributes.hasAttribute("id"))
                {
//                    qDebug() << QString("id:%1").arg(attributes.value("id").toString());
//                    qDebug() << attributes.value("id");
                }
//                判断当前节点的名字是否为X
                if(reader.name() == "X")
                {
//                    qDebug() << QString("X:%1").arg(reader.readElementText());
                    QString x = reader.readElementText();
//                    qDebug() << "X:" << x.toInt();
//                    obstacles_cordinates[count][0] = x.toInt();
                    temp.append(x.toInt());
                }
//                判断当前节点的名字是否为Y
                else if(reader.name() == "Y")
                {
//                    qDebug() << QString("Y:%1").arg(reader.readElementText());
                    QString y = reader.readElementText();
//                    qDebug() << "Y:" << y.toInt();
//                    obstacles_cordinates[count][1] = y.toInt();
                    temp.append(y.toInt());
                }
//                判断当前节点的名字是否为Size
                else if(reader.name() == "Size")
                {
//                    qDebug() << QString("Size:%1").arg(reader.readElementText());
                }
            }
//            节点结束、并且节点名字为Obstacles（含有子节点）
            reader.readNext();
        }
//        qDebug() << temp;
        file.close();

//        坐标二维数组赋值
        int j = 0;
        for (int i = 0; i < 56; i += 2)
        {
            obstacles_cordinates[j][0] = temp[i];
            obstacles_cordinates[j][1] = temp[i + 1];
            j++;
        }
//        qDebug() << obstacles_cordinates;
    });

//    确定导入地图文件
    connect(ui->loadFile, &QToolButton::released, [=](){
        QMessageBox::information(this, "提示", "地图配置文件已导入");

//        for (int i = 0; i < N; i++)
//        {
//            for (int j = 0; j < M; j++)
//            {
//                _MAP[i][j] = 0;
//            }
//        }

        for(int i = 0; i < 28; i++)
        {
//            _MAP[obstacles_cordinates[i][0]][obstacles_cordinates[i][1]] = 1;

            for (int j = obstacles_cordinates[i][0]; j < obstacles_cordinates[i][0] + 2; j++)
            {
                for (int k = obstacles_cordinates[i][1]; k < obstacles_cordinates[i][1] + 3; k++)
                {
                    _MAP[j][k] = 1;
                }
            }
        }
        ui->mapLabel->setText("");
    });

//    取消地图文件导入
    connect(ui->noFile, &QToolButton::released, [=](){
        ui->fileOutput->clear();
        ui->fileName->clear();
    });

//    buildMap();
    for (int i = 0; i < 4; i++)
    {
        for (int j = 1; j < 1 + 5; j++)
        {
            _MAP[i][j] = 6;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 14; j < 14 + 5; j++)
        {
            _MAP[i][j] = 6;
        }
    }
}

frmMain::~frmMain()
{
    delete ui;
}

bool frmMain::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::MouseButtonDblClick) {
        if (watched == ui->widgetTitle) {
            on_btnMenu_Max_clicked();
            return true;
        }
    }

    if (ui->stackedWidget->currentIndex() == 0)
    {
        if (watched == ui->stackedWidget && event->type() == QEvent::Paint)
        {
            QPainter painter(ui->stackedWidget);
            painter.setPen(QPen(QColor("#3A3A3A"), 1.5));

//            for(int i=0;i<N;i++)
//            {
//                for(int j=0;j<M;j++)
//                {
//                    if (_MAP[i][j] == 1)
//                    {
//                        //画墙壁
//                        painter.setBrush(QBrush(QColor("#1e1b24"),Qt::SolidPattern));
//                        painter.drawRect(QRect((i + 1)*32, (j+1)*32, 32, 32));
//                    }
//                }
//            }

            for(int i=0;i<N;i++)
            {
                for(int j=0;j<M;j++)
                {
                    switch(_MAP[i][j])
                    {
                    case 1://画墙壁
                        painter.setBrush(QBrush(QColor(53, 50, 58, 127),Qt::SolidPattern));
//                        painter.setBrush(QBrush(QColor("#000000"),Qt::SolidPattern));
                        painter.drawRect(QRect((i + 1)*32, (j+1)*32, 32, 32));
                        break;
                    case 6://画区
//                        painter.setBrush(QBrush(QColor(53, 50, 58, 127),Qt::SolidPattern));
                        painter.setBrush(QBrush(QColor("#3A3A3A"),Qt::Dense5Pattern));
                        painter.drawRect(QRect((i + 1)*32, (j+1)*32, 32, 32));
                        break;
                    case 0://画空地
                        painter.setBrush(QBrush(QColor("#1e1b24"),Qt::SolidPattern));
//                        painter.setBrush(QBrush(QColor("#ffffff"),Qt::SolidPattern));
                        painter.drawRect(QRect((i + 1)*32, (j+1)*32, 32, 32));
                        break;
                    case 2://画扩展结点、路径
                        painter.setBrush(QBrush(QColor(253, 215, 170, 230), Qt::SolidPattern));
                        painter.drawRect(QRect((i + 1)*32, (j+1)*32, 32, 32));
                        break;
                    }
                    if(setstar)
                    {
                        painter.setBrush(QBrush(QColor(255, 168, 168, 127),Qt::SolidPattern));
                        painter.drawRect(QRect((_starx + 1)*32, (_stary+1)*32, 32, 32));
                    }
                    if(settg)
                    {
                        painter.setBrush(QBrush(QColor(246, 255, 164, 127),Qt::SolidPattern));
                        painter.drawRect(QRect((_endx + 1)*32, (_endy+1)*32, 32, 32));
                    }
                    if(setassemble)
                    {
                        painter.setBrush(QBrush(QColor(182, 255, 206, 127),Qt::SolidPattern));
                        painter.drawRect(QRect((_assx + 1)*32, (_assy+1)*32, 32, 32));
                    }
                    if (getAGVSpot)
                    {
                        painter.setBrush(QBrush(QColor(255, 0, 0, 100),Qt::SolidPattern));
                        painter.drawRect(QRect((agvSpot_x_int + 1)*32, (agvSpot_y_int+1)*32, 32, 32));
                    }
                }
            }

            QPen pen(QColor("#1e1b24"), 3, Qt::DashLine, Qt::RoundCap, Qt::RoundJoin);
            painter.setPen(pen);
        //    折线路径
            painter.drawPolyline(pathPoints, nodeCount);
//            qDebug() << "pathPoints" << pathPoints[0];

//            for (int i = 0; i < nodeCount; i++)
//            {
//                qDebug() << "pathPoints" << pathPoints[i].x()  << ", " << pathPoints[i].y();
//            }


        }
    }

    return QWidget::eventFilter(watched, event);
}

void frmMain::buildMap()
{
//    _MAP[0][1] = 1;_MAP[0][9] = 1;_MAP[0][10] = 1;_MAP[0][18] = 1;



    for (int i = 0; i < 4; i++)
    {
        for (int j = 1; j < 5; j++)
        {
            _MAP[i][j] = 1;
        }
    }

    for (int i = 5; i < 25; i++)
    {
        for (int j = 1; j < 19; j++)
        {
            _MAP[i][j] = 1;
        }
    }

    for (int i = 7; i < 25; i = i + 3)
    {
        for (int j = 1; j < 19; j++)
        {
            _MAP[i][j] = 0;
        }
    }

    for (int i = 5; i < 25; i++)
    {
        for (int j = 4; j < 19; j = j + 5)
        {
            _MAP[i][j] = 0;
            _MAP[i][j + 1] = 0;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 1; j < 1 + 5; j++)
        {
            _MAP[i][j] = 6;
        }
    }

    for (int i = 0; i < 4; i++)
    {
        for (int j = 14; j < 14 + 5; j++)
        {
            _MAP[i][j] = 6;
        }
    }
}

void frmMain::initForm()
{
    //设置无边框
    QUIHelper::setFramelessForm(this);
    //设置图标
//    IconHelper::setIcon(ui->labIco, 0xe694, 35);
    IconHelper::setIcon(ui->labIco, 0xe60a, 35);
    IconHelper::setIcon(ui->btnMenu_Min, 0xf068);
    IconHelper::setIcon(ui->btnMenu_Max, 0xf067);
    IconHelper::setIcon(ui->btnMenu_Close, 0xf00d);

    //ui->widgetMenu->setVisible(false);
    ui->widgetTitle->installEventFilter(this);
    ui->widgetTitle->setProperty("form", "title");
    ui->widgetTop->setProperty("nav", "top");

    QFont font;
    font.setPixelSize(25);
    ui->labTitle->setFont(font);
    ui->labTitle->setText("AGV路径规划软件 ");
    this->setWindowTitle(ui->labTitle->text());

    ui->stackedWidget->setStyleSheet("QLabel{font:9px;}");

    //单独设置指示器大小
    int addWidth = 20;
    int addHeight = 10;
    int rbtnWidth = 15;
    int ckWidth = 13;
    int scrWidth = 12;
    int borderWidth = 3;

    QStringList qss;
    qss << QString("QComboBox::drop-down,QDateEdit::drop-down,QTimeEdit::drop-down,QDateTimeEdit::drop-down{width:%1px;}").arg(addWidth);
    qss << QString("QComboBox::down-arrow,QDateEdit[calendarPopup=\"true\"]::down-arrow,QTimeEdit[calendarPopup=\"true\"]::down-arrow,"
                   "QDateTimeEdit[calendarPopup=\"true\"]::down-arrow{width:%1px;height:%1px;right:2px;}").arg(addHeight);
    qss << QString("QRadioButton::indicator{width:%1px;height:%1px;}").arg(rbtnWidth);
    qss << QString("QCheckBox::indicator,QGroupBox::indicator,QTreeWidget::indicator,QListWidget::indicator{width:%1px;height:%1px;}").arg(ckWidth);
    qss << QString("QScrollBar:horizontal{min-height:%1px;border-radius:%2px;}QScrollBar::handle:horizontal{border-radius:%2px;}"
                   "QScrollBar:vertical{min-width:%1px;border-radius:%2px;}QScrollBar::handle:vertical{border-radius:%2px;}").arg(scrWidth).arg(scrWidth / 2);
    qss << QString("QWidget#widget_top>QToolButton:pressed,QWidget#widget_top>QToolButton:hover,"
                   "QWidget#widget_top>QToolButton:checked,QWidget#widget_top>QLabel:hover{"
                   "border-width:0px 0px %1px 0px;}").arg(borderWidth);
    qss << QString("QWidget#widgetleft>QPushButton:checked,QWidget#widgetleft>QToolButton:checked,"
                   "QWidget#widgetleft>QPushButton:pressed,QWidget#widgetleft>QToolButton:pressed{"
                   "border-width:0px 0px 0px %1px;}").arg(borderWidth);
    this->setStyleSheet(qss.join(""));

    QSize icoSize(32, 32);
    int icoWidth = 120;

    //设置顶部导航按钮
    QList<QToolButton *> tbtns = ui->widgetTop->findChildren<QToolButton *>();
    foreach (QToolButton *btn, tbtns) {
        btn->setIconSize(icoSize);
        btn->setMinimumWidth(icoWidth);
        btn->setCheckable(true);
        connect(btn, SIGNAL(clicked()), this, SLOT(buttonClick()));
    }

    ui->btnMain->click();

    ui->tabWidget->addTab(&tcpClient, "AGV客户端");
    ui->tabWidget->addTab(&tcpServer, "路径规划服务端");

//    串口设置
    comOk = false;
    com = 0;
    sleepTime = 10;
    receiveCount = 0;
    sendCount = 0;
    isShow = true;

    ui->cboxSendInterval1->addItems(AppData::Intervals);
    ui->cboxData1->addItems(AppData::Datas);

    //读取数据
    timerRead = new QTimer(this);
    timerRead->setInterval(100);
    connect(timerRead, SIGNAL(timeout()), this, SLOT(readData()));

    //发送数据
    timerSend = new QTimer(this);
    connect(timerSend, SIGNAL(timeout()), this, SLOT(sendData()));
    connect(ui->btnSend1, SIGNAL(clicked()), this, SLOT(sendData()));

    //保存数据
    timerSave = new QTimer(this);
    connect(timerSave, SIGNAL(timeout()), this, SLOT(saveData()));
    connect(ui->btnSave1, SIGNAL(clicked()), this, SLOT(saveData()));

    ui->tabWidget1->setCurrentIndex(0);
    changeEnable(false);

    tcpOk = false;
    socket = new QTcpSocket(this);
    socket->abort();
    connect(socket, SIGNAL(readyRead()), this, SLOT(readDataNet()));
#if (QT_VERSION >= QT_VERSION_CHECK(6,0,0))
    connect(socket, SIGNAL(errorOccurred(QAbstractSocket::SocketError)), this, SLOT(readErrorNet()));
#else
    connect(socket, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(readErrorNet()));
#endif

    timerConnect = new QTimer(this);
    connect(timerConnect, SIGNAL(timeout()), this, SLOT(connectNet()));
    timerConnect->setInterval(3000);
    timerConnect->start();

#ifdef __arm__
    ui->widgetRight->setFixedWidth(280);
#endif

//    日志记录设置

    //启动定时器追加数据
    count = 0;
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(append1()));
    timer->setInterval(1000);

    //添加消息类型
    QStringList types, datas;
    types << "Debug" << "Info" << "Warning" << "Critical" << "Fatal";
    datas << "1" << "2" << "4" << "8" << "16";
    ui->cboxType->addItems(types);

    //添加消息类型到列表用于勾选设置哪些类型需要重定向
    int count = types.count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem *item = new QListWidgetItem;
        item->setText(types.at(i));
        item->setData(Qt::UserRole, datas.at(i));
        item->setCheckState(Qt::Checked);
        ui->listType->addItem(item);
    }

    //添加日志文件大小下拉框
    ui->cboxSize->addItem("不启用", 0);
    ui->cboxSize->addItem("5kb", 5);
    ui->cboxSize->addItem("10kb", 10);
    ui->cboxSize->addItem("30kb", 30);
    ui->cboxSize->addItem("1mb", 1024);

    ui->cboxRow->addItem("不启用", 0);
    ui->cboxRow->addItem("100条", 100);
    ui->cboxRow->addItem("500条", 500);
    ui->cboxRow->addItem("2000条", 2000);
    ui->cboxRow->addItem("10000条", 10000);

    //设置是否开启日志上下文打印比如行号、函数等
    SaveLog::Instance()->setUseContext(false);
    //设置文件存储目录
    SaveLog::Instance()->setPath(qApp->applicationDirPath() + "/log");
}

void frmMain::initConfig()
{
    ui->tabWidget->setCurrentIndex(AppConfig::CurrentIndex);
    connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(saveConfig()));

    QStringList comList;
    for (int i = 1; i <= 20; i++) {
        comList << QString("COM%1").arg(i);
    }

    comList << "ttyUSB0" << "ttyS0" << "ttyS1" << "ttyS2" << "ttyS3" << "ttyS4";
    comList << "ttymxc1" << "ttymxc2" << "ttymxc3" << "ttymxc4";
    comList << "ttySAC1" << "ttySAC2" << "ttySAC3" << "ttySAC4";
    ui->cboxPortName->addItems(comList);
    ui->cboxPortName->setCurrentIndex(ui->cboxPortName->findText(AppConfig::PortName));
    connect(ui->cboxPortName, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    QStringList baudList;
    baudList << "50" << "75" << "100" << "134" << "150" << "200" << "300" << "600" << "1200"
             << "1800" << "2400" << "4800" << "9600" << "14400" << "19200" << "38400"
             << "56000" << "57600" << "76800" << "115200" << "128000" << "256000";

    ui->cboxBaudRate->addItems(baudList);
    ui->cboxBaudRate->setCurrentIndex(ui->cboxBaudRate->findText(QString::number(AppConfig::BaudRate)));
    connect(ui->cboxBaudRate, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    QStringList dataBitsList;
    dataBitsList << "5" << "6" << "7" << "8";

    ui->cboxDataBit->addItems(dataBitsList);
    ui->cboxDataBit->setCurrentIndex(ui->cboxDataBit->findText(QString::number(AppConfig::DataBit)));
    connect(ui->cboxDataBit, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    QStringList parityList;
    parityList << "无" << "奇" << "偶";
#ifdef Q_OS_WIN
    parityList << "标志";
#endif
    parityList << "空格";

    ui->cboxParity->addItems(parityList);
    ui->cboxParity->setCurrentIndex(ui->cboxParity->findText(AppConfig::Parity));
    connect(ui->cboxParity, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    QStringList stopBitsList;
    stopBitsList << "1";
#ifdef Q_OS_WIN
    stopBitsList << "1.5";
#endif
    stopBitsList << "2";

    ui->cboxStopBit->addItems(stopBitsList);
    ui->cboxStopBit->setCurrentIndex(ui->cboxStopBit->findText(QString::number(AppConfig::StopBit)));
    connect(ui->cboxStopBit, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    ui->ckHexSend1->setChecked(AppConfig::HexSend);
    connect(ui->ckHexSend1, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));

    ui->ckHexReceive1->setChecked(AppConfig::HexReceive);
    connect(ui->ckHexReceive1, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));

    ui->ckDebug1->setChecked(AppConfig::Debug);
    connect(ui->ckDebug1, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));

    ui->ckAutoClear1->setChecked(AppConfig::AutoClear);
    connect(ui->ckAutoClear1, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));

    ui->ckAutoSend1->setChecked(AppConfig::AutoSend);
    connect(ui->ckAutoSend1, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));

    ui->ckAutoSave1->setChecked(AppConfig::AutoSave);
    connect(ui->ckAutoSave1, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));

    QStringList sendInterval;
    QStringList saveInterval;
    sendInterval << "100" << "300" << "500";

    for (int i = 1000; i <= 10000; i = i + 1000) {
        sendInterval << QString::number(i);
        saveInterval << QString::number(i);
    }

    ui->cboxSendInterval1->addItems(sendInterval);
    ui->cboxSaveInterval1->addItems(saveInterval);

    ui->cboxSendInterval1->setCurrentIndex(ui->cboxSendInterval1->findText(QString::number(AppConfig::SendInterval)));
    connect(ui->cboxSendInterval1, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));
    ui->cboxSaveInterval1->setCurrentIndex(ui->cboxSaveInterval1->findText(QString::number(AppConfig::SaveInterval)));
    connect(ui->cboxSaveInterval1, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    timerSend->setInterval(AppConfig::SendInterval);
    timerSave->setInterval(AppConfig::SaveInterval);

    if (AppConfig::AutoSend) {
        timerSend->start();
    }

    if (AppConfig::AutoSave) {
        timerSave->start();
    }

    //串口转网络部分
    ui->cboxMode1->setCurrentIndex(ui->cboxMode1->findText(AppConfig::Mode));
    connect(ui->cboxMode1, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    ui->txtServerIP1->setText(AppConfig::ServerIP);
    connect(ui->txtServerIP1, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    ui->txtServerPort1->setText(QString::number(AppConfig::ServerPort));
    connect(ui->txtServerPort1, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    ui->txtListenPort1->setText(QString::number(AppConfig::ListenPort));
    connect(ui->txtListenPort1, SIGNAL(textChanged(QString)), this, SLOT(saveConfig()));

    QStringList values;
    values << "0" << "10" << "50";

    for (int i = 100; i < 1000; i = i + 100) {
        values << QString("%1").arg(i);
    }

    ui->cboxSleepTime1->addItems(values);

    ui->cboxSleepTime1->setCurrentIndex(ui->cboxSleepTime1->findText(QString::number(AppConfig::SleepTime)));
    connect(ui->cboxSleepTime1, SIGNAL(currentIndexChanged(int)), this, SLOT(saveConfig()));

    ui->ckAutoConnect1->setChecked(AppConfig::AutoConnect);
    connect(ui->ckAutoConnect1, SIGNAL(stateChanged(int)), this, SLOT(saveConfig()));
}

void frmMain::saveConfig()
{
    AppConfig::CurrentIndex = ui->tabWidget->currentIndex();
    AppConfig::PortName = ui->cboxPortName->currentText();
    AppConfig::BaudRate = ui->cboxBaudRate->currentText().toInt();
    AppConfig::DataBit = ui->cboxDataBit->currentText().toInt();
    AppConfig::Parity = ui->cboxParity->currentText();
    AppConfig::StopBit = ui->cboxStopBit->currentText().toDouble();

    AppConfig::HexSend = ui->ckHexSend1->isChecked();
    AppConfig::HexReceive = ui->ckHexReceive1->isChecked();
    AppConfig::Debug = ui->ckDebug1->isChecked();
    AppConfig::AutoClear = ui->ckAutoClear1->isChecked();

    AppConfig::AutoSend = ui->ckAutoSend1->isChecked();
    AppConfig::AutoSave = ui->ckAutoSave1->isChecked();

    int sendInterval = ui->cboxSendInterval1->currentText().toInt();
    if (sendInterval != AppConfig::SendInterval) {
        AppConfig::SendInterval = sendInterval;
        timerSend->setInterval(AppConfig::SendInterval);
    }

    int saveInterval = ui->cboxSaveInterval1->currentText().toInt();
    if (saveInterval != AppConfig::SaveInterval) {
        AppConfig::SaveInterval = saveInterval;
        timerSave->setInterval(AppConfig::SaveInterval);
    }

    AppConfig::Mode = ui->cboxMode1->currentText();
    AppConfig::ServerIP = ui->txtServerIP1->text().trimmed();
    AppConfig::ServerPort = ui->txtServerPort1->text().toInt();
    AppConfig::ListenPort = ui->txtListenPort1->text().toInt();
    AppConfig::SleepTime = ui->cboxSleepTime1->currentText().toInt();
    AppConfig::AutoConnect = ui->ckAutoConnect1->isChecked();

    AppConfig::writeConfig();
}

void frmMain::buttonClick()
{
    QToolButton *b = (QToolButton *)sender();
    QString name = b->text();

    QList<QToolButton *> tbtns = ui->widgetTop->findChildren<QToolButton *>();
    foreach (QToolButton *btn, tbtns) {
        btn->setChecked(btn == b);
    }

    if (name == "主界面") {
        ui->stackedWidget->setCurrentIndex(0);
    } else if (name == "地图管理") {
        ui->stackedWidget->setCurrentIndex(1);
    } else if (name == "网络配置") {
        ui->stackedWidget->setCurrentIndex(2);
    } else if (name == "串口配置") {
        ui->stackedWidget->setCurrentIndex(3);
    } else if (name == "日志记录") {
        ui->stackedWidget->setCurrentIndex(4);
    }
}

void frmMain::on_btnMenu_Min_clicked()
{
    showMinimized();
}

void frmMain::on_btnMenu_Max_clicked()
{
    static bool max = false;
    static QRect location = this->geometry();

    if (max) {
        this->setGeometry(location);
    } else {
        location = this->geometry();
        this->setGeometry(QUIHelper::getScreenRect());
    }

    this->setProperty("canMove", max);
    max = !max;
}

void frmMain::on_btnMenu_Close_clicked()
{
    close();
}


void frmMain::shift_to_b_set_mode()
{
    mode=0;
//    update()时调用虚函数paintEvent()
    update();
}

void frmMain::shift_to_agv_set_mode()
{   mode=1;
    update();
}

void frmMain::shift_to_tg_set_mode()
{
    mode=2;
    update();
}

void frmMain::shift_to_assemble_set_mode()
{
    mode = 3;
    update();
}

void frmMain::mousePressEvent(QMouseEvent* event)
{
//    qDebug() << "y" << event->y();
//    qDebug() << "x" << event->x();

    if (event->y() > 106 && event->y() < 720 && event->x() > 34 && event->x() < 990)
    {
        if(event->button() == Qt::LeftButton && mode == 0)
        {
            int j = (event->y())/32 - 3, i=(event->x())/32 - 1;
            _MAP[i][j] =! _MAP[i][j];
            update();
        }

        if(event->button() == Qt::LeftButton && mode == 1)
        {
            int j = (event->y())/32 - 3, i=(event->x())/32 - 1;
            _starx = i;
            _stary = j;
            setstar = true;
            update();
        }

        if(event->button() == Qt::LeftButton && mode == 2)
        {
            int j = (event->y()) / 32 - 3, i = (event->x()) / 32 - 1;
            _endx = i;
            _endy = j;
            settg = true;
            update();
        }

        if(event->button() == Qt::LeftButton && mode == 3)
        {
            int j = (event->y()) / 32 - 3, i = (event->x()) / 32 - 1;
            _assx = i;
            _assy = j;
            setassemble = true;
            update();
        }
    }
}

void frmMain:: _findpath()
{
    if (_starx)
    {
//        QMessageBox::information(this, "提示", "请选择位于栅格地图内的点位");
    }

    Node *startPos = new Node(_starx, _stary);
    Node *endPos = new Node(_endx, _endy);

//    //记录运行时间，微秒精度
//    LARGE_INTEGER litmp;
//    LONGLONG Qpart1,Qpart2,Useingtime;
//    double dfMinus,dfFreq,dfTime;

//    //获得CPU计时器的时钟频率
//    QueryPerformanceFrequency(&litmp);//取得高精度运行计数器的频率f,单位是每秒多少次（n/s），
//    dfFreq = (double)litmp.QuadPart;
//    QueryPerformanceCounter(&litmp);//取得高精度运行计数器的数值
//    Qpart1 = litmp.QuadPart; //开始计时

    a->search(startPos,endPos);

//    QueryPerformanceCounter(&litmp);//取得高精度运行计数器的数值
//    Qpart2 = litmp.QuadPart; //终止计时
//    dfMinus = (double)(Qpart2 - Qpart1);//计算计数器值
//    dfTime = dfMinus / dfFreq;//获得对应时间，单位为秒,可以乘1000000精确到微秒级（us）
//    Useingtime = dfTime*1000000;
//    qDebug()<<dfTime<<"s";

    draw(a->path);

    Node *assPos = new Node(_assx, _assy);
    a->search(endPos,assPos);
    draw(a->path);


//    插入操作日志
    QUuid uuid = QUuid::createUuid();
    strUuid = uuid.toString();

    append1(QString("完成取货点（%1, %2），装配点（%3, %4）的路径规划，任务编号 %5").arg(_endx).arg(_endy).arg(_assx).arg(_assy).arg(strUuid));

//    qDebug() << strUuid;

//    日志写入数据库
    QString Task_DATE = QDate::currentDate().toString("yyyy-MM-dd");
    QString Task_TIME = QTime::currentTime().toString();

    QSqlQuery query;
    QString strsql;

    strsql = "insert into PATHFINDING.PATHTASK values(?,?,?,?,?,?)";
    query.prepare(strsql);
    query.bindValue(0, strUuid);
    query.bindValue(1, Task_DATE);
    query.bindValue(2, Task_TIME);
    query.bindValue(3, "ADMIN");
    query.bindValue(4, QString("（%1, %2）").arg(_endx).arg(_endy));
    query.bindValue(5, QString("（%1, %2）").arg(_assx).arg(_assy));

    query.exec();
}

void frmMain::send_path_serial()
{
    for (int i = 0; i < nodeCount; i++)
    {
//        qDebug() << "pathPoint:" << pathPoints[i].x() << ", " << pathPoints[i].y();

//        指令编码格式（十六进制）
//        3位 |5位 B
//        1位  |1位 H
//        操作码|x坐标或y坐标
//        假设操作码3位，001代表AGV移动指令，011为位置上报指令
//        如00110111B
//        31 2A H
//        001 10001 001 01010 B移动
//        011 10001 011 01010 B位置
//        71 6A H
//        17 10
        QString op = "001";
        QString xHex = QString("%1").arg(indexPoints[i].x(), 5, 2, QLatin1Char('0'));
        QString yHex = QString("%1").arg(indexPoints[i].y(), 5, 2, QLatin1Char('0'));

        QString highFourX = op + xHex.mid(0, 1);
        QString lowFourX = xHex.mid(1, 4);

        QString highFourY = op + yHex.mid(0, 1);
        QString lowFourY = yHex.mid(1, 4);

        bool ok;
//        二进制指令
        QString strX = QString("%1%2").arg(highFourX).arg(lowFourX);
        int valX = strX.toInt(&ok, 2);
        strX = QString::number(valX, 16);
        strX = strX.toUpper();

        QString strY = QString("%1%2").arg(highFourY).arg(lowFourY);
        int valY = strY.toInt(&ok, 2);
        strY = QString::number(valY, 16);
        strY = strY.toUpper();

        sendData(QString("%1 %2").arg(strX).arg(strY));
    }
}

void frmMain::send_path_tcp()
{
    tcpServer.ui->btnSend->click();
}

void frmMain::sendAGVSpot()
{
    QString spot = tcpClient.ui->cboxData->currentText();
    QString spotX = spot.mid(0, 2);
    QString spotY = spot.mid(3, 2);
//    qDebug() << spotX << " " << spotY;

    bool ok;
    int val1 = spotX.toInt(&ok, 16);
    int val2 = spotY.toInt(&ok, 16);

    spotX = spotX.setNum(val1, 2);
    spotY = spotY.setNum(val2, 2);

//    spotX = spotX.mid()

//    qDebug() << spotX << " " << spotY;

//        011 10001 011 01010 B 位置指令
//        71 6A H

//        011 01011 011 01110 B 位置指令
//        6B 6E H
    QString agvSpot_x = spotX.mid(2, 5);
    QString agvSpot_y = spotY.mid(2, 5);

//    qDebug() << agvSpot_x << ", " << agvSpot_y;


    agvSpot_x_int = agvSpot_x.toInt(&ok, 2);
    agvSpot_y_int = agvSpot_y.toInt(&ok, 2);

//    qDebug() << agvSpot_x_int << ", " << agvSpot_y_int;

//    ui->stackedWidget->setCurrentIndex(0);
    ui->btnMain->click();

//    控制是否标记AGV
    getAGVSpot = true;
}

void frmMain::draw(Node *current)
{
    if (current->father != nullptr)
       draw(current->father);

   _MAP[current->x][current->y]=2;
//   记录路径节点下标坐标
   indexPoints[nodeCount].setX(current->x);
   indexPoints[nodeCount].setY(current->y);

//   记录路径节点像素坐标
   pathPoints[nodeCount].setX((current->x + 1.5) * 32);
   pathPoints[nodeCount].setY((current->y + 1.5) * 32);

//   qDebug() << "pathPoints:" << pathPoints[nodeCount].x()  << ", " << pathPoints[nodeCount].y();

   nodeCount++;
   update();
}

void frmMain:: ref1()
{
    for (int i=0;i<N;i++)
        for(int j=0;j<M;j++)
        {
            if(_MAP[i][j]==1||_MAP[i][j]==2)
                _MAP[i][j]=0;
        }
    update();
}

void frmMain:: ref2()
{
    for (int i=0;i<N;i++)
        for(int j=0;j<M;j++)
        {
            if(_MAP[i][j]==2)
                _MAP[i][j]=0;
        }
    nodeCount = 0;
    getAGVSpot = false;
    update();
}

void  frmMain::_mode1()
{
//    a->mode=true;
}

void  frmMain::_mode2()
{
//    a->mode=false;
}


void frmMain::changeEnable(bool b)
{
    ui->cboxBaudRate->setEnabled(!b);
    ui->cboxDataBit->setEnabled(!b);
    ui->cboxParity->setEnabled(!b);
    ui->cboxPortName->setEnabled(!b);
    ui->cboxStopBit->setEnabled(!b);
    ui->btnSend1->setEnabled(b);
    ui->ckAutoSend1->setEnabled(b);
    ui->ckAutoSave1->setEnabled(b);
}

void frmMain::append(int type, const QString &data, bool clear)
{
    static int currentCount = 0;
    static int maxCount = 100;

    if (clear) {
        ui->txtMain1->clear();
        currentCount = 0;
        return;
    }

    if (currentCount >= maxCount) {
        ui->txtMain1->clear();
        currentCount = 0;
    }

    if (!isShow) {
        return;
    }

    //过滤回车换行符
    QString strData = data;
    strData = strData.replace("\r", "");
    strData = strData.replace("\n", "");

    //不同类型不同颜色显示
    QString strType;
    if (type == 0) {
        strType = "串口发送 >>";
        ui->txtMain1->setTextColor(QColor("#e2e2e2"));
    } else if (type == 1) {
        strType = "串口接收 <<";
        ui->txtMain1->setTextColor(QColor("#5500e9"));
    } else if (type == 2) {
        strType = "处理延时 >>";
        ui->txtMain1->setTextColor(QColor("gray"));
    } else if (type == 3) {
        strType = "正在校验 >>";
        ui->txtMain1->setTextColor(QColor("green"));
    } else if (type == 4) {
        strType = "网络发送 >>";
        ui->txtMain1->setTextColor(QColor(24, 189, 155));
    } else if (type == 5) {
        strType = "网络接收 <<";
        ui->txtMain1->setTextColor(QColor(255, 107, 107));
    } else if (type == 6) {
        strType = "提示信息 >>";
        ui->txtMain1->setTextColor(QColor(100, 184, 255));
    }

    strData = QString("时间[%1] %2 %3").arg(TIMEMS).arg(strType).arg(strData);
    ui->txtMain1->append(strData);
    currentCount++;
}

void frmMain::readData()
{
    if (com->bytesAvailable() <= 0) {
        return;
    }

    QUIHelper::sleep(sleepTime);
    QByteArray data = com->readAll();
    int dataLen = data.length();
    if (dataLen <= 0) {
        return;
    }

    if (isShow) {
        QString buffer;
        if (ui->ckHexReceive1->isChecked()) {
            buffer = QUIHelperData::byteArrayToHexStr(data);
        } else {
            //buffer = QUIHelperData::byteArrayToAsciiStr(data);
            buffer = QString::fromLocal8Bit(data);
        }

        //启用调试则模拟调试数据
        if (ui->ckDebug1->isChecked()) {
            int count = AppData::Keys.count();
            for (int i = 0; i < count; i++) {
                if (buffer.startsWith(AppData::Keys.at(i))) {
                    sendData(AppData::Values.at(i));
                    break;
                }
            }
        }

        append(1, buffer);
        receiveCount = receiveCount + data.size();
        ui->btnReceiveCount1->setText(QString("接收 : %1 字节").arg(receiveCount));

        //启用网络转发则调用网络发送数据
        if (tcpOk) {
            socket->write(data);
            append(4, QString(buffer));
        }
    }
}

void frmMain::sendData()
{
    QString str = ui->cboxData1->currentText();
    if (str.isEmpty()) {
        ui->cboxData1->setFocus();
        return;
    }

    sendData(str);

    if (ui->ckAutoClear1->isChecked()) {
        ui->cboxData1->setCurrentIndex(-1);
        ui->cboxData1->setFocus();
    }
}

void frmMain::sendData(QString data)
{
    if (com == 0 || !com->isOpen()) {
        return;
    }

    //短信猫调试
    if (data.startsWith("AT")) {
        data += "\r";
    }

    QByteArray buffer;
    if (ui->ckHexSend1->isChecked()) {
        buffer = QUIHelperData::hexStrToByteArray(data);
    } else {
        buffer = QUIHelperData::asciiStrToByteArray(data);
    }

    com->write(buffer);
    append(0, data);
    sendCount = sendCount + buffer.size();
    ui->btnSendCount1->setText(QString("发送 : %1 字节").arg(sendCount));
}

void frmMain::saveData()
{
    QString tempData = ui->txtMain1->toPlainText();

    if (tempData == "") {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();
    QString name = now.toString("yyyy-MM-dd-HH-mm-ss");
    QString fileName = QString("%1/%2.txt").arg(QUIHelper::appPath()).arg(name);

    QFile file(fileName);
    file.open(QFile::WriteOnly | QIODevice::Text);
    QTextStream out(&file);
    out << tempData;
    file.close();

    on_btnClear1_clicked();
}

void frmMain::on_btnOpen1_clicked()
{
    if (ui->btnOpen1->text() == "打开串口") {
        com = new QextSerialPort(ui->cboxPortName->currentText(), QextSerialPort::Polling);
        comOk = com->open(QIODevice::ReadWrite);

        if (comOk) {
            //清空缓冲区
            com->flush();
            //设置波特率
            com->setBaudRate((BaudRateType)ui->cboxBaudRate->currentText().toInt());
            //设置数据位
            com->setDataBits((DataBitsType)ui->cboxDataBit->currentText().toInt());
            //设置校验位
            com->setParity((ParityType)ui->cboxParity->currentIndex());
            //设置停止位
            com->setStopBits((StopBitsType)ui->cboxStopBit->currentIndex());
            com->setFlowControl(FLOW_OFF);
            com->setTimeout(10);

            changeEnable(true);
            ui->btnOpen1->setText("关闭串口");
            timerRead->start();
        }
    } else {
        timerRead->stop();
        com->close();
        com->deleteLater();

        changeEnable(false);
        ui->btnOpen1->setText("打开串口");
        on_btnClear1_clicked();
        comOk = false;
    }
}

void frmMain::on_btnSendCount1_clicked()
{
    sendCount = 0;
    ui->btnSendCount1->setText("发送 : 0 字节");
}

void frmMain::on_btnReceiveCount1_clicked()
{
    receiveCount = 0;
    ui->btnReceiveCount1->setText("接收 : 0 字节");
}

void frmMain::on_btnStopShow1_clicked()
{
    if (ui->btnStopShow1->text() == "停止显示") {
        isShow = false;
        ui->btnStopShow1->setText("开始显示");
    } else {
        isShow = true;
        ui->btnStopShow1->setText("停止显示");
    }
}

void frmMain::on_btnData1_clicked()
{
    QString fileName = QString("%1/%2").arg(QUIHelper::appPath()).arg("send.txt");
    QFile file(fileName);
    if (!file.exists()) {
        return;
    }

    if (ui->btnData1->text() == "管理数据") {
        ui->txtMain1->setReadOnly(false);
        ui->txtMain1->clear();
        file.open(QFile::ReadOnly | QIODevice::Text);
        QTextStream in(&file);
        ui->txtMain1->setText(in.readAll());
        file.close();
        ui->btnData1->setText("保存数据");
    } else {
        ui->txtMain1->setReadOnly(true);
        file.open(QFile::WriteOnly | QIODevice::Text);
        QTextStream out(&file);
        out << ui->txtMain1->toPlainText();
        file.close();
        ui->txtMain1->clear();
        ui->btnData1->setText("管理数据");
        AppData::readSendData();
    }
}

void frmMain::on_btnClear1_clicked()
{
    append(0, "", true);
}

void frmMain::on_btnStart1_clicked()
{
    if (ui->btnStart1->text() == "启动") {
        if (AppConfig::ServerIP == "" || AppConfig::ServerPort == 0) {
            append(6, "IP地址和远程端口不能为空");
            return;
        }

        socket->connectToHost(AppConfig::ServerIP, AppConfig::ServerPort);
        if (socket->waitForConnected(100)) {
            ui->btnStart1->setText("停止");
            append(6, "连接服务器成功");
            tcpOk = true;
        }
    } else {
        socket->disconnectFromHost();
        if (socket->state() == QAbstractSocket::UnconnectedState || socket->waitForDisconnected(100)) {
            ui->btnStart1->setText("启动");
            append(6, "断开服务器成功");
            tcpOk = false;
        }
    }
}

void frmMain::on_ckAutoSend1_stateChanged(int arg1)
{
    if (arg1 == 0) {
        ui->cboxSendInterval1->setEnabled(false);
        timerSend->stop();
    } else {
        ui->cboxSendInterval1->setEnabled(true);
        timerSend->start();
    }
}

void frmMain::on_ckAutoSave1_stateChanged(int arg1)
{
    if (arg1 == 0) {
        ui->cboxSaveInterval1->setEnabled(false);
        timerSave->stop();
    } else {
        ui->cboxSaveInterval1->setEnabled(true);
        timerSave->start();
    }
}

void frmMain::connectNet()
{
    if (!tcpOk && AppConfig::AutoConnect && ui->btnStart1->text() == "启动") {
        if (AppConfig::ServerIP != "" && AppConfig::ServerPort != 0) {
            socket->connectToHost(AppConfig::ServerIP, AppConfig::ServerPort);
            if (socket->waitForConnected(100)) {
                ui->btnStart1->setText("停止");
                append(6, "连接服务器成功");
                tcpOk = true;
            }
        }
    }
}

void frmMain::readDataNet()
{
    if (socket->bytesAvailable() > 0) {
        QUIHelper::sleep(AppConfig::SleepTime);
        QByteArray data = socket->readAll();

        QString buffer;
        if (ui->ckHexReceive1->isChecked()) {
            buffer = QUIHelperData::byteArrayToHexStr(data);
        } else {
            buffer = QUIHelperData::byteArrayToAsciiStr(data);
        }

        append(5, buffer);

        //将收到的网络数据转发给串口
        if (comOk) {
            com->write(data);
            append(0, buffer);
        }
    }
}

void frmMain::readErrorNet()
{
    ui->btnStart1->setText("启动");
    append(6, QString("连接服务器失败,%1").arg(socket->errorString()));
    socket->disconnectFromHost();
    tcpOk = false;
}

//日志记录

void frmMain::append1(const QString &flag)
{
    if (count >= 100) {
        count = 0;
        ui->txtMain->clear();
    }

    QString str1;
    int type = ui->cboxType->currentIndex();
    if (!ui->ckSave->isChecked()) {
        if (type == 0) {
            str1 = "Debug ";
        } else if (type == 1) {
            str1 = "Infox ";
        } else if (type == 2) {
            str1 = "Warnx ";
        } else if (type == 3) {
            str1 = "Error ";
        } else if (type == 4) {
            str1 = "Fatal ";
        }
    }

    QString str2 = QDateTime::currentDateTime().toString("yyyy年MM月dd日 HH:mm:ss");
    QString str3 = flag.isEmpty() ? "自动插入消息" : flag;
//    QString msg = QString("%1当前时间: %2 %3").arg(str1).arg(str2).arg(str3);
    QString msg = QString(" %1 %2").arg(str2).arg(str3);

    //开启网络重定向换成英文方便接收解析不乱码
    //对方接收解析的工具未必是utf8
//    if (ui->ckNet->isChecked()) {
//        msg = QString("%1time: %2 %3").arg(str1).arg(str2).arg("(QQ: 517216493 WX: feiyangqingyun)");
//    }

    count++;
    ui->txtMain->append(msg);

    //根据不同的类型打印
    //TMD转换要分两部走不然msvc的debug版本会乱码(英文也一样)
    //char *data = msg.toUtf8().data();
    QByteArray buffer = msg.toUtf8();
    const char *data = buffer.constData();

    if (type == 0) {
        qDebug(data);
    } else if (type == 1) {
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
        qInfo(data);
#endif
    } else if (type == 2) {
        qWarning(data);
    } else if (type == 3) {
        qCritical(data);
    } else if (type == 4) {
        //调用下面这个打印完会直接退出程序
        qFatal(data);
    }
}

void frmMain::on_btnLog_clicked()
{
    //    QString(" %1 %2").arg(str2).arg(str3)
    append1(QString("完成取货点（%3, %4），装配点（%5, %6）的路径规划，任务编号 %7").arg(_starx).arg(_stary).arg(_endx).arg(_endy).arg(_assx).arg(_assy).arg(strUuid));
}

void frmMain::on_ckTimer_stateChanged(int arg1)
{
    if (arg1 == 0) {
        timer->stop();
    } else {
        timer->start();
//        on_btnLog_clicked();
    }
}

//void frmMain::on_ckNet_stateChanged(int arg1)
//{
//    SaveLog::Instance()->setListenPort(ui->txtPort->text().toInt());
//    SaveLog::Instance()->setToNet(ui->ckNet->isChecked());
////    on_btnLog_clicked();
//}

void frmMain::on_ckSave_stateChanged(int arg1)
{
    if (arg1 == 0) {
        SaveLog::Instance()->stop();
    } else {
        SaveLog::Instance()->start();
//        on_btnLog_clicked();
    }
}

void frmMain::on_cboxSize_currentIndexChanged(int index)
{
    int size = ui->cboxSize->itemData(index).toInt();
    SaveLog::Instance()->setMaxSize(size);
//    on_btnLog_clicked();
}

void frmMain::on_cboxRow_currentIndexChanged(int index)
{
    int row = ui->cboxRow->itemData(index).toInt();
    SaveLog::Instance()->setMaxRow(row);
//    on_btnLog_clicked();
}

void frmMain::on_listType_itemPressed(QListWidgetItem *item)
{
    //切换选中行状态
    item->setCheckState(item->checkState() == Qt::Checked ? Qt::Unchecked : Qt::Checked);

    //找到所有勾选的类型进行设置
    quint8 types = 0;
    int count = ui->listType->count();
    for (int i = 0; i < count; ++i) {
        QListWidgetItem *item = ui->listType->item(i);
        if (item->checkState() == Qt::Checked) {
            types += item->data(Qt::UserRole).toInt();
        }
    }

    SaveLog::Instance()->setMsgType((MsgType)types);
}

