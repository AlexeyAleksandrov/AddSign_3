#ifndef CRYPTOPRO_CSP_H
#define CRYPTOPRO_CSP_H

#include <QDate>
#include <QObject>
#include <QProcess>
#include <QDebug>

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
        QDate startDate;
        QDate finishDate;

        int index = -1; // информация о подписи для создания sig файла
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

    };
    struct s_csptest
    {
    public:
        bool createSign(QString file, CryptoSignData sign); // создать подпись для файла
        void setCryptoProDirectory(const QString &value);
    private:
        QString runfile = "csptest.exe";

    };

public:
    s_certmgr certmgr; // менеджер сертификатов
    s_csptest csptest; // отвечает за создание sig файла

};

#endif // CRYPTOPRO_CSP_H
