#include "frmmain.h"
#include "appinit.h"
#include "quihelper.h"
#include "appdata.h"

#include <QCoreApplication>
#include <QtSql>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlRecord>

int main(int argc, char *argv[])
{
    //设置不应用操作系统设置比如字体
    QApplication::setDesktopSettingsAware(false);
#if (QT_VERSION >= QT_VERSION_CHECK(5,0,0))
    QApplication::setAttribute(Qt::AA_Use96Dpi);
#endif
#if (QT_VERSION >= QT_VERSION_CHECK(5,14,0))
    QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::Floor);
#endif

    QApplication a(argc, argv);

//    数据源连接
    QSqlDatabase db = QSqlDatabase::addDatabase("QODBC");
    db.setHostName("localhost");
    db.setPort(5236);
    db.setDatabaseName("DM");
    db.setUserName("SYSDBA");
    db.setPassword("SYSDBA");

    if (db.open())
        qDebug() << "connect ok!";
    else
    {
        qDebug() << "connect fail! " << db.lastError().text();
    }
     QSqlQuery query;
     QString strsql;

//////     清空表
//     strsql = "delete from PATHFINDING.PATHTASK";
//     query.exec(strsql);

////     查询数据
//     strsql = "select * from PATHFINDING.PATHTASK";

//     if (query.exec(strsql))
//     {
//         qDebug() << "select ok!";
//     }
//     else
//     {
//         qDebug() << "select fail! " << query.lastError().text();
//         getchar();
//         exit(-1);
//     }
//     while (query.next())
//     {
////          qDebug() << query.value(0).toString().toStdString().c_str();
//         qDebug() << query.value(0);
//     }

////     插入数据
//         strsql = "insert into PATHFINDING.PATHTASK values(?,?,?,?,?)";
//         query.prepare(strsql);
//         query.bindValue(0, "{351c869d-fe30-4376-ac7a-e7e3a3638e28}");
//         query.bindValue(1, "2022-04-30");
//         query.bindValue(2, "14:56:55");
//         query.bindValue(3, "ADMIN");
//         query.bindValue(4, "（13, 7）");

//         if (query.exec())
//         {
//             qDebug() << "insert ok!";
//         }
//         else
//         {
//             qDebug() << "insert fail! " << query.lastError().text();
//             getchar();
//             exit(-1);
//         }

//     query.clear();

    AppInit::Instance()->start();

//    设置编码+字体+中文翻译文件
    QUIHelper::initAll();

    //读取配置文件
    AppConfig::ConfigFile = QString("%1/%2.ini").arg(QUIHelper::appPath()).arg(QUIHelper::appName());
    AppConfig::readConfig();

    AppData::Intervals << "1" << "10" << "20" << "50" << "100" << "200" << "300" << "500" << "1000" << "1500" << "2000" << "3000" << "5000" << "10000";
    AppData::readSendData();
    AppData::readDeviceData();

    QUIHelper::setFont();
    QUIHelper::setCode();

    //加载样式表
    QFile file(":/qss/blacksoft.css");
    if (file.open(QFile::ReadOnly)) {
        QString qss = QLatin1String(file.readAll());
        QString paletteColor = qss.mid(20, 7);

        qApp->setPalette(QPalette(QColor(paletteColor)));
        qApp->setStyleSheet(qss);
        file.close();
    }

    frmMain w;
    w.resize(800, 600);
    QUIHelper::setFormInCenter(&w);
    w.show();

    return a.exec();
}
