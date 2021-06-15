#include "imagetransformer.h"

imageTransformer::imageTransformer(QObject *parent) : QObject(parent)
{

}

QImage imageTransformer::getTransformedImage(QImage source, int widht, int heigh)
{
    QPixmap pixmap = QPixmap::fromImage(source); // переводим в pixmax

    QLabel* imageLabel = new QLabel; // создаём временный label для вставки в него и масштабирования через него же

    imageLabel->setBackgroundRole(QPalette::Dark); // ставим черный фон (так было в примере)
    imageLabel->setScaledContents(true); // разрешаем масштабирование элементов
    imageLabel->setPixmap(pixmap); // ставим картинку как pixmap в label

    imageLabel->setFixedWidth(widht); // устанавливаем ширину label
    imageLabel->setFixedHeight(heigh); // устанавливаем высоту label

    // производим сохранение картинки
    pixmap = QPixmap(imageLabel->size());
    imageLabel->render(&pixmap);

    delete imageLabel;
    imageLabel = nullptr;

    return pixmap.toImage();
}

QImage imageTransformer::getScaledImage(QImage source, double scalefactor)
{
    if(scalefactor < 0.01)
    {
        return QImage();
    }
    else
    {
        return getTransformedImage(source,
                                   source.width() * scalefactor,
                                   source.height() * scalefactor);
    }
}
