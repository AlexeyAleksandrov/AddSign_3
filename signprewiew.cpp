#include "signprewiew.h"

#define drawText(line, text) pntr.drawText(QRect(signLeftIndent, signUpIndent + textInterval * line, backgroundWidth - signRightIndent, backgroundHeight - signDownIndent), text);
#define getYPos(line) signUpIndent + textInterval * line
#define getStartImageYpos(line) getYPos(line) + ((getYPos(line+1) - getYPos(line))/2)


signPrewiew::signPrewiew(QWidget *parent) : QWidget(parent)
{
    //    const double scale = (double)parent->height() / (double)this->height() /*+ 2.5 - 2.5*/;
    //    //    const double scale  = 1.0;
    //    #ifdef DEBUGGING
    //    qDebug() << "scale = " << scale << parent->width() / this->width();
    //    qDebug() << "Исходная высота: " << this->height() << "Общая высота: " << parent->height();
    //#endif
    //    const int imageWight = this->height() * scale/sqrt2; // ширина
    //    const int imageHeight = this->height() * scale; // высота


    //    const int imageWight = this->height() /sqrt2; // ширина
    //    const int imageHeight = this->height() ; // высота
    //    img = QImage(imageWight, imageHeight, QImage::Format_A2BGR30_Premultiplied);
    //    #ifdef DEBUGGING
    //    qDebug() << "Ширина. высота: " << imageWight << imageHeight;
    //#endif
    //    rectImage = QImage(":/images/image_gerb_384_160.jpg");
    //    drawSmallRect(0,0);

    rescale();
}

void signPrewiew::drawSmallRect(int xMode, int yMode)
{
    smallImagePosXMode = xMode;
    smallImagePosYMode = yMode;
    img.fill(0); // Очищаем картинку

    // параметры листа А4 в миллиетрах
    // =================================
    // размеры листа 210 * 297
    const int pageHorizontalSize = 210; // 21 см
    const int pageVerticalSize = 297; // 29.7 см

    // горзинтальные отсупы в миллиметрах
    const int pageLeftIndent = 30; // 3 см
    const int pageRightIndent = 15; // 1.5 см

    // вертикальные отступы в миллиметрах
    const int pageUpIndent = 20; // 2 см
    const int pageDownIndent = 20; // 2 см

    //====================================
    // параметры координат подписи
    const int linesCount = 30; // количество строк на странице (если интервал 1.0)
    const int columnsCount = 7; // количество вертикальных позиций на странице

    //====================================
    // параметры отсовываемого виджета в пикселях
    const int backgroundWidth = (double)img.width()/1.2f; // ширина виджета
    const int backgroundHeight = img.height(); // высота виджета

    // считаем смещение картинки подписи на виджете
    const int signLeftIndent = ((((double)pageLeftIndent)/((double)pageHorizontalSize)) * (double)backgroundWidth); // отсуп по горизонтали от левого края (оси координат)
    const int signRightIndent = ((((double)pageRightIndent)/((double)pageHorizontalSize)) * (double)backgroundWidth); // отсуп по горизонтали от левого края (оси координат)
    const int signDownIndent = ((((double)pageDownIndent)/((double)pageVerticalSize)) * (double)backgroundHeight); // отсуп по верстикале
    const int signUpIndent = ((((double)pageUpIndent)/((double)pageVerticalSize)) * (double)backgroundHeight); // отсуп по верстикале

    // считаем размеры рабочей области листа:
    const int workingWidht = backgroundWidth - (signLeftIndent + signRightIndent); // ширина рабочей области
    const int workingHeight = backgroundHeight - (signUpIndent + signDownIndent); // высота рабочей области

#ifdef DEBUGGING
    qDebug() << "Смещение слева: " << signLeftIndent << "смещение справа: " << signRightIndent << "смещение снизу: " << signDownIndent << "смещение сверху:" << signUpIndent;
#endif

    //====================================

    // считаем шаг картинки
    const int horizontalStep = workingWidht/columnsCount; // у нас 3 шага, значит делим на 3

    // реальный размер картинки
    const int signRealWidht = 60; // реальная ширина 6 см (по требованию заказчика)
    const int signRealHeight = 25; // реальная высота 2.5 см (по требованию заказчика)

    // считаем размер картинки подписи в писелях
    const int signImageWidht = ((((double)signRealWidht)/((double)pageHorizontalSize)) * (double)backgroundWidth);
    const int signImageHeight = ((((double)signRealHeight)/((double)pageVerticalSize)) * (double)backgroundHeight);

    //====================================

    // считаем межстрочный интервал для отрисовки текста
#ifdef INSERT_IN_CENTER
    if(drawingType == PREWIEW_DROW_PDF)
    {
        linesInterval = 1.0;
    }
#endif
    const int linesCountWithInterval = (double)linesCount/linesInterval; // получаем высоту прямоугольника, которую надо выделить для текста
    const int textInterval = workingHeight/linesCountWithInterval; // делим высоту рабочей области на количество строк (количество строк считает как количество с интервалом 1.0 делённое на интервал, т.к. интервал сжирает часть высоты строки)

#ifdef INSERT_IN_CENTER
    // считаем, сколько строк займёт картинка
    int imageRowsCount = getImageRowsCount(); // получаем количество строк, замнимаемое картинкой
#else
    int imageRowsCount = 4; // получаем количество строк, занимаемое картинкой
#endif
    drawLinesCount = linesCountWithInterval - imageRowsCount; // считаем сколько нужно отрисовать строк, с учетом того, что нужно оставить пустые для картинки

    //=====================================

    // считаем координаты картинки
#ifdef INSERT_IN_CENTER
    int drawImageLineNumber = 0; // номер строки, на которой надо нарисовать картинку
    if(yMode == 0)
    {
        drawImageLineNumber = drawLinesCount + 1; // номер строки равен фактическому количеству строк документа +1 для отрисовки после строки с датой/подписью
    }
    else
    {
        drawImageLineNumber = (drawLinesCount - imageRowsCount) - yMode + 1; // номер строки равен фактическому количеству строк документа, минус количество строк, выделяемых для картинки, минус необходимое смещение строк и +1, т.к последняя строка - дата, подпись, и её надо тоже учитывать
    }
#else
    int drawImageLineNumber = (drawLinesCount) - yMode + 1; // номер строки равен фактическому количеству строк документа, минус количество строк, выделяемых для картинки, минус необходимое смещение строк и +1, т.к последняя строка - дата, подпись, и её надо тоже учитывать
#endif
    const int startXpos = horizontalStep * xMode + signLeftIndent;
    const int startYpos = getYPos(drawImageLineNumber) /*+ (textInterval/(2 * linesInterval))*/; // получаем координаты для текста по номеру строки и делаем смещение на половину интервала, чтобы было как в ворде
#ifdef DEBUGGING
    qDebug() << "text interval = " << textInterval;
    qDebug() << "Координаты картинки: " << startXpos << startYpos;
#endif

    //====================================

    // создаем прямоугольник для подписи
    QRect smallRect(startXpos, startYpos, signImageWidht, signImageHeight);

    QPainter pntr(&this->img);
    QPen pen;
    pen.setStyle(Qt::SolidLine);
    pntr.setPen(pen);

    drawBigRect(&pntr);

#ifdef INSERT_IN_CENTER
    if(drawingType == PREWIEW_DROW_WORD)
    {
#endif
#ifdef DRAW_TEXT_IN_PREWIEW
        // настраиваем шрифт для текста
        QFont font = pntr.font();
        font.setFamily("MS Shell Dlg 2");
        int fontSize = img.width() / 41;
        if(fontSize < 1)
        {
            fontSize = 1;
        }
        font.setPointSizeF(fontSize);
        font.setBold(true);
        pntr.setFont(font);

        // отрисовываем текст
#ifdef DRAW_TEXT_IN_PREWIEW
        // сначала рисуем просто текст документа (в прямом смысле)

#ifdef DRAW_TEXT_TO_START_OF_PICTURE
        for (int i=0; i<drawImageLineNumber + (yMode == 0 ? -1 : 0); i++) // отрисовываем текст до картинки (если картинка в самом конце, то делаем -1, чтобы не залезть на строку дата/подпись)
        {
            drawText(i, "Пример расположения текста в документе");
        }
#else
        for (int i=0; i<drawLinesCount - imageRowsCount - 5; i++) // отрисовываем текст до строки дата/подпись, но оставляем 5 строк свободными
        {
            drawText(i, "Пример расположения текста в документе");
        }
#ifndef NO_DRAW_TEXT_ON_PICTURE
        if(yMode > 0) // если картинка находится в тексте, то пишем, что строка должна быть пустой
        {
            for (int i=drawImageLineNumber; i<drawImageLineNumber + imageRowsCount; i++) // пишем, какие строки нужно освободить от текста
            {
                drawText(i, "   Эта строка должна быть пустой !!!!");
            }
        }
#endif
#endif

#ifdef DRAW_END_TEXT
        for (int i=drawImageLineNumber + imageRowsCount; i<drawLinesCount; i++) // отрисовываем текст после картинки
        {
            drawText(i, "Пример расположения текста в документе");
        }
#endif

        //    drawText(drawLinesCount, "Дата: __.__.____ г.    Подпись: Иванов Н.В.");
        drawText(drawLinesCount, "              -- Последняя строка документа --");

#endif
#endif
#ifdef INSERT_IN_CENTER
    }
#endif

    pntr.drawImage(smallRect,rectImage); // рисуем картинку подписи

}

