#ifndef QPDF_CMD_H
#define QPDF_CMD_H

#include <QObject>
#include <QProcess>
#include <QDebug>

class qpdf_cmd : public QObject
{
    Q_OBJECT
public:
    explicit qpdf_cmd(QObject *parent = nullptr);

    bool overlay(QString upperfile, QString underfile, QString outputfile, QString options = "--to=z"); // оверлей файла

    void setQpdfPath(const QString &value);

signals:

private:
    QString qpdfPath; // директория исполняемого файла

};

#endif // QPDF_CMD_H
