#include "libreoffice.h"

LibreOffice::LibreOffice(QObject *parent) : QObject(parent)
{
    setTempDir(QDir::currentPath() + "/tempLibreOffice/"); // папка для временных файлов по умолчанию
}

void LibreOffice::convertFile(QString inputFile, QString outputFile, LibreOfficeFormats inputFormat, bool *ok)
{
    if(ok != nullptr)
    {
        *ok = false;
    }
    // проверем, что исходный    файл существует
    if(!QFile(inputFile).exists())  // если файл, который подаётся на вход не существует
    {
        qDebug() << "LibreOffice: Файл не найден: " + inputFile;
        return;
    }
    if(outputFile != "" && !outputFile.endsWith(".pdf"))
    {
        qDebug() << "LibreOffice: Выходной файл должен быть формата *.pdf : " + outputFile;
        return;
    }

//    QString outFile = inputFile.replace("." + getFormatName(inputFormat), ".pdf");
    QString outFile = tempDir + QFileInfo(inputFile).fileName().replace("." + getFormatName(inputFormat), ".pdf");
    if(!outFile.endsWith(".pdf"))
    {
        qDebug() << "LibreOffice: Ошибка формата файла: " + inputFile + " Формат: " + getFormatName(inputFormat);
        return;
    }
    QFile tempFileOut(outFile);
    if(tempFileOut.exists())
    {
        tempFileOut.remove();   // удаляем временный файл
    }

    QProcess process;
    QStringList params;
    params.append("--headless");
    params.append("--convert-to");
    if(inputFormat == docx || inputFormat == doc || inputFormat == rtf)
    {
       params.append("pdf:writer_pdf_Export");
    }
    else if(inputFormat == xlsx || inputFormat == xls)
    {
       params.append("pdf:calc_pdf_Export");
    }
    params.append("--outdir");
    params.append(tempDir);
    params.append(inputFile);

    process.start(sofficePath, params);

    if(!process.waitForStarted())   // ожидаем запуска процесса
    {
        qDebug() << "LibreOffice: Процесс не был запущен!";
        return;
    }
    if(!process.waitForFinished())  // ожидаем завершения процесса
    {
        qDebug() << "LibreOffice: Процесс не было завершён в заданное время";
        return;
    }

    bool correct = false;

    // делаем проверку
    correct = tempFileOut.exists();    // устанавливаем статус результата

    if(correct)
    {
        if(outputFile != "")
        {
            QFile fileOut(outputFile);  // выходной файл

            tempFileOut.open(QIODevice::ReadOnly);
            fileOut.open(QIODevice::WriteOnly);

            QByteArray data = tempFileOut.readAll();
            fileOut.write(data);

            tempFileOut.close();
            fileOut.close();
        }
    }
    else
    {
        qDebug() << "LibreOffice: Файл не был сформирован: " + outFile;
        return;
    }
    if(ok != nullptr)
    {
        *ok = correct;
    }
    qDebug() << "LibreOffice: Файл готов: " + outputFile;
}

void LibreOffice::setTempDir(const QString &newTempDir)
{
    QString tDir = newTempDir;
    if(tDir.contains("\\"))
    {
        tDir.replace("\\", "/");
    }
    if(!tDir.endsWith("/"))
    {
        tDir.append("/");
    }
    tempDir = newTempDir;
    QDir dir(tempDir);
    if(!dir.exists())
    {
        dir.mkdir(tempDir);
    }
}

QString LibreOffice::getFormatName(LibreOfficeFormats format)
{
    switch (format)
    {
        case docx: return "docx";
        case doc: return "doc";
        case rtf: return "rtf";
        case xlsx: return "xlsx";
        case xls: return "xls";
        case pdf: return "pdf";
        default: return "";
    }
}


void LibreOffice::setSofficePath(const QString &newSofficePath)
{
    sofficePath = newSofficePath;
}
