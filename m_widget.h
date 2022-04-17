#ifndef M_WIDGET_H
#define M_WIDGET_H

#include <QWidget>
#include <QPaintEvent>
#include <QDebug>
#include <QPainter>
#include <QList>
#include <libs/imagetransformer.h>

//#define DEBUGGING

class m_widget : public QWidget
{
    Q_OBJECT
public:
    explicit m_widget(QWidget *parent = nullptr);

    void paintEvent(QPaintEvent *);

    void setWidth(int widht);

    void setScale(double scalefactor);

    struct imagesData
    {
        QString imageName; // название для пользователя
        QString imageDir; // директория хранения файла картинки
        imagesData(QString name, QString path);
    };

    QList<imagesData> imagesList; // список доступных картинок
    void setCurrentImage(QString imageName); // уставновка актуальной картинки по пути к файлу
    void addImage(QString imageName, QString imagePath); // добавление картинки

signals:
//    void updatePrewiew(); // сигнал обноления превью

private:
    QPen pen;
    QBrush brush;
    QString imageDir;
    QImage drawingImage;
};

#endif // M_WIDGET_H
