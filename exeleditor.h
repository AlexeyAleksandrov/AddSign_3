#ifndef ExcelEditor_H
#define ExcelEditor_H

#include <QObject>
#include <QFile>
#include <QTableWidget>
#include <QAxObject>
#include <QFileDialog>
#include <QMessageBox>
#include <QProgressBar>
#include <QApplication>
#include <QDebug>
#include "logclass.h"

// параметры Excel
#define xlTypePDF 0
#define xlQualityStandard 0
#define IncludeDocProperties true
#define IgnorePrintAreas true
#define OpenAfterPublish false

//#define xlPortrait 1    // вертикальная ориентация
//#define xlLandscape 2   // альбомная ориентация


class ExcelEditor : public QObject
{
    Q_OBJECT
public:
    explicit ExcelEditor(QObject *parent = nullptr);
    ~ExcelEditor();

    enum orientation
    {
        xlPortrait = 1,
        xlLandscape = 2
    };

    bool openBook(QString fileDir); // открыть документ
    bool saveBook(); // просто сохранить документ
    bool exportToPdf(QString outputFileName = QString()); // экспортировать в PDF
    bool closeBook(); // закрыть документ
    bool updatePageSetup(); // обновить данные о странице
    int getPageOrientation(); // получить данные об ориентации страницы
    bool setPageOrientation(int PageOrientation);


    static QString **readFile(QString fileDiretory, int &rowsCount, int &colsCount, QProgressBar *progressBar = nullptr);
    static void writeFile(QString fileDiretory, QString **text, int rows, int cols);

signals:


private:
    bool ExcelInit(); // инициализация ворда
    bool ExcelQuit(); // завершение ворда
    QAxObject* excel = nullptr;
    QAxObject* workbooks = nullptr;
    QAxObject* workbook = nullptr;
    QAxObject* sheets = nullptr;
    QAxObject* sheet = nullptr;
    QAxObject *ActiveSheet = nullptr;
    QAxObject *pageSetup = nullptr;

    logClass log;

private slots:
    void exception(int code, QString source, QString desc, QString help); // слот получения ошибок

};

#endif // ExcelEditor_H
