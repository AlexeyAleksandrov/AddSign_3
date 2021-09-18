#include "signprocessor.h"

#include <QApplication>
#include <QDir>

//#define MIREA_LOGO_HTML ":/img/MIREA_logo_resize.png"
#define MIREA_LOGO_HTML PDFOptions.image_dir

#define DISPLAY_STATUS_ON_NEW_PAGE
#define assert(var, texterror) if(var == false) { qDebug() << "Assert: " << texterror; log.addToLog(texterror); return; }
#define check_file_status(status) if(status != files_status::in_process){continue;}

#if QT_VERSION >= 0x050f00 // версия Qt 5.15.0
#define SPLITTER Qt::SplitBehavior(Qt::SkipEmptyParts)
#else
#define SPLITTER QString::SkipEmptyParts
#endif

SignProcessor::SignProcessor(QObject *parent) : QObject(parent)
{

}

void SignProcessor::setFilesList(const QList<FileForSign> &value)
{
    filesList = value;
}

void SignProcessor::setFilesList(const QStringList &files)
{
    QList<FileForSign> filesList; // создаем список файлов
    for(auto &&inputFile : files)
    {
        log.addToLog("добавляем файл " + inputFile);
        FileForSign file; // создаем экземпляр файла
        file.sourceFile = inputFile;

        if(WordOptions.outputdir == "") // если редачим оригинал файла
        {
            if(WordOptions.exportToWord) // если нужно подписать исодный Word
            {
                file.signWordFile = inputFile;
            }
            if(((WordOptions.insertType == insert_standart || WordOptions.insertType == insert_by_tag_in_table) && WordOptions.exportToPDF) || WordOptions.insertType == insert_in_exported_pdf) // если нужно экспортировать в PDF
            {
                file.signPDFFile = getFileNameInPDFFormat(inputFile); // просто меняем расщирение файла, а пуь оставляем тот же самый
            }
        }
        else // если экспортируем в какую-то папку
        {
            if(WordOptions.exportToWord) // если нужно подписать Word файл
            {
                file.signWordFile = WordOptions.getOutputdir() + getFileName(file.sourceFile); // формируем путь к ворду, который экспортируем
            }
            if(((WordOptions.insertType == insert_standart || WordOptions.insertType == insert_by_tag_in_table) && WordOptions.exportToPDF) || WordOptions.insertType == insert_in_exported_pdf) // если нужно экспортировать в PDF
            {
                file.signPDFFile = getFileNameInPDFFormat(WordOptions.getOutputdir() + getFileName(file.sourceFile)); // тоже самое, но меняем расширение файла
                qDebug() << "PDF filename: " << file.signPDFFile;
            }
        }
        filesList.append(file); // добавляем в список
    }
    setFilesList(filesList); // вызываем обычную функцию установки файлов
    log.addToLog("Все файлы добавлены в список");
}

void SignProcessor::setWordOptions(const WordParams &value)
{
    WordOptions = value;
}

void SignProcessor::setPDFOptions(const PDFParams &value)
{
    PDFOptions = value;
}

void SignProcessor::setCryptoPROOptions(const CryptoPROParams &value)
{
    CryptoPROOptions = value;
}

