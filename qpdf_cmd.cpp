#include "qpdf_cmd.h"

qpdf_cmd::qpdf_cmd(QObject *parent) : QObject(parent)
{

}

bool qpdf_cmd::overlay(QString upperfile, QString underfile, QString outputfile, QString options)
{
    // запускаем процесс
    QProcess qpdf_process(this);
    qpdf_process.setReadChannel(QProcess::StandardOutput);

    if(!upperfile.endsWith(".pdf") || (!underfile.endsWith(".pdf")) || (!outputfile.endsWith(".pdf")))
    {
        qDebug() << "Error on input files! It must *.pdf";
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
        return false;
    }
    if(!qpdf_process.waitForFinished())
    {
        qDebug() << "The process didn't finished" << qpdf_process.error();
        return false;
    }

    return true;
}

void qpdf_cmd::setQpdfPath(const QString &value)
{
    qpdfPath = value;
}
