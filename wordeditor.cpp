#include "wordeditor.h"

#define checkAxObject(pointer, object_name)  if(pointer==nullptr) { qDebug() << "Нет данных " + QString(object_name); throw QString("AxObject Error: ") + object_name;  return false; } else { QObject::connect(pointer, SIGNAL(exception(int,QString,QString,QString)), this, SLOT(exception(int,QString,QString,QString))); pointer->setObjectName(object_name); }
#define checkNullPointer(pointer, texterror) if(pointer == nullptr) { qDebug() << texterror << " is nullptr"; throw QString("NullPointer Error: ") + texterror;  return false; }

WordEditor::WordEditor(QObject *parent) : QObject(parent)
{
   Q_ASSERT(wordInit()); // инициализируем ворд, и если ошибюка, то qt на сообщит об этом
}

WordEditor::~WordEditor()
{
    if(!wordQuit())
    {
        qDebug() << "Не удалось закрыть Word!";
    }
}

bool WordEditor::openDocument(QString fileDir)
{
    checkNullPointer(word, "word");
    QString f_dir;
    QString f_name;

    if(opened) // если уже открыт документ
    {
        closeDocument(); // закрываем открытый
    }

    f_dir = QFileInfo(fileDir).absolutePath() + "/";
    f_name = QFileInfo(fileDir).fileName();

    word->dynamicCall("ChangeFileOpenDirectory(String)", f_dir); // устанавливаем рабочую директорию
    document = documents->querySubObject("Open(QVariant)", QVariant(f_name)); //добавляем свой документ в коллекцию
    checkAxObject(document, "document");
    qDebug() << "Открыли документ";
    word->setProperty("Visible", false);

    ActiveDocument = word->querySubObject("ActiveDocument()");
    checkAxObject(ActiveDocument, "ActiveDocument");
    Range = ActiveDocument->querySubObject("Range()");
    checkAxObject(Range, "Range");
    activeFile = fileDir; // сохраняем путь к файлу
    opened = true;
    return true;
}

bool WordEditor::insertText(QString text)
{
    checkNullPointer(word, "word");
    checkNullPointer(Range, "Range");
    Range->dynamicCall("InsertAfter(Text)", text); // вставляем перенос строки
    updateRange(); // обновляем информацию
    return true;
}

bool WordEditor::moveSelectionToEnd()
{
    if(!updateSelection()) return false;
    int end = getRangeEnd();
    if(!selectionSetRange(0, end)) return false;
    if(!selectionCollapse(WordEditor::collapse::wdCollapseEnd)) return false;
    return true;
}

bool WordEditor::updateRange()
{
    checkNullPointer(word, "word");
    checkNullPointer(Range, "Range");
    Range = ActiveDocument->querySubObject("Range()");
    return true;
}

int WordEditor::getRangeStart()
{
    checkNullPointer(Range, "Range");
    if(Range == nullptr)
    {
        return -1;
    }
    return Range->property( "Start" ).toInt(); // получаем указатель на конец текущей рабочей области;
}

int WordEditor::getRangeEnd()
{
    checkNullPointer(Range, "Range");
    if(Range == nullptr)
    {
        return -1;
    }
    return Range->property( "End" ).toInt(); // получаем указатель на конец текущей рабочей области;
}

bool WordEditor::updateSelection()
{
    checkNullPointer(word, "word");
    selection = word->querySubObject("Selection") ; // текущая точка вставки, рабочая область документа
    checkAxObject(selection, "selection");
    return true;
}

bool WordEditor::selectionSetRange(int start, int end)
{
    checkNullPointer(selection, "selection");
    checkNullPointer(word, "word");
    updateSelection();
    checkNullPointer(Range, "Range");
    selection->dynamicCall( "SetRange(int,int)", start, end );
    return true;
}

bool WordEditor::selectionCollapse(int postion)
{
    checkNullPointer(word, "word");
    if(selection == nullptr)
    {
        if(!updateSelection())
        {
            qDebug() << "не удалось получить selection";
            throw "не удалось получить selection";
            return false;
        }
    }
    selection->dynamicCall( "Collapse(int)", postion ); // после установки новой рабочей области перемещаемся в её начало
    return true;
}