void SignProcessor::runProcessing()
{
    log.addToLog("Запущен процесс подписи");
    CryptoPRO_CSP CryptoPRO; // создаем обработчик подписи
    CryptoPRO.csptest.setCryptoProDirectory(CryptoPROOptions.CryptoPRODirectore); // устанавливаем директорию

    for(auto &&file : filesList) // перебераем все файлы
    {
        QApplication::processEvents(); // прогружаем интерфейс
        log.addToLog("Обработка файла " + file.sourceFile);
        if(closing != nullptr)
        {
            if(*closing == true)
            {
                break;
            }
        }
        emit newFileStatus(file, files_status::in_process);
        QApplication::processEvents(); // прогружаем интерфейс
        bool movedToNextPage = false; // флаг перехода на новую страницу

        // проверяем, нет-ли в директории уже созданного такого файла
        if(file.signPDFFile != "")
        {
            int index = 1;  // индекс копии файла
            QString tempFilename = file.signPDFFile;  // временное название для поиска копий файла
            while (QFile::exists(tempFilename))
            {
                tempFilename = file.signPDFFile.remove(".pdf");
                index++;
                tempFilename.append("_" + QString::number(index) + ".pdf");
            }
            file.signPDFFile = tempFilename;
        }

        // обрабатывем файл
        #ifdef WIN32
        if(isWordFile(file.sourceFile)) // если у нас вордовский файл
        {
            // проверяем картинку
            QString imagedir = WordOptions.getImageDir();
            if(imagedir == "" || (imagedir != "" && !QFile::exists(imagedir)))
            {
                qDebug() << "Нельзя добавить картинку - файл не найден!" << imagedir;
                emit newFileStatus(file, files_status::error_no_open);
                continue;
            }

            // создаём временную купию документа
            QString tempFile; // место хранения временного файла
            tempFile = createFileCopy(file.sourceFile, WordOptions.getTempdir()); // создаем временный файл, который будет иметь такое же название как и исходный
            AutoDeleter tempFileDirContol(tempFile); // автоматическое удаление файла
            if(tempFile == "")
            {
                qDebug() << "Произошла ошибка создания временного файла!";
                emit newFileStatus(file, files_status::error_no_open);
                continue;
            }

            // стандартная вставка
            if(WordOptions.insertType == insert_standart) // если выбран классический вариант создания подписи
            {
                int status = -1;    // статус обработки файла
                standartAddImageToWordFile(file, tempFile, imagedir, WordOptions, status, movedToNextPage); // выполняем обычную вставку в ворд файл
                assert(status != -1, "standartAddImageToWordFile"); // если произошла критическая ошибка и статус файла == -1
                emit newFileStatus(file, status);  // отправляем сигнал обновления статуса файла
                check_file_status(status);  // проверяем статус файла и делаем continue, если необходимо
            }

            // вставка по координатам
            else if (WordOptions.insertType == insert_in_exported_pdf) // если выбрана вставка в PDF
            {
                // экспортируем в PDF
                WordEditor word; // создаем обработчик ворда
                word.openDocument(tempFile); // открываем документ
                int wordPageOrientation = word.getPageOrientation(); // получаем ориентацию страницы

                tempFile = WordOptions.getTempdir() + getFileNameInPDFFormat(QFileInfo(file.sourceFile).fileName()); // временный файл подписи
                AutoDeleter tempFileDirContol(tempFile); // автоматическое удаление файла
                if(!word.exportToPdf(tempFile)) // экспортируем файл в PDF
                {
                    qDebug() << "Не удалось экспортировать файл в PDF";
                    log.addToLog("Не удалось экспортировать файл в PDF");
                    emit newFileStatus(file, files_status::error_pdf_no_export);
                    continue;
                }
                word.closeDocument(); // закрываем

                // получаем ориентацию документа
                PDFCreator::orientation pdfPageOrientation; // ориентация страницы в PDF
                if(wordPageOrientation == WordEditor::pageOrientation::wdOrientLandscape)
                {
                    pdfPageOrientation = PDFCreator::Landscape; // ставим альбомную ориентацию
                }
                else
                {
                    pdfPageOrientation = PDFCreator::Portrait; // ставим портретную ориентацию
                }

                // вставляем подпись в PDF по координатам
                int status = -1;
                addImageToPdfFileInCoordinates(tempFile, file.signPDFFile, WordOptions, PDFOptions, pdfPageOrientation, MIREA_LOGO_HTML, status);   // добавляем картинку по координатам
                assert(status != -1, "addImageToPdfFileInCoordinates"); // если произошла критическая ошибка и статус файла == -1
                emit newFileStatus(file, status);  // отправляем сигнал обновления статуса файла
                check_file_status(status);  // проверяем статус файла и делаем continue, если необходимо
            }

            // вставка по тэгу
            else if(WordOptions.insertType == insert_by_tag_in_table) // если выбран вариант вставки по тэгу
            {
                int status = -1;    // статус обработки файла
                addImageToWordFileByTagInTable(file, tempFile, imagedir, WordOptions, status, movedToNextPage); // выполняем обычную вставку в ворд файл
                assert(status != -1, "standartAddImageToWordFile"); // если произошла критическая ошибка и статус файла == -1
                emit newFileStatus(file, status);  // отправляем сигнал обновления статуса файла
                check_file_status(status);  // проверяем статус файла и делаем continue, если необходимо
            }
        }
        else if (isExcelFile(file.sourceFile))  // обработчик Excel файлов
        {
            // создаём временный файл
            QString tempFile; // место хранения временного файла
            tempFile = createFileCopy(file.sourceFile, WordOptions.getTempdir()); // создаем временный файл, который будет иметь такое же название как и исходный
            AutoDeleter tempFileDirContol(tempFile); // автоматическое удаление файла
            if(tempFile == "")
            {
                qDebug() << "Произошла ошибка создания временного файла!";
                log.addToLog("Произошла ошибка создания временного файла!");
                emit newFileStatus(file, files_status::error_no_open);
                continue;
            }

            // экспортируем Excel в PDF
            ExcelEditor excel; // создаем обработчик ворда
            excel.openBook(tempFile); // открываем документ
            int excelPageOrientation = excel.getPageOrientation(); // получаем ориентацию страницы

            QString tempPdfFile = WordOptions.getTempdir() + getFileNameInPDFFormat(QFileInfo(file.sourceFile).fileName()); // создаем временный pdf файл
            AutoDeleter tempPdfFileDirContol(tempPdfFile); // автоматическое удаление файла
            excel.saveBook();
            if(!excel.exportToPdf(tempPdfFile)) // экспортируем файл в PDF
            {
                qDebug() << "Не удалось экспортировать файл в PDF";
                log.addToLog("Не удалось экспортировать файл в PDF");
                emit newFileStatus(file, files_status::error_pdf_no_export);
                continue;
            }
            excel.closeBook(); // закрываем

            // получаем ориентацию книги
            PDFCreator::orientation pdfPageOrientation; // ориентация страницы в PDF
            if(excelPageOrientation == ExcelEditor::orientation::xlLandscape)
            {
                pdfPageOrientation = PDFCreator::Landscape; // ставим альбомную ориентацию
            }
            else
            {
                pdfPageOrientation = PDFCreator::Portrait; // ставим портретную ориентацию
            }

            // стандартная вставка
            if(WordOptions.insertType == insert_standart)
            {
                // вставляем подпись в PDF по координатам
                int status = -1;
                addImageToPdfFileInEndOfFile(tempPdfFile, file.signPDFFile, WordOptions, PDFOptions, pdfPageOrientation, MIREA_LOGO_HTML, status);   // добавляем картинку по координатам
                assert(status != -1, "addImageToPdfFileInEndOfFile"); // если произошла критическая ошибка и статус файла == -1
                emit newFileStatus(file, status);  // отправляем сигнал обновления статуса файла
                check_file_status(status);  // проверяем статус файла и делаем continue, если необходимо
            }

            // вставка по координатам
            else if (WordOptions.insertType == insert_in_exported_pdf)
            {
                // вставляем подпись в PDF по координатам
                int status = -1;
                addImageToPdfFileInCoordinates(tempPdfFile, file.signPDFFile, WordOptions, PDFOptions, pdfPageOrientation, MIREA_LOGO_HTML, status);   // добавляем картинку по координатам
                assert(status != -1, "addImageToPdfFileInCoordinates"); // если произошла критическая ошибка и статус файла == -1
                emit newFileStatus(file, status);  // отправляем сигнал обновления статуса файла
                check_file_status(status);  // проверяем статус файла и делаем continue, если необходимо
            }

            // вставка по тэгу
            else
            {
                qDebug() << "Для данного типа файла невозможно выполнить действие " << file.sourceFile;
                log.addToLog("Для данного типа файла невозможно выполнить действие " + file.sourceFile);
                emit newFileStatus(file, files_status::no_supported);
                continue;
            }
        }
#endif
        else if (isPDFFile(file.sourceFile)) // обрабатываем PDF файл
        {
            // создаём временный файл
            QString tempPdfFile; // место хранения временного файла
            tempPdfFile = createFileCopy(file.sourceFile, WordOptions.getTempdir()); // создаем временный файл, который будет иметь такое же название как и исходный
            AutoDeleter tempFileDirContol(tempPdfFile); // автоматическое удаление файла
            if(tempPdfFile == "")
            {
                qDebug() << "Произошла ошибка создания временного файла!";
                log.addToLog("Произошла ошибка создания временного файла!");
                emit newFileStatus(file, files_status::error_no_open);
                continue;
            }

            // получаем информацию о файле
            PDFToPNGConverter pdfConverter;

            bool ok = false;
            auto info = pdfConverter.getLastLineInfo(file.sourceFile, &ok);
            if(!ok)
            {
                qDebug() << "Произошла ошибка получения информации о PDF файле!";
                log.addToLog("Произошла ошибка получения информации о PDF файле!");
                emit newFileStatus(file, files_status::error_no_open);
                continue;
            }

            // получаем ориентацию последней страницы
            PDFCreator::orientation pdfPageOrientation;
            if(info.height > info.width) // если высота больше ширины
            {
                pdfPageOrientation = PDFCreator::Portrait;
            }
            else
            {
                pdfPageOrientation = PDFCreator::Landscape;
            }

            // стандартная подпись
            if(WordOptions.insertType == insert_standart)
            {
                // вставляем подпись в PDF по координатам
                int status = -1;
                addImageToPdfFileInEndOfFile(tempPdfFile, file.signPDFFile, WordOptions, PDFOptions, pdfPageOrientation, MIREA_LOGO_HTML, status);   // добавляем картинку по координатам
                assert(status != -1, "addImageToPdfFileInEndOfFile"); // если произошла критическая ошибка и статус файла == -1
                emit newFileStatus(file, status);  // отправляем сигнал обновления статуса файла
                check_file_status(status);  // проверяем статус файла и делаем continue, если необходимо
            }
            // вставка по координатам
            else if (WordOptions.insertType == insert_in_exported_pdf)
            {                
                // вставляем подпись в PDF по координатам
                int status = -1;
                addImageToPdfFileInCoordinates(tempPdfFile, file.signPDFFile, WordOptions, PDFOptions, pdfPageOrientation, MIREA_LOGO_HTML, status);   // добавляем картинку по координатам
                assert(status != -1, "addImageToPdfFileInCoordinates"); // если произошла критическая ошибка и статус файла == -1
                emit newFileStatus(file, status);  // отправляем сигнал обновления статуса файла
                check_file_status(status);  // проверяем статус файла и делаем continue, если необходимо
            }
            // вставка по тэгу
            else
            {
                qDebug() << "Для данного типа файла невозможно выполнить действие " << file.sourceFile;
                log.addToLog("Для данного типа файла невозможно выполнить действие " + file.sourceFile);
                emit newFileStatus(file, files_status::no_supported);
                continue;
            }
        }
        // неподдерживаемое расширение файла
        else
        {
            qDebug() << "Не удалось определить расшиение файла: " << file.sourceFile;
            log.addToLog("Не удалось определить расшиение файла: " + file.sourceFile);
            emit newFileStatus(file, files_status::no_supported);
            continue;
        }

        // производим подписание файла
        if(isWordFile(file.sourceFile) && file.signWordFile != "" && WordOptions.insertType == insert_standart) // подпись ворд файла (docx doc rtf)
        {
            if(!CryptoPRO.csptest.createSign(file.signWordFile, CryptoPROOptions.sign))
            {
                qDebug() << "Не удалось подписать файл " << file.signWordFile;
                log.addToLog("Не удалось подписать файл " + file.signWordFile);
                emit newFileStatus(file, files_status::no_signed);
                continue;
            }
        }
        // всё, что в PDF
        if((isWordFile(file.sourceFile) && (file.signPDFFile != "")) || isExcelFile(file.sourceFile) || isPDFFile(file.sourceFile))
        {
            if(!CryptoPRO.csptest.createSign(file.signPDFFile, CryptoPROOptions.sign))
            {
                qDebug() << "Не удалось подписать файл " << file.signPDFFile;
                log.addToLog("Не удалось подписать файл " + file.signPDFFile);
                emit newFileStatus(file, files_status::no_signed);
                continue;
            }
        }
        file.isSigned = true; // ставим флаг, что файл подписан
        if(movedToNextPage)
        {
#ifdef DISPLAY_STATUS_ON_NEW_PAGE
            emit newFileStatus(file, files_status::error_new_page_added);
#else
            emit newFileStatus(file, files_status::no_errors);
#endif
        }
        emit newFileStatus(file, files_status::no_errors);
        QApplication::processEvents(); // прогружаем интерфейс

    }
    log.addToLog("Завершена обработка файлов");
    emit procesingFinished();
    QApplication::processEvents(); // прогружаем интерфейс
}

