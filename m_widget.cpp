#include "m_widget.h"

m_widget::m_widget(QWidget *parent) : QWidget(parent)
{
    pen.setWidth(5);
    brush.setColor(Qt::black);

    addImage("С гербом", ":/images/image_gerb_384_160.jpg");
    addImage("Без герба", ":/images/image_without_gerb_384_160.jpg");
    setCurrentImage(imagesList.at(0).imageDir);
}

void m_widget:: paintEvent(QPaintEvent *)
{
#ifdef DEBUGGING
    qDebug() << "m_widget event";
#endif

    QPainter painter(this);

    //    QImage image = drawingImage;
    this->setGeometry(0, 0, drawingImage.width(), drawingImage.height());
    //    painter.drawImage(0, 0, QImage(image));
    painter.drawImage(0, 0, drawingImage);

    //    emit updatePrewiew();

}

void m_widget::setWidth(int widht)
{
    pen.setWidth(widht);
}

void m_widget::setScale(double scalefactor)
{
    imageTransformer scaler;
    QImage image(imageDir); // текущая картинка
    QImage scaledImg = scaler.getScaledImage(image, scalefactor); // получаем отмасштабирвоанную картинку
    drawingImage = scaledImg;
    this->setGeometry(0, 0, drawingImage.width(), drawingImage.height());
#ifdef DEBUGGING
    qDebug() << "m_widget setScale";
#endif
    this->repaint();
}

void m_widget::setCurrentImage(QString imageName)
{
    for (auto &&image : imagesList)
    {
        if(image.imageName == imageName)
        {
            imageDir = image.imageDir;
            break;
        }
    }
    if(imageDir == "")
    {
#ifdef DEBUGGING
        qDebug() << "m_widget Ошибка пути к файлу картинки";
#endif
        return;
    }
    drawingImage = QImage(imageDir);
    this->setGeometry(0, 0, drawingImage.width(), drawingImage.height());
#ifdef DEBUGGING
    qDebug() << "перерисовываем" << imageDir /* << "parent - " << this->parent()*/;
#endif
    this->repaint();
}

void m_widget::addImage(QString imageName, QString imagePath)
{
    imagesList.append(imagesData(imageName, imagePath)); // добавляем данные в список
}

m_widget::imagesData::imagesData(QString name, QString path)
{
    this->imageName = name;
    this->imageDir = path;
}
