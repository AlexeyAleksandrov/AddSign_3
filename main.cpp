#include "mainwindow.h"

#include <QApplication>
#include <QTextCodec>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("IBM 866"));

    // получаем аргументы
    QStringList args;
    for(int i=1; i<argc; i++) // начинаем с 1, т.к. 0 - это директория запуска, а она не нужна
    {
        QString arg = argv[i];
        args.append(arg); // добавляем аргумент в список
    }

    MainWindow w;
    w.show();
    w.setArgs(args); // передаём аргументы запуска
    w.customConstructor();
    a.setWindowIcon(QIcon(":/images/MIREA_logo.ico"));

    return a.exec();
}
