#ifndef LIBREOFFICE_H
#define LIBREOFFICE_H

#include <QObject>
#include <QDir>
#include <QProcess>
#include <QDebug>

class LibreOffice : public QObject
{
    Q_OBJECT
public:
    explicit LibreOffice(QObject *parent = nullptr);

    enum LibreOfficeFormats // поддерживаемые форматы
    {
        docx,
        doc,
        rtf,
        xlsx,
        xls,
        pdf,
        no_supported
    };

    void convertFile(QString inputFile, QString outputFile = "", LibreOfficeFormats inputFormat = LibreOfficeFormats::pdf, bool *ok = nullptr);    // функция конвертации файла в другой формат

    void setTempDir(const QString &newTempDir);

    void setSofficePath(const QString &newSofficePath);

signals:

private:
    QString getFormatName(LibreOfficeFormats format);
    QString getFileName(QString file);

private:
    QString tempDir;    // папка для временных файлов
    QString sofficePath = "soffice";    // путь к файлу soffice


};

#endif // LIBREOFFICE_H
