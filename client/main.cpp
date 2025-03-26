#include "mainwindow.h"

#include <QApplication>
#include<QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QFile qss(":/style/stylesheet.qss");
    if(qss.open(QFile::ReadOnly))
    {
        qDebug("Open success");
        //将qbyteArray转化为qstring  ----QLatin1String()
        QString style = QLatin1String(qss.readAll());
        a.setStyleSheet(style);
        qss.close();
    }
    else
    {
        qDebug("Open false");
    }

    QString filename = "config.ini";
    QString app_path = QCoreApplication::applicationDirPath();
    QString config_path =  QDir::toNativeSeparators(app_path + QDir::separator() + filename);
    qDebug()<<config_path;
    QSettings settings(config_path, QSettings::IniFormat);//ini模式,读取得模式
    QString gate_host = settings.value("GateServer/host").toString();
    QString gate_port = settings.value("GateServer/port").toString();
    qDebug()<<gate_host<<"    "<<gate_port;
    gate_url_prefix = "http://" + gate_host + ":" + gate_port;


    MainWindow w;
    w.show();
    return a.exec();
}