void signPrewiew::drawBigRect(QPainter *pntr)
{
    QRect bigRect(0,0,img.width()-1,img.height()-1);
    pntr->drawRect(bigRect);
    //    update();
}

int signPrewiew::getDrawLinesCount() const
{
    return drawLinesCount;
}
#ifdef INSERT_IN_CENTER
void signPrewiew::setLinesInterval(double value)
{
    linesInterval = value;
    drawSmallRect(smallImagePosXMode, smallImagePosYMode);
    update();
}

int signPrewiew::getImageRowsCount()
{
    int imageRowsCount = 5; // значение по умолчанию для интервала меньше 1.0
    if(linesInterval >= 1.0 && linesInterval < 1.5){ imageRowsCount = 4; }
    else if(linesInterval >= 1.5 && linesInterval < 2.0){ imageRowsCount = 3; }
    else if(linesInterval >= 2.0 && linesInterval <= 3.0){ imageRowsCount = 2; }
    else { imageRowsCount = 1; }
    return imageRowsCount;
}
#endif

void signPrewiew::setRectImage(const QImage &value)
{
    rectImage = value;
    rescale();
}

void signPrewiew::rescale()
{
    const int imageWight = this->height() /sqrt2; // ширина
    const int imageHeight = this->height() ; // высота
    img = QImage(imageWight, imageHeight, QImage::Format_A2BGR30_Premultiplied);
#ifdef DEBUGGING
    qDebug() << "Ширина. высота: " << imageWight << imageHeight;
#endif
    //    rectImage = QImage(":/images/image_gerb_384_160.jpg");
    drawSmallRect(smallImagePosXMode,smallImagePosYMode);
}

void signPrewiew::paintEvent(QPaintEvent *){
    QPainter pntr(this);
    //    qDebug() << "event sign";
    rescale();
    pntr.drawImage(0,0,img);
    //    Q_UNUSED(e);
}
