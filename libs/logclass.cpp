#include "logclass.h"
#include "QFileInfo"
#include <QDate>
#include <QMutex>

#define LOG_FILE "log_"+(QDate::currentDate()).toString("dd_MM_yyyy")+".txt"

logClass::logClass()
{
    filelog.setFileName(LOG_FILE);
}
// Функция проходит итератором по всем файлам в текущей директории, выбирает
// файлы  логов и смотрит их даты
void logClass::autoDeleteLog()
{
    QDirIterator ligIter(QDir::currentPath(),{"*.txt"},QDir::Files);
    while (ligIter.hasNext())
    {
        // Берем следующий файл в списке, подходящий по параметрам
        QFile logFile(ligIter.next());
        QFileInfo inf(logFile); // Эта переменная нужна для возвращения имени файла без полного пути
        QString logFileName = inf.baseName(); // Сохраняем только имя файла без расширения
        if( logFileName.startsWith("log_"))
        {
            // Если это лог-файл
            if (QDate::fromString(logFileName,"'log_'dd_MM_yyyy").daysTo(QDate::currentDate())>30)
            {
                logFile.remove();
            }
        }
    }
}

void logClass::writeLog(QString text)
{
#ifdef DEBUGGING
    qDebug() << "Запись в лог: " <<  writter << text;
#else
#endif
    if (filelog.open(QIODevice::Append))
    {
#ifdef DEBUGGING
        filelog.write((writter + " " + text).toUtf8());
#else
        filelog.write(text.toUtf8());
#endif
        filelog.close();
    }
#ifdef DEBUGGING
    qDebug() << "Данные записаны в лог";
#endif
}

QString logClass::toFormat(int number){
    QString str;
    if(number < 10)
    {
        str = "0" + QString::number(number);
    }
    else
    {
        str = QString::number(number);
    }
    return str;
}

QString logClass::logTime(){
    QDate date = QDate::currentDate();
    QString year = QString::number(date.year());
    QString mounth = toFormat(date.month());
    QString day = toFormat(date.day());

    QTime time = QTime::currentTime();
    QString hour = toFormat(time.hour());
    QString minute = toFormat(time.minute());
    QString second = toFormat(time.second());

    return year + "." + mounth+ "." + day + " " + hour + ":" + minute + ":" + second;
}

void logClass::addToLog(QString logtext){

#ifdef DEBUGGING
    qDebug() << "addToLog logtext " << logtext;
#endif
    QString text = logTime() + "\t" + logtext + "\r\n";
    writeLog(text);
}

//void logClass::addToLog(QString logtext, preset& pr)
//{
//#ifdef DEBUGGING
//    qDebug() << "addToLog logtext preset" << logtext;
//#endif
//    QString text = logTime() +"\t" + logtext + "\t"+ "Число шаблонов:" + QString::number(pr.currentRecNum) + "\r\n";
//    writeLog(text);
//    for(int i=0; i<pr.presetList.size(); i++)
//    {
//        addToLog("Владелец: " + pr.presetList.at(i).name); // было 4, до изменения, теперь 0
//    }
//}

void logClass::addToLog(QString logtext,int num)
{
#ifdef DEBUGGING
    qDebug() << "addToLog logtext num" << logtext << num;
#endif
    QString text = logTime() +"\t" + logtext + "\t" + " Номер: "+QString::number(num)+"\r\n";
    writeLog(text);

}

void logClass::addToLog(QString logtext, QDir file, QString status)
{
#ifdef DEBUGGING
    qDebug() << "addToLog logtext file status" << logtext << file << status;
#endif
    QString text = logTime() +"\t" + logtext + "\t" +" Файл: "+file.path()+"\t"+" Статус: "+status+"\r\n";
    writeLog(text);
}

void logClass::addToLog(QString logtext, QStringList files)
{
#ifdef DEBUGGING
    qDebug() << "addToLog logtext files" << logtext << files;
#endif
    QString text = logTime() + "\t" + logtext + "\n";
    for (int i = 0; i<files.length(); i++){
        text+=(files.at(i));
        text+="\n";
    }
    text+="\r\n";
    writeLog(text);
}
