#ifndef LIBPOI_H
#define LIBPOI_H

#include <QObject>
#include <QProcess>
#include <QDebug>
#include <QTextCodec>


class LibPOI : public QObject
{
    Q_OBJECT
public:
    explicit LibPOI(QObject *parent = nullptr);

    enum insertTypes
    {
        simple_insert,
        insert_with_export_pdf,
        simple_export_pdf,
        insert_by_tag
    };

    struct jar_params
    {
        QString inputWordFile = "";
        QString outputWordFile = "-";
        QString outputPdfFile = "-";
        QString imageFile = "";
        int insertType = 0;
        QString signOwner = "";
    };

    void setJarDir(const QString &newJar_dir);  // установка пути запуска JAR
    bool process(jar_params JarOptions); // запуск процесса

signals:

private:
    QString jar_dir = "";   // расположение исполянемого JAR архива

};

#endif // LIBPOI_H
