#include "exeleditor.h"

#define checkAxObject(pointer, object_name)  if(pointer==nullptr) { qDebug() << "Нет данных " + QString(object_name); throw QString("AxObject Error: ") + object_name;  return false; } else { connect(pointer, SIGNAL(exception(int,QString,QString,QString)), this, SLOT(exception(int,QString,QString,QString))); pointer->setObjectName(object_name); }
#define checkNullPointer(pointer, texterror) if(pointer == nullptr) { qDebug() << texterror << " is nullptr"; throw QString("NullPointer Error: ") + texterror;  return false; }

ExcelEditor::ExcelEditor(QObject *parent) : QObject(parent)
{
    Q_ASSERT(ExcelInit());
}

ExcelEditor::~ExcelEditor()
{
    ExcelQuit();
}

bool ExcelEditor::openBook(QString fileDir)
{
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
    return true;
}

bool ExcelEditor::saveBook()
{
    checkNullPointer(workbook, "workbook");
    workbook->dynamicCall("Save()");
    return true;
}

bool ExcelEditor::exportToPdf(QString outputFileName)
{
    checkNullPointer(ActiveSheet, "ActiveSheet");
    QString tempOutputFileName = "C:/temp.pdf";
    ActiveSheet->dynamicCall("ExportAsFixedFormat(int, QVariant, int, bool, bool, bool)",
                               xlTypePDF, tempOutputFileName, xlQualityStandard, IgnorePrintAreas, OpenAfterPublish);
    QFile::copy(tempOutputFileName, outputFileName);
    QFile::remove(tempOutputFileName);
    qDebug() << "Экспортировано в " << outputFileName;
    return true;
}

bool ExcelEditor::closeBook()
{
    checkNullPointer(workbook, "workbook");
    workbook->dynamicCall("Close()");
    return true;
}

bool ExcelEditor::updatePageSetup()
{
    checkNullPointer(ActiveSheet, "ActiveSheet");
    pageSetup = ActiveSheet->querySubObject("PageSetup");
    return true;
}

int ExcelEditor::getPageOrientation()
{
    checkNullPointer(pageSetup, "pageSetup");
    QVariant orientation = pageSetup->dynamicCall("Orientation()");
    return orientation.toInt(); // получаем значение ориентации
}

bool ExcelEditor::setPageOrientation(int PageOrientation)
{
    checkNullPointer(pageSetup, "pageSetup");
    if(PageOrientation != xlLandscape && PageOrientation != xlPortrait)
    {
        return false;
    }
    pageSetup->setProperty("Orientation", PageOrientation);
    return true;
}

QString **ExcelEditor::readFile(QString fileDiretory, int &rowsCount, int &colsCount, QProgressBar *progressBar)
{
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
}

void ExcelEditor::writeFile(QString fileDiretory, QString **text, int rows, int cols)
{
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
}

bool ExcelEditor::ExcelInit()
{
    excel = new QAxObject("Excel.Application", 0);
    checkAxObject(excel, "Excel.Application");
    excel->dynamicCall("SetDisplayAlerts(bool)", false);
    workbooks = excel->querySubObject("Workbooks");
    checkAxObject(workbooks, "workbooks");
    return true;
}

bool ExcelEditor::ExcelQuit()
{
    if (excel)
    {
        delete excel;
    }
    return true;
}

void ExcelEditor::exception(int code, QString source, QString desc, QString help)
{
    qDebug() << "EXCEPTION: " << "Sender: " + sender()->objectName() << "code: " << code << "Source: " + source << "Desc: " +  desc << "Help: " + help;
}
