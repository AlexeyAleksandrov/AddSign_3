#include "filesignprocessor.h"

#define xlPortrait 1    // вертикальная ориентация
#define xlLandscape 2   // альбомная ориентация

fileSignProcessor::fileSignProcessor(QObject *parent) : QObject(parent)
{
    log.writter = "fileSignProcessor";
}

fileSignProcessor::~fileSignProcessor()
{

}

void fileSignProcessor::run()
{
    processor.runProcessing(); // запускаем процесс подписи
    emit ready(); // посылаем сигнал о завершении
}



