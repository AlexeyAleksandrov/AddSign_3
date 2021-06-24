#include "qpdf_cmd.h"

qpdf_cmd::qpdf_cmd(QObject *parent) : QObject(parent)
{
    QString qpdfFile = QDir::currentPath() + "/qpdf/qpdf.exe";
    if(QFile::exists(qpdfFile))
    {
        this->qpdfPath = qpdfFile;
        qDebug() << "Файл QPDF найден!";
    }
}

bool qpdf_cmd::overlay(QString upperfile, QString underfile, QString outputfile, QString options)
{
    // запускаем процесс
    log.addToLog("Запускаем процесс QPDF. upperfile = " + upperfile + " underfile = " + underfile + " outputfile = " + outputfile + " options = " + options);
    QProcess qpdf_process(this);
    qpdf_process.setReadChannel(QProcess::StandardOutput);

    if(!upperfile.endsWith(".pdf") || (!underfile.endsWith(".pdf")) || (!outputfile.endsWith(".pdf")))
    {
        if(!upperfile.endsWith(".pdf"))
        {
            qDebug() << "Ошибка! Файл upperfile должен бать формата PDF " + upperfile;
            log.addToLog("Ошибка! Файл upperfile должен бать формата PDF " + upperfile);
        }
        if(!underfile.endsWith(".pdf"))
        {
            qDebug() << "Ошибка! Файл underfile должен бать формата PDF " + underfile;
            log.addToLog("Ошибка! Файл underfile должен бать формата PDF " + underfile);
        }
        if(!outputfile.endsWith(".pdf"))
        {
            qDebug() << "Ошибка! Файл outputfile должен бать формата PDF " + outputfile;
            log.addToLog("Ошибка! Файл outputfile должен бать формата PDF " + outputfile);
        }
        return false;
    }

    QString underlayFile = underfile; // нижний файл (на него накладываем)
    QString overlayFile = upperfile; // верхний файл (его накладываем сверху)
    QString outputFile = outputfile; // выходной файл

    QStringList params = QStringList() << "--overlay" << overlayFile << options << "--" << underlayFile << outputFile;

    qpdf_process.start(qpdfPath, params); // запускаем процесс
    if(!qpdf_process.waitForStarted())
    {
        qDebug() << "The process didn't start" << qpdf_process.error();
        log.addToLog(&"The process didn't start " [ qpdf_process.error()]);
        return false;
    }
    if(!qpdf_process.waitForFinished())
    {
        qDebug() << "The process didn't finished" << qpdf_process.error();
        log.addToLog(&"The process didn't finished " [ qpdf_process.error()]);
        return false;
    }

    log.addToLog("Процесс QPDF успешно завершён");
    return true;
}

bool qpdf_cmd::merge(QString followingFile, QString secondFile, QString outputfile)
{
    // запускаем процесс
    log.addToLog("Запускаем процесс QPDF merge. upperfile = " + followingFile + " underfile = " + secondFile + " outputfile = " + outputfile );
    QProcess qpdf_process(this);
    qpdf_process.setReadChannel(QProcess::StandardOutput);

    if(!followingFile.endsWith(".pdf") || (!secondFile.endsWith(".pdf")) || (!outputfile.endsWith(".pdf")))
    {
        if(!followingFile.endsWith(".pdf"))
        {
            qDebug() << "Ошибка! Файл upperfile должен бать формата PDF " + followingFile;
            log.addToLog("Ошибка! Файл upperfile должен бать формата PDF " + followingFile);
        }
        if(!secondFile.endsWith(".pdf"))
        {
            qDebug() << "Ошибка! Файл underfile должен бать формата PDF " + secondFile;
            log.addToLog("Ошибка! Файл underfile должен бать формата PDF " + secondFile);
        }
        if(!outputfile.endsWith(".pdf"))
        {
            qDebug() << "Ошибка! Файл outputfile должен бать формата PDF " + outputfile;
            log.addToLog("Ошибка! Файл outputfile должен бать формата PDF " + outputfile);
        }
        return false;
    }

    QStringList params = QStringList() << "--empty" << "--pages" << followingFile << secondFile << "--" << outputfile;

    qpdf_process.start(qpdfPath, params); // запускаем процесс
    if(!qpdf_process.waitForStarted())
    {
        qDebug() << "The process didn't start" << qpdf_process.error();
        log.addToLog(&"The process didn't start " [ qpdf_process.error()]);
        return false;
    }
    if(!qpdf_process.waitForFinished())
    {
        qDebug() << "The process didn't finished" << qpdf_process.error();
        log.addToLog(&"The process didn't finished " [ qpdf_process.error()]);
        return false;
    }

    log.addToLog("Процесс QPDF успешно завершён");
    return true;
}

void qpdf_cmd::setQpdfPath(const QString &value)
{
    qpdfPath = value;
}
