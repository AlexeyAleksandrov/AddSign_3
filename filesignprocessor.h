#ifndef FILESIGNPROCESSOR_H
#define FILESIGNPROCESSOR_H

#include <QObject>
#include <QStringList>
#include <files_status.h>
#include <logclass.h>

#include "signprocessor.h"

#define DEBUGGING

class fileSignProcessor : public QObject
{
    Q_OBJECT
public:
    explicit fileSignProcessor(QObject *parent = nullptr);
    ~fileSignProcessor();

    SignProcessor processor;

//    void setClosing(bool value);

private:


public:
//    void setWord_settings(const SignProcessor::WordParams &value);
//    void setPDF_settings(const SignProcessor::PDFParams &value);
//    void setCryptoPRO_settings(const SignProcessor::CryptoPROParams &value);

//    QStringList getFiles() const;


//    void setWordSignPreset(const SignProcessor::signPreset &value);

//    void setWord_insert_type(int value);

public slots:
    void run();
    //    void fileSigned(QString file, QProcess::ExitStatus exitStatus); // сигнал о том, что файл подписан
//    void fileReady(QString fileDir, bool isSigned, QStringList outputFiles);
//    void signAdded(QString file, bool isSigned);
//    void updateWordFileStatus(QString fileDir, int status);
//    void wordTimerTimiout(); // слот окончания времени ожидания ворда


signals:
    void ready();
//    void newFileStatus(QString fileDir, int status);
//    void fileSigned(QString fileDir, bool isSigned, QStringList outputFiles);

private:
//    void setWord(WordEditor *value);
//    void nextFile(); // переход к следующему файлу
//    void signNextFileInQueueSign(); // подписывает следующий на очереди файл (word/pdf)

private:
//    WordEditor *word = nullptr;
//    QThread *wordThread = nullptr;
//    WordEditor::signPreset wordSignPreset;
//    qpdf_adder pdf_adder;
//    CryptoCMD *crypto = nullptr;
//    QStringList files; // исходные файлы
//    QString currentFile; // текущий обрабатываемый исходный файл
//    QString currentExportedPDF; // текущий экспортируемый файл
//    int currentStatus; // статус текущего файла
//    QStringList signingsFiles; // текущие выходные файлы, которые нужно подписать
//    QStringList allSigningsFiles; // все выходные файлы, которые нужно подписать
//    bool closing = false;
//    int word_insert_type = insert_standart;

//    cryptosign *sign = nullptr; // подпись, нужно каждый раз пересоздавать
//    QString sign_index;
//    QString sign_email;

    logClass log;

};

#endif // FILESIGNPROCESSOR_H
