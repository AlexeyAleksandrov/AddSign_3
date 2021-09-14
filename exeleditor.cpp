#include "exeleditor.h"

#ifdef WIN32
#define checkAxObject(pointer, object_name)  if(pointer==nullptr) { qDebug() << "Нет данных " + QString(object_name); throw QString("AxObject Error: ") + object_name;  return false; } else { connect(pointer, SIGNAL(exception(int,QString,QString,QString)), this, SLOT(exception(int,QString,QString,QString))); pointer->setObjectName(object_name); }
#define checkNullPointer(pointer, texterror) if(pointer == nullptr) { qDebug() << texterror << " is nullptr"; log.addToLog(texterror + QString(" is nullptr")); throw QString("NullPointer Error: ") + texterror;  return false; }

ExcelEditor::ExcelEditor(QObject *parent) : QObject(parent)
{
    if(!(ExcelInit()))
    {
        qDebug() << "Excel no init!";
    }
}

ExcelEditor::~ExcelEditor()
{
    ExcelQuit();
}

bool ExcelEditor::openBook(QString fileDir)
{
#ifdef WIN32
    checkNullPointer(excel, "excel");
    checkNullPointer(workbooks, "workbooks");
//    excel->dynamicCall("ChDir(String)", QFileInfo(fileDir).absolutePath());
//    excel->dynamicCall("ChangeFileOpenDirectory(String)", QFileInfo(fileDir).absolutePath()); // устанавливаем рабочую директорию
    workbook = workbooks->querySubObject("Open(const QString&)", fileDir);
    checkAxObject(workbook, "workbook");
    sheets = workbook->querySubObject("Worksheets");
    checkAxObject(sheets, "sheets");
    sheet = sheets->querySubObject("Item(int)", 1);
    checkAxObject(sheet, "sheet");
    ActiveSheet = excel->querySubObject("ActiveSheet");
    checkAxObject(ActiveSheet, "ActiveSheet");
    pageSetup = ActiveSheet->querySubObject("PageSetup");
    checkAxObject(pageSetup, "pageSetup");
    qDebug() << "Открыли файл " << fileDir;
#endif
    return true;
}

bool ExcelEditor::saveBook()
{
#ifdef WIN32
    checkNullPointer(workbook, "workbook");
    workbook->dynamicCall("Save()");
#endif
    return true;
}

bool ExcelEditor::exportToPdf(QString outputFileName)
{
#ifdef WIN32
    checkNullPointer(ActiveSheet, "ActiveSheet");
    QString t_dir = "C:/tempAddSign";
    QDir tempdir(t_dir);
    if(!tempdir.exists())
    {
        if(!tempdir.mkdir(t_dir))
        {
            qDebug() << "Не удалось создать временную папку!";
            return false;
        }
    }
    QString tempOutputFileName =  t_dir + "/temp.pdf";
    ActiveSheet->dynamicCall("ExportAsFixedFormat(int, QVariant, int, bool, bool, bool)",
                               xlTypePDF, tempOutputFileName, xlQualityStandard, IgnorePrintAreas, OpenAfterPublish);
    QFile::copy(tempOutputFileName, outputFileName);
    if(QFile::exists(outputFileName))
    {
        QFile::remove(tempOutputFileName);
        tempdir.rmdir(t_dir);
    }
    else
    {
        qDebug() << "Не удалось экспортировать в PDF " << outputFileName;
        tempdir.rmdir(t_dir);
        return false;
    }
    qDebug() << "Экспортировано в " << outputFileName;
#endif
    return true;
}

bool ExcelEditor::closeBook()
{
#ifdef WIN32
    checkNullPointer(workbook, "workbook");
    workbook->dynamicCall("Close()");
#endif
    return true;
}

bool ExcelEditor::updatePageSetup()
{
#ifdef WIN32
    checkNullPointer(ActiveSheet, "ActiveSheet");
    pageSetup = ActiveSheet->querySubObject("PageSetup");
#endif
    return true;
}

int ExcelEditor::getPageOrientation()
{
#ifdef WIN32
    checkNullPointer(pageSetup, "pageSetup");
    QVariant orientation = pageSetup->dynamicCall("Orientation()");
    return orientation.toInt(); // получаем значение ориентации
#endif
}