bool WordEditor::deleteSelection()
{
    checkNullPointer(selection, "selection");
    updateSelection();
    selection->dynamicCall("Delete()");
    return true;
}

bool WordEditor::updateShapes(QAxObject *selection)
{
    if(selection == nullptr)
    {
        selection = this->selection;
    }
    checkNullPointer(selection, "selection");
    if(selection == nullptr)
    {
        if(!updateSelection())
        {
            qDebug() << "не удалось получить selection";
            throw "не удалось получить selection";
            return false;
        }
    }
    shapes = selection->querySubObject( "InlineShapes" ); // получаем указатель на объект, отвечающий за Smart-Объекты (картинки, видео и т.п.)
    return true;
}

bool WordEditor::addPicture(QString imagedir, const float size_santimetr, QAxObject *selection)
{
    checkNullPointer(word, "word");
    if(selection == nullptr)
    {
        selection = this->selection;
    }
    if(!updateShapes(selection))
    {
        qDebug() << "не удалось получить updateShapes";
        throw "не удалось получить updateShapes";
        return false;
    }

    checkNullPointer(shapes, "shapes");
    QAxObject* inlineShape = shapes->querySubObject("AddPicture(const QString&,bool,bool,QVariant)", imagedir, false, true );   // метод добавления картинки
    checkAxObject(inlineShape,"inlineShape");
    checkNullPointer(inlineShape, "inlineShape");

//    const float size_santimetr = 3.0; // желаемая высота картинки в сантиметрах
    const float size_koefficient = 0.05081632653; // постоянный коэффициент для преобразования сантиметров в размер (высчитан опытным путём)
    const float size_for_word = size_santimetr/size_koefficient; // считаем коээфицент для масштабирвоания в ворде
    const int output_size = static_cast<int>(size_for_word); // переводим в целое число, потому что ворду нужны целые числа

    const int height = output_size; // задаём коэффицент масштабирвоания по высоте
    const int width = output_size; // задаём коэффицент масштабирования по ширине
    inlineShape->dynamicCall( "ScaleHeight", height );
    inlineShape->dynamicCall( "ScaleWidth", width );
    return true;
}

WordEditor::WordTables WordEditor::tables()
{
    if(!word || !ActiveDocument)
    {
        qDebug() << "Ошибка getTables";
        return WordTables(nullptr);
    }
    return WordTables(ActiveDocument->querySubObject("Tables()"));
}

WordEditor::WordTable WordEditor::table(int tableIndex)
{
    if(!word || !ActiveDocument)
    {
        qDebug() << "Ошибка getTables";
        return WordTable(nullptr, -1);
    }
    return WordTable(ActiveDocument->querySubObject("Tables(int)", tableIndex), tableIndex);
}

WordEditor::WordTable::WordTable(QAxObject *table, int index)
{
    this->table = table;
    this->index = index;
}

bool WordEditor::WordTable::tableToText()
{
    checkNullPointer(table, "table");
    table->dynamicCall("ConvertToText()");
    return true;
}

//int WordEditor::WordTable::getTablesCount()
//{
//    checkNullPointer(word, "word");
//    checkNullPointer(ActiveDocument, "ActiveDocument");
//    return ActiveDocument->querySubObject("Tables()")->dynamicCall("Count()").toInt();
//}

int WordEditor::WordTable::columnsCount()
{
    checkNullPointer(table, "table");
    return table->querySubObject("Columns()")->dynamicCall("Count()").toInt();
}

int WordEditor::WordTable::rowsCount()
{
    checkNullPointer(table, "table");
    return table->querySubObject("Rows()")->dynamicCall("Count()").toInt();
}

int WordEditor::WordTable::taleIndex() const
{
    return index;
}

WordEditor::TableCell WordEditor::WordTable::cell(int row, int col)
{
    if(table == nullptr)
    {
        qDebug() << "Не указана таблица. Получить ячейку невозможно.";
        return TableCell(nullptr, -1, -1);
    }
    int rows = rowsCount();
    int cols = columnsCount();
    if(row < 1 && row > rows)
    {
        qDebug() << "Некорректный номер строки" << row << rows;
        return TableCell(nullptr, -1, -1);
    }
    if(col < 1 && col > cols)
    {
        qDebug() << "Некорректный номер столбца" << col << cols;
        return TableCell(nullptr, -1, -1);
    }
    return TableCell(table->querySubObject("Cell(int,int)", row, col), row, col); // возвращаем ячейку
}

