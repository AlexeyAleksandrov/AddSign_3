#ifndef PDFCREATOR_H
#define PDFCREATOR_H

#include <QObject>
#include "libs/logclass.h"
#include "QFile"
#include "QTextStream"
#include <QDebug>
#include <QPainter>
#include <QPdfWriter>
#include <QPrinter>
#include <QTextDocument>
#include "qpdf_cmd.h"

class PDFCreator : public QObject
{
    Q_OBJECT
public:
    explicit PDFCreator( QObject *parent = nullptr);

    enum orientation
    {
        Portrait = 0,
        Landscape = 1
    };


    bool drawSign(QString fileName, int image_pos_x, int image_pos_y, QString line_sertifacate, QString line_owner, QString line_validTime, orientation orientation);   // для вставки по координатам (точные координаты)
    bool drawSign(QString fileName, QPoint pixel, QString line_sertifacate, QString line_owner, QString line_validTime, orientation orientation, bool *onNextPage = nullptr); // рисует подпись по центру и координате Y
    bool drawImage(QString fileName, QString imageDir, int image_pos_x, int image_pos_y, orientation orientation);
    bool drawImage(QString fileName, QString imageDir, QPoint pixel, orientation orientation);


    void drawHtml(QPainter &painter, QString &html); // нарисовать HTML на странице
    QString getHtmlBySign(QString line_sertifacate, QString line_owner, QString line_validTime); // создать html по данным подписи

signals:

private:
    QString getLine(QString symvol, int lenght);
    QString tab(int count);
//    QPoint getSignPos(int fullHeight, int fullWidth, QPageLayout::Orientation pageOrientation, int rowsCount, int columnsCount, int offsetX, int offsetY);

    logClass log;

    struct AutoDeleter
    {
        AutoDeleter(QString controlFile)
        {
            this->controlFile = controlFile;
        }
        ~AutoDeleter();

    private:
        QString controlFile;
    };
};

#endif // PDFCREATOR_H