bool SignProcessor::isExtension(QString file, QStringList filesExtensions)
{
    for(auto &&extension : filesExtensions) // проходим по списку расширений
    {
        if(file.endsWith(extension)) // если файл оканчивается на это расширение
        {
            return true;
        }
    }
    return false;
}

bool SignProcessor::isWordFile(QString file)
{
    return isExtension(file, filesExtensionsWord);
}

bool SignProcessor::isExcelFile(QString file)
{
    return isExtension(file, filesExtensionsExcel);
}

bool SignProcessor::isPDFFile(QString file)
{
    return isExtension(file, filesExtensionsPDF);
}

QString SignProcessor::createFileCopy(QString sourceFile, QString dirForCopy, QString nextFileName)
{
    if(sourceFile == "" || dirForCopy == "")
    {
        qDebug() << "Отсутвуюет входные параметры! Нельзя скопировать файл.";
        return "";
    }
    if(!QFile::exists(sourceFile))
    {
        qDebug() << "Не найден исходный файл! Нельзя скопировать файл.";
        return "";
    }
    if(!dirForCopy.endsWith("/") && !dirForCopy.endsWith("\\")) // если в конце нет слеша
    {
        dirForCopy.append("/");
    }

    QDir copyngDir(dirForCopy);
    if(!copyngDir.exists()) // проверяем надичие директории
    {
        qDebug() << "Создаём директорию: " << dirForCopy;
        if(!copyngDir.mkdir(dirForCopy)) //  создаём директорию
        {
            qDebug() << "Не удалось создать директорию для копирования файла! Нельзя скопировать файл.";
            return "";
        }
    }
    if(nextFileName == "")
    {
        QString f_name = QFileInfo(sourceFile).fileName(); // получаем имя файла без пути
        nextFileName = dirForCopy + f_name; // директория + исходное название
    }
    if(!replaceOriginalFileByTemp(nextFileName, sourceFile)) // зачем писать функцию копирования, если можно модифицировать старую (профит)
    {
        qDebug() << "Не удалось скопеировать файл! Нельзя скопировать файл.";
        return "";
    }
    return nextFileName; // возврщаем путь к новому файлу
}

