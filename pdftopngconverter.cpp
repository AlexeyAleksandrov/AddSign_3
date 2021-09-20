#include "pdftopngconverter.h"

PDFToPNGConverter::PDFToPNGConverter(QObject *parent) : QObject(parent)
{

}

PDFToPNGConverter::PDFLastPageInfo PDFToPNGConverter::getLastLineInfo(QString fileName, bool *ok)
{
    if(ok != nullptr) *ok = false;
    QString pdftopngDir = QDir::currentPath() + "/pdftopng/pdftopng.exe";
    if(!QFile::exists(pdftopngDir))
    {
        qDebug() << "Ошибка! Отсутствует файл запуска!";
        return PDFLastPageInfo();
    }
    QProcess PDF_To_PNG_process(this);
    PDF_To_PNG_process.start(pdftopngDir, QStringList() << fileName << "rootimage");
    PDF_To_PNG_process.waitForStarted();
    PDF_To_PNG_process.waitForFinished();

    int index = 0;
    while (QFile::exists(QDir::currentPath() + "/" + getImageFileNameWithIndex(index+1))) // пока существует файл с индексом на 1 больше
    {
        index++;
    }
    if(index == 0)
    {
         qDebug() << "Не найдены файлы картинок!";
         return PDFLastPageInfo();
    }
    for (int i=1; i<index; i++) // удаляем все файлы, кроме последнего
    {
        QString file = getImageFileNameWithIndex(i);
        if(QFile::exists(file))
        {
            QFile::remove(file);
        }
    }
    QString useImage = getImageFileNameWithIndex(index);
    QImage image(useImage, "PNG"); // загружаем картинку из последнего файла
//    message(QString("Высота: ") + QString::number(image.height()) + "\nШирина: " + QString::number(image.width()));
    for (int j=image.height()-1; j>=0; j--)
    {
        for (int i=0; i<image.width(); i++)
        {
            QColor color = image.pixelColor(i, j);
            if(color != Qt::white)
            {
                 qDebug() << "Найдена не пустая строка: " + QString::number(j);
                QFile::remove(useImage);
                PDFLastPageInfo info;
                info.width = image.width();
                info.height = image.height();
                info.lastLine = j - 40;

                int offset = (double)info.height * 0.058; // величина вертикального отступа (0.057 - примерный стандартный отступ)
//                info.lastLine = j - offset; // номер строки пикселя с учетом отступа

                int imageOffset = 180; // картинка занимает около 175 пикселей, но на всякий случай берём 180, чтобы наверняка
                if(offset > j) // если величина вертикального отсупа больше, чем найденная строка
                {
                    info.offsetLine = 30; // ставим строку на 30 (самая верхняя) (будет небольшой разрыв, но такое возможно только если установлен верхний колонтитул и при этом лист полностью пустой)
                }
                else if ((image.height() - offset - imageOffset) < j) // если картинка физически не вмещается в рамки страницы, с учетом нижнего колонтитула
                {
                    info.offsetLine = 30; // ставим самую верхнюю строку (т.к. новая страница)
                    info.onNextPage = true; // ставим, что происходит переход на новую страницу
                }
                else
                {
                    int workHeight = image.height() - offset*2; // для рассчёта высоты рабочей области вычитаем из общей высоты верхний и нижний колонтитул (т.к. они равны просто умножаем на 2)
                    int workImageLine = info.lastLine - offset; // вычитаем из высоты расположения последней строки величины нижнего колонтитула
                    int presetLine = 30 - workImageLine / ((workHeight - imageOffset)/30); // рассчитываем, какую из 30 строк должна занять картинка
                    if(presetLine < 30) presetLine++; // чтобы не задевать последний пиксель, скидываем ещё 1 строку
                    info.offsetLine = presetLine; // передаём информацию
                }

                if(ok != nullptr) *ok = true;
                return info;
            }
        }
    }
    QFile::remove(useImage);
    qDebug() << "Информация не была получена " << image.height() << image.width();
    PDFLastPageInfo info;
    info.width = image.width();
    info.height = image.height();
    info.lastLine = 0;  // ставим последнюю строку (верхнюю)
    info.onNextPage = false;
    info.offsetLine = 15;
    if(ok != nullptr) *ok = true;
    return info;

    // 118 - верхняя граница
    // 1608 - нижняя граница (без колонтитулов)
    // 1668 с картинкой
    // 1517 - без картинки
}

QString PDFToPNGConverter::getImageFileNameWithIndex(int index)
{
    QString name = "rootimage-";
    QString index_str = QString::number(index);
    while(index_str.count() < 6) // Докидываем 0 до 6 разрядов
    {
        index_str = "0" + index_str;
    }
    name.append(index_str); // добавляем индекс
    name.append(".png");
    return name;
}
