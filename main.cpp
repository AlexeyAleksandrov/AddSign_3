#include "mainwindow.h"

#include <QApplication>
#include <QCommandLineParser>
#include <QTextCodec>
#include "systemwin32.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    #ifdef WIN32
    systemWin32 processesInWin;
    if(processesInWin.getProcessCount("AddSign.exe") > 1)
    {
        QMessageBox::warning(0, "Ошибка!", "Программа AddSign уже запущена!\n"
                                           "Запуск второй копии невозможен!");
        return 0;
    }
    #endif

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
