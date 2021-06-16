#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

//    QCommandLineParser parser;

//    QCommandLineOption name_param(QStringList() << "s" << "signfile", "File", "file for sign");
//    QCommandLineOption port_param(QStringList() << "a" << "addsign", "sign", "create sign");
//    parser.addOption(name_param);
//    parser.addOption(port_param);
//    parser.process(a);

//    if(parser.isSet(name_param))
//        qDebug()<<"name is " << parser.value(name_param);
//    if(parser.isSet(port_param))
//        qDebug()<<"port is " << parser.value(port_param);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("IBM 866"));

    // получаем аргументы
    QStringList args;
    for(int i=1; i<argc; i++) // начинаем с 1, т.к. 0 - это директория запуска, а она не нужна
    {
        args.append(a.arguments().at(i));
//        qDebug() << a.arguments().at(i);
    }

    MainWindow w;
    w.show();
    w.setArgs(args); // передаём аргументы запуска
    w.customConstructor();
    a.setWindowIcon(QIcon(":/images/MIREA_logo.ico"));

    return a.exec();
}
