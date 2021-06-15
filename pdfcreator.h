#ifndef PDFCREATOR_H
#define PDFCREATOR_H

#include <QObject>

class PDFCreator : public QObject
{
    Q_OBJECT
public:
    explicit PDFCreator(QObject *parent = nullptr);

    enum orientation
    {
        Portrait = 0,
        Landscape = 1
    };

    bool createSignPDFbyHTML(QString fileName, int image_pos_x, int image_pos_y, QString mireaLogoFileName, QString line_sertifacate, QString line_owner, QString line_validTime, orientation orientation, bool drawMireaLogo);


signals:

private:
    QString getLine(QString symvol, int lenght);
    QString tab(int count);

};

#endif // PDFCREATOR_H