bool WordEditor::setParagraphAlignment(int alignment)
{
    checkNullPointer(word, "word");
    if(selection == nullptr)
    {
        updateSelection();
    }
    selection->querySubObject( "ParagraphFormat" )->setProperty( "Alignment", alignment ); // выравниваем по центру
    return true;
}

bool WordEditor::exportToPdf(QString outputFileName)
{
    if((!outputFileName.endsWith(".pdf")) && (outputFileName != ""))
    {
        qDebug() << "Выходной файл должен быть *.pdf";
        return false;
    }
    checkNullPointer(word, "word");
    checkNullPointer(ActiveDocument, "ActiveDocument");
    QString exportFilename = outputFileName; // имя экспортируемого файла

    QString f_dir = QFileInfo(activeFile).absolutePath() + "/"; // формируем путь к файлу
    QString f_name = QFileInfo(activeFile).baseName() + ".pdf"; // формируем название файла

    if(exportFilename == "")
    {
        exportFilename = f_dir + f_name; // собираем название файла
    }
    else if(!exportFilename.contains("/") || !exportFilename.contains("\\")) // если на вход подано только название файла
    {
        exportFilename = outputFileName; // директория файла + новое название
    }
    qDebug() << "Сохряняем файл в PDF: " << exportFilename;
    QString change_dir = QFileInfo(exportFilename).absolutePath() + "/";

    word->dynamicCall("ChangeFileOpenDirectory(String)", change_dir); // устанавливаем рабочую директорию
    ActiveDocument->dynamicCall("ExportAsFixedFormat(String, WdExportFormat, Boolean, WdExportOptimizeFor, WdExportRange, Long, Long, WdExportItem, Boolean, Boolean, WdExportCreateBookmarks, Boolean, Boolean, Boolean)",
                                                          QFileInfo(exportFilename).baseName() + ".pdf", "wdExportFormatPDF", false);
    word->dynamicCall("ChangeFileOpenDirectory(String)", change_dir); // устанавливаем рабочую директорию
    return true;
}

int WordEditor::getPagesCount()
{
    checkNullPointer(word, "word");
    checkNullPointer(ActiveDocument, "ActiveDocument");
    QAxObject* content = ActiveDocument->querySubObject("Content"); //
    checkAxObject(content, "content");
    content = ActiveDocument->querySubObject("Content");
    int numberOfPages = content->dynamicCall("Information(wdNumberOfPagesInDocument)").toInt(); // получаем количество страниц после обработки
    return numberOfPages;
}

bool WordEditor::setPageOrientation(pageOrientation orientation)
{
    checkNullPointer(word, "word");
    checkNullPointer(ActiveDocument, "ActiveDocument");
    QAxObject *PageSetup = selection->querySubObject("PageSetup"); // получаем параметры страницы
    checkAxObject(PageSetup, "PageSetup");

    PageSetup->setProperty("Orientation", orientation); // ставим портретную ориентацию
    return true;
}

int WordEditor::getPageOrientation()
{
    checkNullPointer(word, "word");
    checkNullPointer(ActiveDocument, "ActiveDocument");
    updateSelection();
    checkNullPointer(selection, "selection");
    QAxObject *PageSetup = selection->querySubObject("PageSetup"); // получаем параметры страницы
    checkAxObject(PageSetup, "PageSetup");

    return PageSetup->dynamicCall("Orientation()").toInt(); // получаем ориентацию страницы
}

bool WordEditor::printDocument(QString printer)
{
    checkNullPointer(word, "word");
    checkNullPointer(ActiveDocument, "ActiveDocument");
    if(printer != "")
    {
        word->setProperty("ActivePrinter", printer); // если задан конкретный принтер
    }

    word->dynamicCall("PrintOut()"); // отправляем на печать
    return true;
}

bool WordEditor::saveDocument()
{
    checkNullPointer(word, "word");
    checkNullPointer(document, "document");
    document->dynamicCall("Save()");
    return true;
}

bool WordEditor::closeDocument()
{
    checkNullPointer(word, "word");
    checkNullPointer(document, "document");
    document->dynamicCall("Close()");
    opened = false;
    return true;
}