QString SignProcessor::getFileNameInPDFFormat(QString sourceFileName)
{
    // теоритически - это не очень рационально,
    // но практически, если нам нужно будет добавить новое расширение файла,
    // то это будет сделать очень удобно
    QList<QStringList> allFilesExtensions; // список всех возможных расширений файлов
    allFilesExtensions.append(filesExtensionsWord);
    allFilesExtensions.append(filesExtensionsExcel);

    for(auto &&list : allFilesExtensions) // проходим по всем спискам расширений
    {
        for(auto &&ext : list) // проходим по всем расширениям
        {
            if(sourceFileName.endsWith(ext)) // если совпадает
            {
                return sourceFileName.replace(ext, ".pdf"); // делаем замену и возвращаем
            }
        }
    }
    return sourceFileName;
}

bool SignProcessor::replaceOriginalFileByTemp(QString originalFile, QString tempFile)
{
    if(originalFile == "" || tempFile == "")
    {
        qDebug() << "Отсутвуют входные данные о файлах! Нельзя скопировать файл.";
        return false;
    }
    if(!QFile::exists(tempFile))
    {
        qDebug() << "Не найден временный файл! Нельзя скопировать файл.";
        return false;
    }

    QFile lastFile(tempFile); // берём исходный файл
    QFile nextFile(originalFile); // берём файл, в который копируем
    if(!lastFile.open(QIODevice::ReadOnly))
    {
        qDebug() << "Не удалось временный файл! Нельзя скопировать файл.";
        return false;
    }
    if(!nextFile.open(QIODevice::WriteOnly))
    {
        lastFile.close();
        qDebug() << "Не удалось открыть исходный файл для записи! Нельзя скопировать файл.";
        return false;
    }
    auto data = lastFile.readAll(); // считываем файл
    nextFile.write(data); // записываем файл
    lastFile.close();
    nextFile.close(); // закрываем файлы
    return true;
}

