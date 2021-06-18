#include "signprocessor.h"

#include <QDir>

//#define MIREA_LOGO_HTML ":/img/MIREA_logo_resize.png"
#define MIREA_LOGO_HTML PDFOptions.image_dir

#define DISPLAY_STATUS_ON_NEW_PAGE
#define assert(var, texterror) if(var == false) { qDebug() << "Assert: " << texterror; log.addToLog(texterror); return; }

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
            if(WordOptions.exportToPDF) // если нужно экспортировать в PDF
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
            if(WordOptions.exportToPDF) // если нужно экспортировать в PDF
            {
                file.signPDFFile = getFileNameInPDFFormat(WordOptions.getOutputdir() + getFileName(file.sourceFile)); // тоже самое, но меняем расширение файла
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

//    PDFOptions.htmlParams.lineSertificate = "Сертификат: " + CryptoPROOptions.sign.serial; // создаём строку сертификата
//    PDFOptions.htmlParams.lineOwner = "Владелец: " + CryptoPROOptions.sign.subname; // ЗАМЕНИТЬ!!!!!
//    PDFOptions.htmlParams.lineDate = "Действителен с 09.09.2021 до 10.10.2021";

//    WordEditor word; // создаем обработчик ворда

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
        if(isWordFile(file.sourceFile)) // если у нас вордовский файл
        {
            WordEditor word; // создаем обработчик ворда
            QString imagedir = WordOptions.getImageDir();
            if(imagedir == "" || (imagedir != "" && !QFile::exists(imagedir)))
            {
                qDebug() << "Нельзя добавить картинку - файл не найден!" << imagedir;
                emit newFileStatus(file, files_status::error_no_open);
                continue;
            }
            QString tempFile; // место хранения временного файла
            tempFile = createFileCopy(file.sourceFile, WordOptions.getTempdir()); // создаем временный файл, который будет иметь такое же название как и исходный
            AutoDeleter tempFileDirContol(tempFile); // автоматическое удаление файла
            if(tempFile == "")
            {
                qDebug() << "Произошла ошибка создания временного файла!";
                emit newFileStatus(file, files_status::error_no_open);
                continue;
            }
            if(WordOptions.insertType == insert_standart) // если выбран классический вариант создания подписи
            {
//                WordEditor word; // создаем обработчик ворда

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
                        emit newFileStatus(file, files_status::error_new_page_no_added);
//                        word.closeDocument(); // закрываем
                        continue;
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
                        emit newFileStatus(file, files_status::error_pdf_no_export);
                        continue;
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
                        emit newFileStatus(file, files_status::error_no_open);
                        continue;
                    }
                }
            }
            else if (WordOptions.insertType == insert_in_exported_pdf) // если выбрана вставка в PDF
            {
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

                PDFCreator pdf;
                PDFCreator::orientation pdfPageOrientation; // ориентация страницы в PDF
                if(wordPageOrientation == WordEditor::pageOrientation::wdOrientLandscape)
                {
                    pdfPageOrientation = PDFCreator::Landscape; // ставим альбомную ориентацию
                }
                else
                {
                    pdfPageOrientation = PDFCreator::Portrait; // ставим портретную ориентацию
                }

                // создаем PDF из HTML
                QString simpleSignFile = WordOptions.getTempdir() + "simpleSign.pdf";
                AutoDeleter simpleSignFileDirContol(simpleSignFile); // автоматическое удаление файла
                if(!pdf.createSignPDFbyHTML(simpleSignFile,
                                            PDFOptions.pdf_preset.alignment,
                                            PDFOptions.pdf_preset.paragraphOffset, MIREA_LOGO_HTML,
                                            PDFOptions.htmlParams.lineSertificate,
                                            PDFOptions.htmlParams.lineOwner,
                                            PDFOptions.htmlParams.lineDate,
                                            pdfPageOrientation,
                                            PDFOptions.drawLogo)) // создаем PDF из HTML
                {
                    qDebug() << "Не удалось создать PDF файл подписи!";
                    log.addToLog("Не удалось создать PDF файл подписи!");
                    emit newFileStatus(file, files_status::error_pdf_no_export);
                    continue;
                }

                // теперь с помощью QPDF объединяем файлы
                qpdf_cmd qpdf;
                qpdf.setQpdfPath(PDFOptions.qpdf_dir); // устанавливаем путь к QPDF
                if(qpdf.overlay(simpleSignFile, tempFile, file.signPDFFile)) // объединяем файлы и готово
                {
                    qDebug() << "Не удалось объединить файлы" << tempFile << simpleSignFile;
                    log.addToLog("Не удалось объединить файлы " + tempFile + " " + simpleSignFile);
                    emit newFileStatus(file, files_status::error_pdf_no_export);
                    continue;
                }
            }
            else if(WordOptions.insertType == insert_by_tag_in_table) // если выбран вариант вставки по тэгу
            {
                assert(word.openDocument(tempFile), "openDocument"); // открываем документ
                int pagesCountBefore = word.getPagesCount(); // получаем количество страниц

                int tableCount = word.tables().count();
                if(tableCount <= 0)
                {
                    qDebug() << "В файле отсутвуют таблицы " + file.sourceFile;
                    log.addToLog("В файле отсутвуют таблицы " + file.sourceFile);
                    emit newFileStatus(file, files_status::error_no_tabels);
                    continue;
                }
                if(WordOptions.signTag == "")
                {
                    qDebug() << "Задан пустой тэг для поиска " + file.sourceFile;
                    log.addToLog("Задан пустой тэг для поиска " + file.sourceFile);
                    emit newFileStatus(file, files_status::error_no_tabels);
                    continue;
                }
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

                            if(currentText.contains(WordOptions.signTag)) // если ячейка содержит тэг
                            {
                                cell.clear(); // очищаем ячейку
                                cell.setImage(WordOptions.getImageDir()); // ставим картинку в ячейку
                            }
                        }
                    }
                }

                int pageCountAfter = word.getPagesCount(); // получаем количество страниц после вставки

                if(pageCountAfter > pagesCountBefore)
                {
                    if(!WordOptions.ignoreMovingToNextList) // если нельзя игнорировать переход на новую страницу и переход произошёл
                    {
                        qDebug() << "Произошло увеличение количества страниц. Отменяем подпись.";
                        log.addToLog("Произошло увеличение количества страниц. Отменяем подпись.");
                        emit newFileStatus(file, files_status::error_new_page_no_added);
//                        word.closeDocument(); // закрываем
                        continue;
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
                        emit newFileStatus(file, files_status::error_pdf_no_export);
                        continue;
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
                        emit newFileStatus(file, files_status::error_no_open);
                        continue;
                    }
                }
            }
        }
        else if (isExcelFile(file.sourceFile))  // обработчик Excel файлов
        {
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

            PDFCreator pdf;
            QString simpleSignFile = WordOptions.getTempdir() + "simplesign.pdf"; // временный файл подписи
            AutoDeleter simpleSignFileDirContol(simpleSignFile); // автоматическое удаление файла
            PDFCreator::orientation pdfPageOrientation; // ориентация страницы в PDF
            if(excelPageOrientation == ExcelEditor::orientation::xlLandscape)
            {
                pdfPageOrientation = PDFCreator::Landscape; // ставим альбомную ориентацию
            }
            else
            {
                pdfPageOrientation = PDFCreator::Portrait; // ставим портретную ориентацию
            }

            // создаем PDF из HTML
            if(!pdf.createSignPDFbyHTML(simpleSignFile,
                                        PDFOptions.pdf_preset.alignment,
                                        PDFOptions.pdf_preset.paragraphOffset, MIREA_LOGO_HTML,
                                        PDFOptions.htmlParams.lineSertificate,
                                        PDFOptions.htmlParams.lineOwner,
                                        PDFOptions.htmlParams.lineDate,
                                        pdfPageOrientation,
                                        PDFOptions.drawLogo)) // создаем PDF из HTML
            {
                qDebug() << "Не удалось создать PDF файл подписи!";
                log.addToLog("Не удалось создать PDF файл подписи!");
                emit newFileStatus(file, files_status::error_pdf_no_export);
                continue;
            }

            // теперь с помощью QPDF объединяем файлы
            qpdf_cmd qpdf;
            qpdf.setQpdfPath(PDFOptions.qpdf_dir); // устанавливаем путь к QPDF
            qDebug() << "Файл для наложения: " << file.signPDFFile;
            qDebug() << "Файл наложения: " << simpleSignFile;
            if(!qpdf.overlay(simpleSignFile, tempPdfFile, file.signPDFFile)) // объединяем файлы и готово
            {
                qDebug() << "Не удалось объединить файлы" << tempPdfFile << simpleSignFile;
                log.addToLog("Не удалось объединить файлы " + tempPdfFile + " " + simpleSignFile);
                emit newFileStatus(file, files_status::error_pdf_no_export);
                continue;
            }
        }
        else if (isPDFFile(file.sourceFile)) // обрабатываем PDF файл
        {
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

            // создаем PDF из HTML
            PDFCreator pdf;
            QString simpleSignFile = WordOptions.getTempdir() + "simpleSign.pdf";
            AutoDeleter simpleSignFileDirContol(simpleSignFile); // автоматическое удаление файла
            if(!pdf.createSignPDFbyHTML(simpleSignFile,
                                        PDFOptions.pdf_preset.alignment,
                                        PDFOptions.pdf_preset.paragraphOffset, MIREA_LOGO_HTML,
                                        PDFOptions.htmlParams.lineSertificate,
                                        PDFOptions.htmlParams.lineOwner,
                                        PDFOptions.htmlParams.lineDate,
                                        PDFCreator::Portrait,
                                        PDFOptions.drawLogo)) // создаем PDF из HTML
            {
                qDebug() << "Не удалось создать PDF файл подписи!";
                log.addToLog("Не удалось создать PDF файл подписи!");
                emit newFileStatus(file, files_status::error_pdf_no_export);
                continue;
            }

            // теперь с помощью QPDF объединяем файлы
            qpdf_cmd qpdf;
            qpdf.setQpdfPath(PDFOptions.qpdf_dir); // устанавливаем путь к QPDF
            if(!qpdf.overlay(simpleSignFile, tempFile, file.signPDFFile)) // объединяем файлы и готово
            {
                qDebug() << "Не удалось объединить файлы" << tempFile << simpleSignFile;
                log.addToLog("Не удалось объединить файлы " + tempFile + " " + simpleSignFile);
                emit newFileStatus(file, files_status::error_pdf_no_export);
                continue;
            }
        }
        else
        {
            qDebug() << "Не удалось определить расшиение файла: " << file.sourceFile;
            log.addToLog("Не удалось определить расшиение файла: " + file.sourceFile);
            emit newFileStatus(file, files_status::error_file_signature_failed);
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
