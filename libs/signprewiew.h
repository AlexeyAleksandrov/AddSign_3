#ifndef SIGNPREWIEW_H
#define SIGNPREWIEW_H

#include <QWidget>
#include <QPainter>
#include <QPen>
#include <QDebug>

#define sqrt2 1.4142857
//#define widthCoeff 0.285714286
//#define heightCoeff 0.0841750842

#define PREWIEW_DROW_WORD 0
#define PREWIEW_DROW_PDF 1


//#define DEBUGGING
//#define INSERT_IN_CENTER
//#define DRAW_TEXT_IN_PREWIEW
#ifdef DRAW_TEXT_IN_PREWIEW
#define DRAW_EXAPLE_TEXT
#define DRAW_TEXT_TO_START_OF_PICTURE
#define NO_DRAW_TEXT_ON_PICTURE
#define DRAW_END_TEXT
#endif

class signPrewiew : public QWidget
{
    Q_OBJECT
public:
    explicit signPrewiew(QWidget *parent = nullptr);
    void drawSmallRect(int xMode, int yMode);
    void setRectImage(const QImage &value);

    void rescale();

#ifdef INSERT_IN_CENTER
    void setLinesInterval(double value);
    int getImageRowsCount(); // получить количество строк, занимаемое картинкой
#endif

    int getDrawLinesCount() const;

private:
    void paintEvent(QPaintEvent *);
    QImage img;
    void drawBigRect(QPainter *pntr);
    QImage rectImage;

    double linesInterval = 1.0; // межстрочный интервал

    int smallImagePosXMode = 0;
    int smallImagePosYMode = 0;

    int drawLinesCount = 1; // количество строк, которое будет отрисовано

#ifdef INSERT_IN_CENTER
    int drawingType = PREWIEW_DROW_WORD;
#endif

signals:

};

#endif // SIGNPREWIEW_H