QString SignProcessor::getFileName(QString file)
{
    return QFileInfo(file).fileName();

}

QString SignProcessor::getDir(QString dir)
{
    if(!dir.endsWith("/") && !dir.endsWith("\\")) // если в конце нет слеша
    {
        dir.append("/");
    }
    return dir;
}

void SignProcessor::standartAddImageToWordFile(FileForSign &file, QString tempFile, QString imagedir, WordParams WordOptions, int &fileStatus, bool &movedToNextPage)
{
    WordEditor word; // создаем обработчик ворда

    assert(word.openDocument(tempFile), "openDocument"); // открываем документ
    int pagesCountBefore = word.getPagesCount(); // получаем количество страниц
    assert(word.insertText("\r"), "insertText"); // добавляем перенос строки (по умолчанию добавляется в конец документа

    if(!WordOptions.noInsertImage) // если нет запрета на вставку картинки
    {
        assert(word.moveSelectionToEnd(), "moveSelectionToEnd"); // перемещаем курсор в конец (потому что обработчик смарт-объектов указывает на начало документа)
        assert(word.addPicture(imagedir, 3.0), "addPicture"); // вставляем картинку
    }
    assert(word.setParagraphAlignment(WordEditor::paragrapfAlignment::center), "setParagraphAlignment"); // выравниваем по центру
    int pageCountAfter = word.getPagesCount(); // получаем количество страниц после вставки

    if(pageCountAfter > pagesCountBefore)
    {
        if(!WordOptions.ignoreMovingToNextList) // если нельзя игнорировать переход на новую страницу и переход произошёл
        {
            qDebug() << "Произошло увеличение количества страниц. Отменяем подпись.";
            log.addToLog("Произошло увеличение количества страниц. Отменяем подпись.");
//            emit newFileStatus(file, files_status::error_new_page_no_added);
//                        word.closeDocument(); // закрываем
//            continue;
            fileStatus = files_status::error_new_page_no_added;
            return;
        }
        movedToNextPage = true; // иначе ставим флаг, что произошёл переход
    }

//                assert(word.saveDocument(), "saveDocument"); // сохраняем
    if(WordOptions.exportToPDF)
    {
        if(!word.exportToPdf(file.signPDFFile)) // экспортируем файл в PDF
        {
            qDebug() << "Не удалось экспортировать файл в PDF";
            log.addToLog("Не удалось экспортировать файл в PDF");
//            emit newFileStatus(file, files_status::error_pdf_no_export);
//            continue;
            fileStatus = files_status::error_pdf_no_export;
            return;
        }
    }
    assert(word.saveDocument(), "saveDocument"); // сохраняем
    word.closeDocument(); // закрываем
    if(WordOptions.exportToWord)
    {
        if(!replaceOriginalFileByTemp(file.signWordFile, tempFile)) // копируем временный файл в необходимый
        {
            qDebug() << "Не удалось экспортировать файл Word";
            log.addToLog("Не удалось экспортировать файл Word");
//            emit newFileStatus(file, files_status::error_no_open);
//            continue;
            fileStatus = files_status::error_no_open;
            return;
        }
    }

    fileStatus = files_status::in_process;   // ставим статус, что всё хорошо
}

