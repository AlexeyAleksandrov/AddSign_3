#ifndef PDFTOPNGCONVERTER_H
#define PDFTOPNGCONVERTER_H

#include <QObject>
#include <QProcess>
#include <QFile>
#include <QDir>
#include <QImage>
#include <qdebug.h>

#ifdef __linux__
    #define PDFTOPNG_DIRECTORY "/pdftopng/pdftopng"
#elif _WIN32
    #define PDFTOPNG_DIRECTORY "/pdftopng/pdftopng.exe"
#endif

class PDFToPNGConverter : public QObject
{
    Q_OBJECT
public:
    explicit PDFToPNGConverter(QObject *parent = nullptr);

    struct PDFLastPageInfo
    {
        int height = -1;
        int width = -1;
        int lastLine = -1;
        int offsetLine = 0;
        bool onNextPage = false;
    };

    PDFLastPageInfo getLastLineInfo(QString fileName, bool *ok = nullptr);

signals:

private:
    QString getImageFileNameWithIndex(int index);

};

#endif // PDFTOPNGCONVERTER_H
