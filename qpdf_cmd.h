#ifndef QPDF_CMD_H
#define QPDF_CMD_H

#include <QObject>
#include <QProcess>
#include <QDebug>

#include "logclass.h"

#ifdef __linux__
    #define QPDF_DIRECTORY "/qpdf/qpdf"
#elif _WIN32
    #define QPDF_DIRECTORY "/qpdf/qpdf.exe"
#endif

class qpdf_cmd : public QObject
{
    Q_OBJECT
public:
    explicit qpdf_cmd(QObject *parent = nullptr);

    bool overlay(QString upperfile, QString underfile, QString outputfile, QString options = "--to=z"); // оверлей файла
    bool merge(QString followingFile, QString secondFile, QString outputfile); // объединить 2 файла в один

    void setQpdfPath(const QString &value);

signals:

private:
    QString qpdfPath; // директория исполняемого файла

    logClass log;

};

#endif // QPDF_CMD_H
