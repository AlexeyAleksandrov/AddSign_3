#ifndef SIGNPROCESSOR_H
#define SIGNPROCESSOR_H

#include <QObject>
#include <QDebug>

#include "logclass.h"

#include "files_status.h"
#include "wordeditor.h"
#include "exeleditor.h"
#include "qpdf_cmd.h"
#include "cryptopro_csp.h"
#include "pdfcreator.h"
#include "pdftopngconverter.h"
#include "libpoi.h"

class SignProcessor : public QObject
{
    Q_OBJECT
public:
    explicit SignProcessor(QObject *parent = nullptr);

    struct FileForSign
    {
        QString sourceFile;
        QString signWordFile;
        QString signPDFFile;
        bool isSigned = false;
    };

    struct WordParams
    {
        QString tempdir;
        QString outputdir;
        QString imageDir;
        bool ignoreMovingToNextList = false;
        bool exportToWord = false;
        bool exportToPDF = false;
        int signImageRowsCount = 2;
        int insertType = insert_standart;
        bool noInsertImage = false;
        QString signImageTag;
        QString signFioTag;

        QString getTempdir() const;
        QString getOutputdir() const;
        QString getImageDir() const;
    };

    enum signAlignment
    {
        left = 0,
        center = 1,
        rignt = 2
    };

    struct signPreset
    {
        int alignment = signAlignment::center; // выравнивание картинки
        int paragraphOffset = 0; // поднять картинку на N параграфов выше
    };

    struct htmlPreset
    {
        QString lineSertificate;
        QString lineOwner;
        QString lineDate;
    };

    struct PDFParams
    {
        QString qpdf_dir;
        QString image_dir;
//        QString output_dir;
        signPreset pdf_preset;
        htmlPreset htmlParams;
        bool drawLogo = true;
    };

    struct CryptoPROParams
    {
        CryptoPRO_CSP::CryptoSignData sign;
        QString CryptoPRODirectore = CRYPTO_PRO_DIRECTORY;
    };

    enum signInsertType
    {
        insert_standart,
        insert_in_exported_pdf,
        insert_by_tag_in_table
    };

    enum filesHandlers  // обработчики файлов
    {
        MS_COM,
        APACHI_POI,
        LIBRE_OFFICE
    };


    void setFilesList(const QList<FileForSign> &value);
    void setFilesList(const QStringList &files);

    void setWordOptions(const WordParams &value);
    void setPDFOptions(const PDFParams &value);
    void setCryptoPROOptions(const CryptoPROParams &value);

    void runProcessing(); // запуск обработчика файлов

private:
    QList<FileForSign> filesList; // список файлов, которые нужно подписать

    WordParams WordOptions;
    PDFParams PDFOptions;
    CryptoPROParams CryptoPROOptions;
    int filesHendlerType = filesHandlers::MS_COM;   // тип обработчика файлов

private:
    bool isExtension(QString file, QStringList filesExtensions);
    QStringList filesExtensionsWord = QStringList() << ".docx" << ".doc" << ".rtf";
    QStringList filesExtensionsExcel = QStringList() << ".xls" << ".xlsx"  << ".xlsm" << ".xlsb";
    QStringList filesExtensionsPDF = QStringList() << ".pdf";

    inline bool isWordFile(QString file);
    inline bool isExcelFile(QString file);
    inline bool isPDFFile(QString file);

    QString createFileCopy(QString sourceFile, QString dirForCopy, QString nextFileName = ""); // создаёт копию файла и возвращает путь к файлу
    QString getFileNameInPDFFormat(QString sourceFileName); // изменяет расширение файла на .pdf и возвращает его
    bool replaceOriginalFileByTemp(QString originalFile, QString tempFile); // заменяет содержимое основного файла, содержимым временного

    inline QString getFileName(QString file); // возвращает имя файла без директории
    static QString getDir(QString dir); // добавляет в конец директории /, если таковой необходим

private:
    void standartAddImageToWordFile(FileForSign &file, QString tempFile, QString imagedir, WordParams WordOptions, int &fileStatus, bool &movedToNextPage);   // выполняет стандартную вставку картинки в ворд документ (в конец)
    void addImageToWordFileByTagInTable(FileForSign &file, QString tempFile, QString imagedir, WordParams WordOptions, int &fileStatus, bool &movedToNextPage);   // выполняет вставку картинки в ворд документ в таблицу по специальным тэгам
    void addImageToPdfFileInCoordinates(QString input_file, QString output_file, WordParams WordOptions, PDFParams PDFOptions, PDFCreator::orientation pdfPageOrientation, QString gerb_file, int &fileStatus);  // вставить подпись по координатам в PDF документ
    void addImageToPdfFileInEndOfFile(QString input_file, QString output_file, WordParams WordOptions, PDFParams PDFOptions, PDFCreator::orientation pdfPageOrientation, QString gerb_file, int &fileStatus);  // вставить подпись по координатам в PDF документ

signals:
    void newFileStatus(FileForSign, int status);
    void procesingFinished(); // процесс подписи завершился

private:
    struct AutoDeleter
    {
        AutoDeleter(QString controlFile)
        {
            this->controlFile = controlFile;
        }
        ~AutoDeleter();

    private:
        QString controlFile;
    };

public:
    bool *closing = nullptr;

    void setFilesHendlerType(int newFilesHendlerType);

private:
    logClass log;

};

#endif // SIGNPROCESSOR_H