bool ExcelEditor::setPageOrientation(int PageOrientation)
{
#ifdef WIN32
    checkNullPointer(pageSetup, "pageSetup");
    if(PageOrientation != xlLandscape && PageOrientation != xlPortrait)
    {
        return false;
    }
    pageSetup->setProperty("Orientation", PageOrientation);
#endif
    return true;
}

QString **ExcelEditor::readFile(QString fileDiretory, int &rowsCount, int &colsCount, QProgressBar *progressBar)
{
    #ifdef WIN32
    QAxObject* excel = new QAxObject("Excel.Application", 0);
    QAxObject* workbooks = excel->querySubObject("Workbooks");
    QAxObject* workbook = workbooks->querySubObject("Open(const QString&)", fileDiretory);
    QAxObject* sheets = workbook->querySubObject("Worksheets");
    QAxObject* sheet = sheets->querySubObject("Item(int)", 1);

    QAxObject* usedRange = sheet->querySubObject("UsedRange");
    QAxObject* rows = usedRange->querySubObject("Rows");
    QAxObject* columns = usedRange->querySubObject("Columns");
    const int countRows = rows->property("Count").toInt();
    const int countCols = columns->property("Count").toInt();

    rowsCount = countRows;
    colsCount = countCols;

    int max = rowsCount * colsCount;

    if(progressBar)
    {

        progressBar->setMinimum(0);
        progressBar->setMaximum(max);
        progressBar->setValue(0);
    }

    QString **text = new QString *[countRows]; // выделяем память под строки
    for(int i=0; i<countRows; i++)
    {
        text[i] = new QString[countCols]; // выделяем память под столбцы
    }

    for ( int row = 0; row < countRows; row++ )
    {
        for ( int column = 0; column < countCols; column++ )
        {
            QAxObject* cell = sheet->querySubObject("Cells(int,int)", row + 1, column + 1);
            QVariant value = cell->property("Value");
            text[row][column] = value.toString();
            if(progressBar)
            {
                int val = (row * countCols) + column;
                progressBar->setValue(val);
                QApplication::processEvents();
            }
        }
    }
    if(progressBar)
    {
        progressBar->setValue(max);
    }

    workbook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");
    delete  excel;
    excel = NULL;

    return text;
#endif
}

void ExcelEditor::writeFile(QString fileDiretory, QString **text, int rows, int cols)
{
    #ifdef WIN32
    QAxObject* excel = new QAxObject("Excel.Application", 0);
    QAxObject* workbooks = excel->querySubObject("Workbooks");
    QAxObject* workbook = workbooks->querySubObject("Open(const QString&)", fileDiretory);
    QAxObject* sheets = workbook->querySubObject("Worksheets");
    QAxObject* sheet = sheets->querySubObject("Item(int)", 1);

    for (int i=0; i<rows; i++)
    {
        for (int j=0; j<cols; j++)
        {
            QAxObject *cell = sheet->querySubObject("Cells(int, int)",i+1, j+1);
            cell->dynamicCall("SetValue(const QVariant&", text[i][j]);
        }
    }

    workbook->dynamicCall("Save()");
    workbook->dynamicCall("Close()");
    excel->dynamicCall("Quit()");
    delete  excel;
    excel = NULL;
#endif
}

bool ExcelEditor::ExcelInit()
{
#ifdef WIN32
    excel = new QAxObject("Excel.Application", 0);
    checkAxObject(excel, "Excel.Application");
    excel->dynamicCall("SetDisplayAlerts(bool)", false);
    workbooks = excel->querySubObject("Workbooks");
    checkAxObject(workbooks, "workbooks");
#endif
    return true;
}

bool ExcelEditor::ExcelQuit()
{
    #ifdef WIN32
    if (excel)
    {
        delete excel;
    }
#endif
    return true;
}

void ExcelEditor::exception(int code, QString source, QString desc, QString help)
{
    qDebug() << "EXCEPTION: " << "Sender: " + sender()->objectName() << "code: " << code << "Source: " + source << "Desc: " +  desc << "Help: " + help;
}

#endif