void SignProcessor::addImageToWordFileByTagInTable(FileForSign &file, QString tempFile, QString imagedir, WordParams WordOptions, int &fileStatus, bool &movedToNextPage)
{
    WordEditor word;
    assert(word.openDocument(tempFile), "openDocument"); // открываем документ
    int pagesCountBefore = word.getPagesCount(); // получаем количество страниц

    int tableCount = word.tables().count();
    if(tableCount <= 0)
    {
        qDebug() << "В файле отсутвуют таблицы " + file.sourceFile;
        log.addToLog("В файле отсутвуют таблицы " + file.sourceFile);
//        emit newFileStatus(file, files_status::error_no_tabels);
//        continue;
        fileStatus = files_status::error_no_tabels;
        return;
    }

    // проверка тэга картинки
    bool hasSignImageTag = false;
    if(WordOptions.signImageTag != "")
    {
        for (int i=1; i<=tableCount; i++)
        {
            WordEditor::WordTable table = word.table(i); // получаем таблицу

            int rowsCount = table.rowsCount();
            int colsCount = table.columnsCount();

            for (int row=1; row<=rowsCount; row++)
            {
                for (int col=1; col<=colsCount; col++)
                {
                    WordEditor::TableCell cell = table.cell(row, col); // получем ячейку
                    QString currentText = cell.text(); // получаем текст ячейки

                    if(currentText.contains(WordOptions.signImageTag)) // если ячейка содержит тэг
                    {
                        cell.clear(); // очищаем ячейку
                        cell.setImage(imagedir); // ставим картинку в ячейку
                        hasSignImageTag = true;
                    }
                }
            }
        }
    }

    // проверка тэга фио
    bool hasSignFioTag = true;
    bool needContinue = false;
    if(WordOptions.signFioTag != "")
    {
        for (int i=1; i<=tableCount; i++)
        {
            WordEditor::WordTable table = word.table(i); // получаем таблицу

            int rowsCount = table.rowsCount();
            int colsCount = table.columnsCount();

            for (int row=1; row<=rowsCount; row++)
            {
                for (int col=1; col<=colsCount; col++)
                {
                    WordEditor::TableCell cell = table.cell(row, col); // получем ячейку
                    QString currentText = cell.text(); // получаем текст ячейки

                    if(currentText.contains(WordOptions.signFioTag)) // если ячейка содержит тэг
                    {
                        QString surname = CryptoPROOptions.sign.surname; // получаем фамилию по сертификату
                        QString name;
                        QString patronymic;
                        QStringList nameList = CryptoPROOptions.sign.name_and_patronymic.split(" ", SPLITTER); // разбиваем имя и фотчество через пробел
                        QString text = "";  // текст ФИО
                        if(nameList.size() > 0)
                        {
                            if(nameList.size() != 2) // если не содержится имя + отчество
                            {
                                QString sertname = CryptoPROOptions.sign.name; // получаем название сертификата
                                nameList = sertname.split(" ", SPLITTER);
                                if(nameList.size() != 3) // если в названии не содержится Фамилия Имя Отчество
                                {
                                    qDebug() << "Некорректное значение имени и отчества " << nameList;
                                    log.addToLog("Некорректное значение имени и отчества ");
                                }
                                surname = nameList.at(0); // 0 - фамилия
                                name = nameList.at(1).at(0); // первая буква имени
                                patronymic = nameList.at(2).at(0); // первая буква отчества
                            }
                            name = nameList.at(0).at(0); // берем первый элемент списка и из него 1ю букву
                            patronymic = nameList.at(1).at(0); // берем второй элемент списка и из него 1ю букву
                            text = surname + " " + name + "." + patronymic + "."; // формируем строку фио
                        }

                        cell.setText(text); // ставим картинку в ячейку
                        hasSignFioTag = true;
                    }
                }
            }
        }
    }
    if(needContinue)
    {
//        continue;
        return;
    }

    if(!hasSignImageTag && !hasSignFioTag)
    {
        qDebug() << "Не найдены тэги в документе " + file.sourceFile;
        log.addToLog("Не найдены тэги в документе " + file.sourceFile);
//        emit newFileStatus(file, files_status::error_no_tags);
//        continue;
        fileStatus = files_status::error_no_tags;
        return;
    }

    int pageCountAfter = word.getPagesCount(); // получаем количество страниц после вставки

    if(pageCountAfter > pagesCountBefore)
    {
        if(!WordOptions.ignoreMovingToNextList) // если нельзя игнорировать переход на новую страницу и переход произошёл
        {
            qDebug() << "Произошло увеличение количества страниц. Отменяем подпись.";
            log.addToLog("Произошло увеличение количества страниц. Отменяем подпись.");
//            emit newFileStatus(file, files_status::error_new_page_no_added);
//                        word.closeDocument(); // закрываем
//            continue;
            fileStatus = files_status::error_new_page_no_added;
            return;

        }
        movedToNextPage = true; // иначе ставим флаг, что произошёл переход
    }

//                assert(word.saveDocument(), "saveDocument"); // сохраняем
    if(WordOptions.exportToPDF)
    {
        if(!word.exportToPdf(file.signPDFFile)) // экспортируем файл в PDF
        {
            qDebug() << "Не удалось экспортировать файл в PDF";
            log.addToLog("Не удалось экспортировать файл в PDF");
//            emit newFileStatus(file, files_status::error_pdf_no_export);
//            continue;
            fileStatus = files_status::error_pdf_no_export;
            return;
        }
    }
    assert(word.saveDocument(), "saveDocument"); // сохраняем
    word.closeDocument(); // закрываем
    if(WordOptions.exportToWord)
    {
        if(!replaceOriginalFileByTemp(file.signWordFile, tempFile)) // копируем временный файл в необходимый
        {
            qDebug() << "Не удалось экспортировать файл Word";
            log.addToLog("Не удалось экспортировать файл Word");
//            emit newFileStatus(file, files_status::error_no_open);
//            continue;
            fileStatus = files_status::error_no_open;
            return;
        }
    }
    fileStatus = files_status::in_process;
}

