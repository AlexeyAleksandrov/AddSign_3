#include "pdfcreator.h"

#define TAB "&nbsp;"

PDFCreator::PDFCreator(QObject *parent) : QObject(parent)
{}

bool PDFCreator::drawSign(QString fileName, int image_pos_x, int image_pos_y, QString line_sertifacate, QString line_owner, QString line_validTime, PDFCreator::orientation orientation)
{
    log.addToLog("Создаём пустой файл подписи " + fileName);
    QFile pdfFile(fileName);
    if(!pdfFile.open(QIODevice::WriteOnly)) // если файл нельзя открыть для записи
    {
        log.addToLog("Файл нельзя открыть для записи" + fileName);
        return false;
    }
    QPageLayout::Orientation pageOrientation = (orientation == 0) ? QPageLayout::Portrait : QPageLayout::Landscape; // получаем нужную ориентацияю страницы
    if(pageOrientation != QPageLayout::Portrait && pageOrientation != QPageLayout::Landscape) // если указана неверная ориентация страницы
    {
        log.addToLog("Указана неверная ориентация страницы");
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

    auto coord = bodyRect.topLeft();
    coord.setY(coord.y()+7);
    painter.translate(coord);

    QString html = getHtmlBySign(line_sertifacate, line_owner, line_validTime); // создаем html с данными о подписи
    drawHtml(painter, html);

    painter.end();
    log.addToLog("Файл сформирован " + fileName);
    return true;
}

bool PDFCreator::drawSign(QString fileName, QPoint pixel, QString line_sertifacate, QString line_owner, QString line_validTime, orientation orientation, bool *onNextPage)
{
    if(onNextPage != nullptr) *onNextPage = false;
    log.addToLog("Создаём пустой файл подписи " + fileName);
    QFile pdfFile(fileName);
    if(!pdfFile.open(QIODevice::WriteOnly)) // если файл нельзя открыть для записи
    {
        log.addToLog("Файл нельзя открыть для записи" + fileName);
        return false;
    }
    QPageLayout::Orientation pageOrientation = (orientation == 0) ? QPageLayout::Portrait : QPageLayout::Landscape; // получаем нужную ориентацияю страницы
    if(pageOrientation != QPageLayout::Portrait && pageOrientation != QPageLayout::Landscape) // если указана неверная ориентация страницы
    {
        log.addToLog("Указана неверная ориентация страницы");
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
    double signPosX = pixel.x();
    double signPosY = pixel.y();

    if(signPosX == -1) // специальное значение для центра
    {
        signPosX = (weight - signRectWeight)/2.0; // считаем центр
    }

    qDebug() << "Сравнение: " << signPosY << (height - signRectHeight);
    if(signPosY > (height - signRectHeight)) // если координата Y больше, чем последняя возможная строка
    {
        signPosY = 0.0;
        if(onNextPage != nullptr) *onNextPage = true;
        qDebug() << "Обнаружен переход на новую страницу!";
    }

//    // рассчёт позиции по X
//    switch (pixel.x())
//    {
//        case 0:
//            signPosX = 0.0;
//            break;
//        case 2:
//            signPosX = weight - signRectWeight;
//            break;
//        default:
//            signPosX = (weight - signRectWeight)/2.0;
//            break;
//    }

//    // рассчёт позиции по Y
//    signPosY = pixel.y();


    QRectF bodyRect(signPosX, signPosY, signRectWeight, signRectHeight);

    int radius = 10;
    painter.drawRoundedRect(bodyRect, radius, radius);

    auto coord = bodyRect.topLeft();
    coord.setY(coord.y()+7);
    painter.translate(coord);

    QString html = getHtmlBySign(line_sertifacate, line_owner, line_validTime); // создаем html с данными о подписи
    drawHtml(painter, html);

    painter.end();
    log.addToLog("Файл сформирован " + fileName);
    return true;
}

bool PDFCreator::drawImage(QString fileName, QString imageDir, int image_pos_x, int image_pos_y, orientation orientation)
{
    log.addToLog("Создаём пустой файл подписи " + fileName);
    QFile pdfFile(fileName);
    if(!pdfFile.open(QIODevice::WriteOnly)) // если файл нельзя открыть для записи
    {
        log.addToLog("Файл нельзя открыть для записи" + fileName);
        return false;
    }
    QPageLayout::Orientation pageOrientation = (orientation == 0) ? QPageLayout::Portrait : QPageLayout::Landscape; // получаем нужную ориентацияю страницы
    if(pageOrientation != QPageLayout::Portrait && pageOrientation != QPageLayout::Landscape) // если указана неверная ориентация страницы
    {
        log.addToLog("Указана неверная ориентация страницы");
        return false;
    }
    pdfFile.close();
    QPdfWriter pdfWriter(fileName);
    QPainter painter;
    pdfWriter.setResolution(2200);
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
    const double scaleFactor = 2200.00 / 150.00;
    int weight = fullrect.width()-100 * scaleFactor;
    int height = fullrect.height()-100 * scaleFactor;
    if(pageOrientation == QPageLayout::Landscape) // если ориентация горизонтальная, то высоту и ширину меняем местами
    {
        std::swap(weight, height); // меняем местами
    }
//    painter.drawRect(0, 0, weight, height);
    qDebug() << "Высота: " << height << "Ширина: " << weight;

    // прямоугольник для подписи
    const double signRectWeight = 352.38 * scaleFactor;
    const double signRectHeight = 150.39 * scaleFactor;
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


    if(!QFile(imageDir).exists()) // если файл лого не найден
    {
        log.addToLog("Не найден файл лого" + imageDir);
        return false;
    }
    QImage mirea_logo(imageDir);
    painter.drawImage(bodyRect.topLeft().x()+2, bodyRect.topLeft().y()+5, mirea_logo);

    painter.end();
    log.addToLog("Файл сформирован " + fileName);
    return true;
}

bool PDFCreator::drawImage(QString fileName, QString imageDir, QPoint pixel, orientation orientation)
{
    log.addToLog("Создаём пустой файл подписи " + fileName);
    QFile pdfFile(fileName);
    if(!pdfFile.open(QIODevice::WriteOnly)) // если файл нельзя открыть для записи
    {
        log.addToLog("Файл нельзя открыть для записи" + fileName);
        return false;
    }
    QPageLayout::Orientation pageOrientation = (orientation == 0) ? QPageLayout::Portrait : QPageLayout::Landscape; // получаем нужную ориентацияю страницы
    if(pageOrientation != QPageLayout::Portrait && pageOrientation != QPageLayout::Landscape) // если указана неверная ориентация страницы
    {
        log.addToLog("Указана неверная ориентация страницы");
        return false;
    }
    pdfFile.close();
    QPdfWriter pdfWriter(fileName);
    QPainter painter;
    pdfWriter.setResolution(2200);
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
    const double scaleFactor = 2200.00 / 150.00;
    int weight = fullrect.width()-100 * scaleFactor;
    int height = fullrect.height()-100 * scaleFactor;
    if(pageOrientation == QPageLayout::Landscape) // если ориентация горизонтальная, то высоту и ширину меняем местами
    {
        std::swap(weight, height); // меняем местами
    }
//    painter.drawRect(0, 0, weight, height);
    qDebug() << "Высота: " << height << "Ширина: " << weight;

    // прямоугольник для подписи
    const double signRectWeight = 352.38 * scaleFactor;
    const double signRectHeight = 150.39 * scaleFactor;
    double signPosX = pixel.x() * scaleFactor;
    double signPosY = pixel.y() * scaleFactor;

    if(pixel.x() == -1)
    {
        signPosX = (weight - signRectWeight)/2.0;
    }

    if(signPosY > (height - signRectHeight)) // если координата Y больше, чем последняя возможная строка
    {
        signPosY = 0.0;
    }

//    const int rowsCount = 30;

//    // рассчёт позиции по X
//    switch (image_pos_x)
//    {
//        case 0:
//            signPosX = 0.0;
//            break;
//        case 2:
//            signPosX = weight - signRectWeight;
//            break;
//        default:
//            signPosX = (weight - signRectWeight)/2.0;
//            break;
//    }

//    // рассчёт позиции по Y
//    switch (image_pos_y)
//    {
//        case 0:
//            signPosY = height - signRectHeight;
//            break;
//        case rowsCount:
//            signPosY = 0.0;
//            break;
//        default:
//            signPosY = ((height - signRectHeight)/rowsCount)*(rowsCount - image_pos_y);
//            break;
//    }


    //    QRectF bodyRect(image_pos_x, image_pos_y, signRectWeight, signRectHeight);
    QRectF bodyRect(signPosX, signPosY, signRectWeight, signRectHeight);


    if(!QFile(imageDir).exists()) // если файл лого не найден
    {
        log.addToLog("Не найден файл лого" + imageDir);
        return false;
    }
    QImage mirea_logo(imageDir);
    painter.drawImage(bodyRect.topLeft().x()+(2 * scaleFactor), bodyRect.topLeft().y()+(5 * scaleFactor), mirea_logo);

    painter.end();
    log.addToLog("Файл сформирован " + fileName);
    return true;
}

void PDFCreator::drawHtml(QPainter &painter, QString &html)
{
    QTextDocument td;
    td.setHtml(html);
    td.drawContents(&painter);
}

QString PDFCreator::getHtmlBySign(QString line_sertifacate, QString line_owner, QString line_validTime)
{
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

    return html;
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
