#ifndef WORDEDITOR_H
#define WORDEDITOR_H

#include <QObject>
#include <QAxObject>
#include <QDebug>
#include <QFileInfo>


class WordEditor : public QObject
{
    Q_OBJECT

public:
    enum collapse
    {
        wdCollapseEnd = 0,
        wdCollapseStart = 1
    };
    enum paragrapfAlignment
    {
        left = 0,
        center = 1,
        rignt = 2
    };
    enum pageOrientation
    {
        wdOrientPortrait = 0,
        wdOrientLandscape = 1
    };

    struct WordTables
    {
    public:
        WordTables(QAxObject *tables);
        int count(); // возвращает количество таблиц в документе
    private:
        QAxObject *tables = nullptr;

    };

    struct TableCell
    {
        TableCell(QAxObject *cell, int row, int col);
        QString text();
        bool setText(QString text);
        bool insertText(QString text);
        int row();
        int column();
        bool setImage(QString imageDir);
        bool clear();

        private:
        QAxObject *cell = nullptr;
        int tableRow = -1;
        int tableColumn = -1;
    };

    struct WordTable
    {
    public:
        WordTable(QAxObject *table, int index);
        bool tableToText(); // перевести таблицу в текст (внутри документа)
        int columnsCount(); // получить количество столбцов в таблице
        int rowsCount(); // возвращает количество строк
        int taleIndex() const;
        TableCell cell(int row, int col); // возвращает ячейку таблицы

        private:
        QAxObject *table = nullptr;
        int index = -1;
    };

public:
    explicit WordEditor(QObject *parent = nullptr);
    ~WordEditor();

    // работа с документом
    bool openDocument(QString fileDir); // открыть документ
    bool saveDocument(); // просто сохранить документ
    bool exportToPdf(QString outputFileName = QString()); // экспортировать в PDF
    bool closeDocument(); // закрыть документ
    bool printDocument(QString printer = ""); // распечатать документ

    // работа с текстом
    bool insertText(QString text); // добавить текст в текущую позицию
    bool setParagraphAlignment(int alignment); // установить выравнивание

    // рабочая область
    bool updateRange(); // обновить диапазон рабочей области
    int getRangeStart(); // получить начало выделенной области
    int getRangeEnd(); // получить конец выделенной области

    // выделенная область
    bool updateSelection(); // обновляет выбранную область
    bool selectionSetRange(int start, int end); // установить область выделения
    bool selectionCollapse(int postion); // переместить курсор в начало или колнец документа
    bool deleteSelection(); // удалить выбранную убласть

    // работа со страницами
    int getPagesCount(); // получить количество страниц в документе
    bool setPageOrientation(pageOrientation orientation); // установить ориентация страницы
    int getPageOrientation(); // получить ориентация страницы

    // работа с картинками
    bool moveSelectionToEnd(); // перемещаем курсор в конец файла
    bool updateShapes(QAxObject *selection = nullptr); // обновляет указатель на smart объекты
    bool addPicture(QString imagedir, const float size_santimetr = 3.0, QAxObject *selection = nullptr); // добавить картинку

    // перемещение по документу
    bool typeBackspace(); // вызов клавиши backspace

    // таблицы
    WordTables tables(); // возвращает объект tables
    WordTable table(int tableIndex); // возвращает объект tables

//    WordTable tables = WordTable(nullptr);

    QAxObject *getWord() const;

    signals:

    private slots:
    void exception(int code, QString source, QString desc, QString help); // слот получения ошибок

private:
    bool wordInit(); // инициализация ворда
    bool wordQuit(); // завершение ворда
    QAxObject *word = nullptr; // объект ворда
    QAxObject *documents = nullptr; //получаем коллекцию документов
    QAxObject *document = nullptr; // открытый документ
    QAxObject* ActiveDocument = nullptr; // активный докуменТ ,который мы сейчас редактируем
    QAxObject* Range = nullptr; // рабочая область
    QAxObject* selection = nullptr; // выделенная область
    QAxObject* shapes = nullptr; // объект работы со смарт-объектами

    QString activeFile; // текущий открытый файл
    bool opened = false;



};

#endif // WORDEDITOR_H