void SignProcessor::addImageToPdfFileInCoordinates(QString input_file, QString output_file, WordParams WordOptions, PDFParams PDFOptions, PDFCreator::orientation pdfPageOrientation, QString gerb_file, int &fileStatus)
{
    // создаем PDF из HTML
    PDFCreator pdf;
    QString simpleSignFile = WordOptions.getTempdir() + "simpleSign.pdf";
    AutoDeleter simpleSignFileDirContol(simpleSignFile); // автоматическое удаление файла
    if(!pdf.drawSign(simpleSignFile,
                                PDFOptions.pdf_preset.alignment,
                                PDFOptions.pdf_preset.paragraphOffset,
                                PDFOptions.htmlParams.lineSertificate,
                                PDFOptions.htmlParams.lineOwner,
                                PDFOptions.htmlParams.lineDate,
                                pdfPageOrientation)) // создаем PDF из HTML
    {
        qDebug() << "Не удалось создать PDF файл подписи!";
        log.addToLog("Не удалось создать PDF файл подписи!");
//        emit newFileStatus(file, files_status::error_pdf_no_export);
//        continue;
        fileStatus = files_status::error_pdf_no_export;
        return;
    }


    if(PDFOptions.drawLogo) // если нужно нарисовать герб
    {
        // отрисовываем герб в отдельный файл
        QString simpleGerbFile = WordOptions.getTempdir() + "simpleGerb.pdf";
        AutoDeleter simpleGerbFileDirContol(simpleGerbFile); // автоматическое удаление файла
        if(!pdf.drawImage(simpleGerbFile, gerb_file, PDFOptions.pdf_preset.alignment, PDFOptions.pdf_preset.paragraphOffset, pdfPageOrientation))
        {
            qDebug() << "Не удалось создать герб PDF файл подписи!";
            log.addToLog("Не удалось создать герб PDF файл подписи!");
//            emit newFileStatus(file, files_status::error_pdf_no_export);
//            continue;
            fileStatus = files_status::error_pdf_no_export;
            return;
        }

        // временный файл с подпиьсю + гербом
        QString tempSignFile = WordOptions.getTempdir() + "tempSignFile.pdf";
        AutoDeleter tempSignDirContol(simpleGerbFile); // автоматическое удаление файла

        // теперь с помощью QPDF объединяем файлы
        qpdf_cmd qpdf;
        qpdf.setQpdfPath(PDFOptions.qpdf_dir); // устанавливаем путь к QPDF
        if(!qpdf.overlay(simpleSignFile, simpleGerbFile, tempSignFile)) // объединяем подпись и герб
        {
            qDebug() << "Не удалось объединить файлы" << input_file << simpleSignFile;
            log.addToLog("Не удалось объединить файлы " + input_file + " " + simpleSignFile);
//            emit newFileStatus(file, files_status::error_pdf_no_export);
//            continue;
            fileStatus = files_status::error_pdf_no_export;
            return;
        }
        if(!qpdf.overlay(tempSignFile, input_file, output_file)) // объединяем файл с подписью и гербом с оригинальным файлом
        {
            qDebug() << "Не удалось объединить файлы" << input_file << simpleSignFile;
            log.addToLog("Не удалось объединить файлы " + input_file + " " + simpleSignFile);
//            emit newFileStatus(file, files_status::error_pdf_no_export);
//            continue;
            fileStatus = files_status::error_pdf_no_export;
            return;
        }
    }
    else // если не надо рисовать герб
    {
        // теперь с помощью QPDF объединяем файлы
        qpdf_cmd qpdf;
        qpdf.setQpdfPath(PDFOptions.qpdf_dir); // устанавливаем путь к QPDF
        qDebug() << "Файл для наложения: " << output_file;
        qDebug() << "Файл наложения: " << simpleSignFile;
        if(!qpdf.overlay(simpleSignFile, input_file, output_file)) // объединяем файл подписи и оригинальный файл
        {
            qDebug() << "Не удалось объединить файлы" << input_file << simpleSignFile;
            log.addToLog("Не удалось объединить файлы " + input_file + " " + simpleSignFile);
//            emit newFileStatus(file, files_status::error_pdf_no_export);
//            continue;
            fileStatus = files_status::error_pdf_no_export;
            return;
        }
    }
    fileStatus = files_status::in_process;
}