bool WordEditor::wordInit()
{
    word = new QAxObject("Word.Application", this);
    checkAxObject(word, "word");
    if(word->control() == "")
    {
        qDebug() << "Ошибка инициализации Word!";
        return false;
    }
    documents = word->querySubObject("Documents"); //получаем коллекцию документов
    checkAxObject(documents, "documents");
    return true;
}

bool WordEditor::wordQuit()
{
    checkNullPointer(word, "word");
    if(opened) // если уже открыт документ
    {
        closeDocument(); // закрываем открытый
    }
    word->dynamicCall("Quit()");
    return true;
}

QAxObject *WordEditor::getWord() const
{
    return word;
}

void WordEditor::exception(int code, QString source, QString desc, QString help)
{
    qDebug() << "EXCEPTION: " << "Sender: " + sender()->objectName() << "code: " << code << "Source: " + source << "Desc: " +  desc << "Help: " + help;
}

WordEditor::WordTables::WordTables(QAxObject *tables)
{
    this->tables = tables;
}

int WordEditor::WordTables::count()
{
    checkNullPointer(this->tables, "tables");
    return tables->dynamicCall("Count()").toInt();
}

WordEditor::TableCell::TableCell(QAxObject *cell, int row, int col)
{
    this->cell = cell;
    this->tableRow = row;
    this->tableColumn = col;
}

QString WordEditor::TableCell::text()
{
    if(cell == nullptr)
    {
        qDebug() <<  "Не удалось получить текст ячейки " + QString::number(tableRow) + " " + QString::number(tableColumn) + ". Не задана ячейка.";
        return "";
    }
    return cell->querySubObject("Range()")->dynamicCall("Text()").toString();
}

bool WordEditor::TableCell::setText(QString text)
{
    if(!clear())
        return false;
    if(!insertText(text))
        return false;
    return true;
}

bool WordEditor::TableCell::insertText(QString text)
{
    if(cell == nullptr)
    {
        qDebug() << "Не удалось установить текст ячейки " + QString::number(tableRow) + " " + QString::number(tableColumn) + ". Не задана ячейка.";
        return false;
    }
    cell->querySubObject("Range()")->dynamicCall("InsertAfter(String)", text);
    return true;
}

int WordEditor::TableCell::row()
{
    return tableRow;
}

int WordEditor::TableCell::column()
{
    return tableColumn;
}

bool WordEditor::TableCell::setImage(QString imageDir)
{
    QAxObject *range = cell->querySubObject("Range()");
    if(range == nullptr)
    {
        qDebug() << "Ошибка range. Картинка не вставлена.";
        return false;
    }
//    checkNullPointer(word, "word");
//    QAxObject* inlineShape = word->querySubObject("Selection()")->querySubObject("InlineShapes()");
    QAxObject* inlineShape = range->querySubObject("InlineShapes()");
    const float size_santimetr = 3.0;
    checkNullPointer(inlineShape, "inlineShape");
    QAxObject *image = inlineShape->querySubObject("AddPicture(const QString&,bool,bool,QVariant)", imageDir, false, true );   // метод добавления картинки
//    checkAxObject(inlineShape,"inlineShape");

//    const float size_santimetr = 3.0; // желаемая высота картинки в сантиметрах
    const float size_koefficient = 0.05081632653; // постоянный коэффициент для преобразования сантиметров в размер (высчитан опытным путём)
    const float size_for_word = size_santimetr/size_koefficient; // считаем коээфицент для масштабирвоания в ворде
    const int output_size = static_cast<int>(size_for_word); // переводим в целое число, потому что ворду нужны целые числа

    const int height = output_size; // задаём коэффицент масштабирвоания по высоте
    const int width = output_size; // задаём коэффицент масштабирования по ширине
    image->dynamicCall( "ScaleHeight", height );
    image->dynamicCall( "ScaleWidth", width );
    return true;
}

bool WordEditor::TableCell::clear()
{
    if(cell == nullptr)
    {
        qDebug() << "Не удалось очистить данные ячейки " + QString::number(tableRow) + " " + QString::number(tableColumn) + ". Не задана ячейка.";
        return false;
    }
    cell->querySubObject("Range()")->dynamicCall("Delete()");
    return true;
}
