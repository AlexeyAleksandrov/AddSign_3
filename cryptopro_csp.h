#ifndef CRYPTOPRO_CSP_H
#define CRYPTOPRO_CSP_H

#include <QDate>
#include <QObject>
#include <QProcess>
#include <QDebug>

#include "logclass.h"

class CryptoPRO_CSP : public QObject
{
    Q_OBJECT
public:
    explicit CryptoPRO_CSP(QObject *parent = nullptr);

    void setCryptoProDirectory(const QString &value);

    // главная структура
    struct CryptoSignData
    {
        // общая информацию о подписи
        QString name = "";
        QString email = "";
        QString serial = "";
        QString subname = "";
        QString surname = "";
        QString name_and_patronymic = "";
        QDate startDate;
        QDate finishDate;

        int index = -1; // информация о подписи для создания sig файла

        QString toString()
        {
            return "email = " + this->email + ", name = " + this->name + ", serial = " + this->serial + ", subname = " + this->subname + ", surname = " + this->surname + ", name_and_patronymic = " + this->name_and_patronymic + ", date_start = " + this->startDate.toString() + ", date_end = " + this->finishDate.toString();
        }
    };

signals:

private:
    QString CryptoProDirectory;



    static int getSignIndex(QList<CryptoSignData> allSignsList, CryptoSignData searchSign); // получает подпись по e-mail

    struct s_certmgr
    {
    public:
        QString getConsoleText(QStringList options); // запустить программу с опциями
        QList<CryptoSignData> getSertifactesList(); // получить список сертификатов
        void setCryptoProDirectory(const QString &value);
    private:
        QString runfile = "certmgr.exe";
        logClass log;

    };
    struct s_csptest
    {
    public:
        bool createSign(QString file, CryptoSignData sign); // создать подпись для файла
        void setCryptoProDirectory(const QString &value);
    private:
        QString runfile = "csptest.exe";
        logClass log;
    };

public:
    s_certmgr certmgr; // менеджер сертификатов
    s_csptest csptest; // отвечает за создание sig файла

private:
    logClass log;

};

#endif // CRYPTOPRO_CSP_H