void SignProcessor::addImageToPdfFileInEndOfFile(QString input_file, QString output_file, WordParams WordOptions, PDFParams PDFOptions, PDFCreator::orientation pdfPageOrientation, QString gerb_file, int &fileStatus)
{
    PDFToPNGConverter pdfConverter;

    bool ok = false;
    auto info = pdfConverter.getLastLineInfo(input_file, &ok);
    if(!ok)
    {
        qDebug() << "Произошла ошибка получения информации о PDF файле!";
        log.addToLog("Произошла ошибка получения информации о PDF файле!");
//        emit newFileStatus(file, files_status::error_no_open);
//        continue;
        fileStatus = files_status::error_no_open;
        return;
    }

    qDebug() << "Данные об PDF: " << "Высота: " << info.height << "Ширина: " << info.width << "Номер строки (пикселя): " << info.lastLine << " Строка для вставки: " << info.offsetLine;

    // создаем PDF из HTML
    PDFCreator pdf;
    QString simpleSignFile = WordOptions.getTempdir() + "simpleSign.pdf";
    AutoDeleter simpleSignFileDirContol(simpleSignFile); // автоматическое удаление файла

    bool onNextPage = false;
    if(!pdf.drawSign(simpleSignFile,
                     QPoint(-1, info.lastLine),
                                PDFOptions.htmlParams.lineSertificate,
                                PDFOptions.htmlParams.lineOwner,
                                PDFOptions.htmlParams.lineDate,
                                pdfPageOrientation,
                     &onNextPage)) // создаем PDF из HTML

    {
        qDebug() << "Не удалось создать PDF файл подписи!";
        log.addToLog("Не удалось создать PDF файл подписи!");
//        emit newFileStatus(file, files_status::error_pdf_no_export);
//        continue;
        fileStatus = files_status::error_pdf_no_export;
        return;
    }

    QString tempSignFile = simpleSignFile; // временный файл подписи

    if(PDFOptions.drawLogo) // если нужно нарисовать герб
    {
        // отрисовываем герб в отдельный файл
        QString simpleGerbFile = WordOptions.getTempdir() + "simpleGerb.pdf";
        AutoDeleter simpleGerbFileDirContol(simpleGerbFile); // автоматическое удаление файла
        if(!pdf.drawImage(simpleGerbFile,
                          gerb_file,
                          QPoint(-1, info.lastLine),
                          pdfPageOrientation))
        {
            qDebug() << "Не удалось создать герб PDF файл подписи!";
            log.addToLog("Не удалось создать герб PDF файл подписи!");
//            emit newFileStatus(file, files_status::error_pdf_no_export);
//            continue;
            fileStatus = files_status::error_pdf_no_export;
            return;
        }

        // временный файл с подпиьсю + гербом
        tempSignFile = WordOptions.getTempdir() + "tempSignFile.pdf";
//                AutoDeleter tempSignDirContol(simpleGerbFile); // автоматическое удаление файла

        // теперь с помощью QPDF объединяем файлы
        qpdf_cmd qpdf;
        qpdf.setQpdfPath(PDFOptions.qpdf_dir); // устанавливаем путь к QPDF
        if(!qpdf.overlay(simpleSignFile, simpleGerbFile, tempSignFile)) // объединяем подпись и герб
        {
            qDebug() << "Не удалось объединить файлы" << input_file << simpleSignFile;
            log.addToLog("Не удалось объединить файлы " + input_file + " " + simpleSignFile);
//            emit newFileStatus(file, files_status::error_pdf_no_export);
//            continue;
            fileStatus = files_status::error_pdf_no_export;
            return;
        }
    }

    // теперь с помощью QPDF объединяем файлы
    AutoDeleter tempSignDirContol(tempSignFile); // автоматическое удаление файла
    qpdf_cmd qpdf;
    qpdf.setQpdfPath(PDFOptions.qpdf_dir); // устанавливаем путь к QPDF
    qDebug() << "Файл для наложения: " << output_file;
    qDebug() << "Файл наложения: " << tempSignFile;
    if(onNextPage) // если подпись переходит на последнюю страницу
    {
        if(WordOptions.ignoreMovingToNextList) // если можно переходить на новую страницу
        {
            if(!qpdf.merge(input_file, tempSignFile, output_file))
            {
                qDebug() << "Не удалось объединить (merge) файлы" << input_file << tempSignFile;
                log.addToLog("Не удалось объединить (merge) файлы " + input_file + " " + tempSignFile);
//                emit newFileStatus(file, files_status::error_pdf_no_export);
//                continue;
                fileStatus = files_status::error_pdf_no_export;
                return;
            }
        }
        else // если нельзя игнорировать переход
        {
            qDebug() << "Произошло увеличение количества страниц. Отменяем подпись.";
            log.addToLog("Произошло увеличение количества страниц. Отменяем подпись.");
//            emit newFileStatus(file, files_status::error_new_page_no_added);
//            continue;
            fileStatus = files_status::error_new_page_no_added;
            return;
        }
    }
    else // если рисуем на последней странице
    {
        if(!qpdf.overlay(tempSignFile, input_file, output_file)) // объединяем файл подписи и оригинальный файл
        {
            qDebug() << "Не удалось объединить (overlay) файлы" << input_file << tempSignFile;
            log.addToLog("Не удалось объединить (overlay) файлы " + input_file + " " + tempSignFile);
//            emit newFileStatus(file, files_status::error_pdf_no_export);
//            continue;
            fileStatus = files_status::error_pdf_no_export;
            return;
        }
    }
    fileStatus = files_status::in_process;
}

QString SignProcessor::WordParams::getTempdir() const
{
    return getDir(this->tempdir);
}

QString SignProcessor::WordParams::getOutputdir() const
{
    return getDir(this->outputdir);
}

QString SignProcessor::WordParams::getImageDir() const
{
    return this->imageDir;
}

SignProcessor::AutoDeleter::~AutoDeleter()
{
    if(QFile::exists(controlFile))
    {
        if(!QFile::remove(controlFile))
        {
            qDebug() << "Не удалось удалить файл " << controlFile;
        }
    }
}
