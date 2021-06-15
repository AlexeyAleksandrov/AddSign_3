#include "pdfcreator.h"

#include "QFile"
#include "QTextStream"
#include <QDebug>
#include <QPainter>
#include <QPdfWriter>
#include <QPrinter>
#include <QTextDocument>

#define TAB "&nbsp;"

PDFCreator::PDFCreator(QObject *parent) : QObject(parent)
{}

bool PDFCreator::createSignPDFbyHTML(QString fileName, int image_pos_x, int image_pos_y, QString mireaLogoFileName, QString line_sertifacate, QString line_owner, QString line_validTime, PDFCreator::orientation orientation, bool drawMireaLogo)
{
    QFile pdfFile(fileName);
    if(!pdfFile.open(QIODevice::WriteOnly)) // если файл нельзя открыть для записи
    {
        return false;
    }
    QPageLayout::Orientation pageOrientation = (orientation == 0) ? QPageLayout::Portrait : QPageLayout::Landscape; // получаем нужную ориентацияю страницы
    if(pageOrientation != QPageLayout::Portrait && pageOrientation != QPageLayout::Landscape) // если укащана неверная ориентация страницы
    {
        return false;
    }
    pdfFile.close();
    QPdfWriter pdfWriter(fileName);
    QPainter painter;
    pdfWriter.setResolution(150);
    QPageSize pageSize(QPageSize::A4);
    QMarginsF margins(10, 10, 10, 10);
    QPageLayout pageLayout(pageSize, pageOrientation, margins, QPageLayout::Millimeter);
    QRectF fullrect = pageSize.rectPixels(pdfWriter.resolution()); // получаем полный прямоугольник
    pdfWriter.setPageLayout(pageLayout);

    painter.begin(&pdfWriter);

    QPen pen;
    pen.setColor(Qt::black);
    pen.setWidth(3);
    painter.setPen(pen);

    // прямоугольник для листа
    int weight = fullrect.width()-100;
    int height = fullrect.height()-100;
    if(pageOrientation == QPageLayout::Landscape) // если ориентация горизонтальная, то высоту и ширину меняем местами
    {
        std::swap(weight, height); // меняем местами
    }
//    painter.drawRect(0, 0, weight, height);
    qDebug() << "Высота: " << height << "Ширина: " << weight;

    // прямоугольник для подписи
    const double signRectWeight = 352.38;
    const double signRectHeight = 150.39;
    double signPosX = 0.0;
    double signPosY = 0.0;
    const int rowsCount = 30;

    // рассчёт позиции по X
    switch (image_pos_x)
    {
        case 0:
            signPosX = 0.0;
            break;
        case 2:
            signPosX = weight - signRectWeight;
            break;
        default:
            signPosX = (weight - signRectWeight)/2.0;
            break;
    }

    // рассчёт позиции по Y
    switch (image_pos_y)
    {
        case 0:
            signPosY = height - signRectHeight;
            break;
        case rowsCount:
            signPosY = 0.0;
            break;
        default:
            signPosY = ((height - signRectHeight)/rowsCount)*(rowsCount - image_pos_y);
            break;
    }


    //    QRectF bodyRect(image_pos_x, image_pos_y, signRectWeight, signRectHeight);
    QRectF bodyRect(signPosX, signPosY, signRectWeight, signRectHeight);

    int radius = 10;
    painter.drawRoundedRect(bodyRect, radius, radius);

    if(drawMireaLogo)
    {
        if(!QFile(mireaLogoFileName).exists()) // если файл лого не найден
        {
            return false;
        }
        QImage mirea_logo(mireaLogoFileName);
        painter.drawImage(bodyRect.topLeft().x()+2, bodyRect.topLeft().y()+5, mirea_logo);
    }

    QTextDocument td;
    auto coord = bodyRect.topLeft();
    coord.setY(coord.y()+7);
    painter.translate(coord);
    QString html;

    QString styleBig = QString::number(5);
    QString stylewSmall = QString::number(3);

    html.append("<font size=" + styleBig +"><b>");
    html.append(tab(4) + "ДОКУМЕНТ ПОДПИСАН");
    html.append("<br>");
    html.append(tab(3) + "ЭЛЕКТРОННОЙ ПОДПИСЬЮ</b>");
    html.append("<br>");
    html.append("<br></font><font size=" + stylewSmall +">");
    html.append(getLine(TAB, 1) + line_sertifacate);

    html.append("</font><font size=" + styleBig +"><br></font><font size=" +  +">");
    html.append(getLine(TAB, 1) + line_owner);

    html.append("</font><font size=" + styleBig +"><br></font><font size=" + stylewSmall +">");
    html.append(getLine(TAB, 1) + line_validTime);

    html.append("</font><font size=" + styleBig +"><br></font><font size=" + stylewSmall +">");
    html.append("</font>");
    td.setHtml(html);
    td.drawContents(&painter);

    painter.end();
    return true;
}

QString PDFCreator::getLine(QString symvol, int lenght)
{
    QString str;
    for(int i=0; i<lenght; i++)
    {
        str.append(symvol);
    }
    return str;
}

QString PDFCreator::tab(int count)
{
    return getLine(TAB, count*4);
}
