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

    // работа со страницами
    int getPagesCount(); // получить количество страниц в документе
    bool setPageOrientation(pageOrientation orientation); // установить ориентация страницы
    int getPageOrientation(); // получить ориентация страницы

    // работа с картинками
    bool moveSelectionToEnd(); // перемещаем курсор в конец файла
    bool updateShapes(); // обновляет указатель на smart объекты
    bool addPicture(QString imagedir, const float size_santimetr = 3.0); // добавить картинку

    // перемещение по документу
    bool typeBackspace(); // вызов клавиши backspace

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
