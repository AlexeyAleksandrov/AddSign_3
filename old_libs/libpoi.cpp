#include "libpoi.h"

LibPOI::LibPOI(QObject *parent) : QObject(parent)
{

}

bool LibPOI::process(jar_params JarOptions)
{
    if(jar_dir == "")
    {
        qDebug() << "Не указан путь JAR";
        return false;
    }
    if(JarOptions.inputWordFile == "")
    {
        qDebug() << "Не указан файл для вставки картинки";
        return false;
    }
    if(JarOptions.imageFile == "")
    {
        qDebug() << "Не указан файл картинки для вставки в документ";
        return false;
    }

    QProcess jar_process;
    QString run_file = "C:/Windows/System32/cmd.exe";
    QStringList options = QStringList() << "/c"
                                        << jar_dir
                                        << JarOptions.inputWordFile
                                        << JarOptions.imageFile
                                        << QString::number(JarOptions.insertType)
                                        << JarOptions.outputWordFile
                                        << JarOptions.outputPdfFile
                                        << JarOptions.signOwner;
    qDebug() << "RunParams: " << options;
    jar_process.start(run_file, options); // запускаем процесс
    if(!jar_process.waitForStarted())
    {
        qDebug() << "The process didn't start" << jar_process.error();
        return false;
    }

    if(!jar_process.waitForFinished())
    {
        qDebug() << "The process didn't finished" << jar_process.error();
        return false;
    }
    if(jar_process.exitCode() != QProcess::NormalExit)
    {
        qDebug() << "Процесс завершился с ошибкой! Код: " << jar_process.exitCode();
        return false;
    }
    return true;
}

void LibPOI::setJarDir(const QString &newJar_dir)
{
    jar_dir = newJar_dir;
}
