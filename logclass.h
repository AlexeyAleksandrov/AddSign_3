#ifndef LOGCLASS_H
#define LOGCLASS_H
//#include "preset.h"
#include "QFile"
#include "QDir"
#include "qdiriterator.h"
//#include "wordparams.h"

//#define DEBUGGING

class logClass
{
public:
    logClass();

    QFile filelog;
    QString text;

    QString writter; // название класса, который производит запись

    QString toFormat(int number);
    QString logTime();

    void writeLog(QString text);

    void addToLog(QString logtext);
//    void addToLog(QString logtext, preset &pr);
    void addToLog(QString logtext, int num);
    void addToLog(QString logtext, QStringList files);
    void addToLog(QString logtext, QDir file, QString status);

    void autoDeleteLog();
};

#endif // LOGCLASS_H
