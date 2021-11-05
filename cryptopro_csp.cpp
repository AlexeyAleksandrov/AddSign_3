#include "cryptopro_csp.h"

#include <QFile>
#include <QTextCodec>

CryptoPRO_CSP::CryptoPRO_CSP(QObject *parent) : QObject(parent)
{
    CryptoProDirectory = CRYPTO_PRO_DIRECTORY;
}

void CryptoPRO_CSP::setCryptoProDirectory(const QString &value)
{
    CryptoProDirectory = value;
    if(value == "")
    {
        CryptoProDirectory = CRYPTO_PRO_DIRECTORY;
    }
    if(!value.endsWith("/") && !value.endsWith("\\"))
    {
        CryptoProDirectory.append("/");
    }
    certmgr.setCryptoProDirectory(CryptoProDirectory);
    csptest.setCryptoProDirectory(CryptoProDirectory);
}

int CryptoPRO_CSP::getSignIndex(QList<CryptoSignData> allSignsList, CryptoSignData searchSign)
{
    int index = 0; // индекс подписи в глобальном списке подписей (начало с 0, дальше +1)
    for(auto &&sig : allSignsList) // проходим по списку всех подписей
    {
//        qDebug() << "sig email = " << sig.email;
        if(sig.email == searchSign.email) // если e-mail совпадает
        {
            if(sig.serial == searchSign.serial) // если совпадают ещё и серийные номера, то это то, что нам надо
            {
                return index; // вызвращаем подпись с email и индексом в одной структуре
            }
            index++; // увеличиваем индекс
        }
    }
    return -1;
}

QString CryptoPRO_CSP::s_certmgr::getConsoleText(QStringList options)
{
    // запускаем процесс
    log.addToLog("запускаем процесс certmgr");
    QProcess certmgr_process;
    certmgr_process.setReadChannel(QProcess::StandardOutput);

    qDebug() << "runfile = " << runfile;
    certmgr_process.start(this->runfile, options); // запускаем процесс
    QString consoleText;
    if(!certmgr_process.waitForStarted())
    {
        qDebug() << "The process didn't start" << certmgr_process.error();
        log.addToLog(&"The process didn't start " [ certmgr_process.error()]);
        return "";
    }

    QString outText;

    while (!consoleText.contains("[ErrorCode:")) // читаем до того момента, пока не будет конец
    {
        if(!certmgr_process.waitForReadyRead()) // ждём доступа на чтение
        {
            qDebug() << "Поток не открыл доступ на чтение!";
            log.addToLog("Поток не открыл доступ на чтение!");
            return "";
        }
        while(certmgr_process.bytesAvailable())
        {
#ifdef _WIN32
            QTextCodec *codec = QTextCodec::codecForName("IBM 866");
#elif __linux__
            QTextCodec *codec = QTextCodec::codecForName("UTF-8");
#endif
            QString dirout =  codec->toUnicode(certmgr_process.readLine());
            outText.append(dirout);
//            log.addToLog("line = " + dirout);
        }
        consoleText.append(outText);
    }

    if(!certmgr_process.waitForFinished())
    {
        qDebug() << "The process didn't finished" << certmgr_process.error();
        log.addToLog(&"The process didn't finished " [ certmgr_process.error()]);
        return "";
    }

    log.addToLog("Текст certmgr получен");
//    log.addToLog("Полученный текст: " + consoleText);
//    qDebug() << "consoleText = " << consoleText;
    return consoleText;
}

QList<CryptoPRO_CSP::CryptoSignData>CryptoPRO_CSP::s_certmgr::getSertifactesList()
{
    log.addToLog("Запускаем процесс получения списка подписей");
    QString cmd_out = getConsoleText(QStringList() << "-list" << "-store" << "uMy");
//    QString cmd_out;
//    QFile filecmdout("C:/Users/ASUS/Desktop/cmd_out.txt");
//    filecmdout.open(QIODevice::ReadOnly);
//    while (!filecmdout.atEnd())
//    {
//        QString line = filecmdout.readLine();
//        cmd_out.append(line);
//    }

    if(cmd_out == "")
    {
        return QList<CryptoSignData>(); // возвращаем пустоту
    }
    QStringList cmdBlocksList = cmd_out.split((QString)"----", SPLITTER);
    for(auto && block : cmdBlocksList)
    {
        qDebug() << "block = " << block;
    }
    log.addToLog("cmd_out = " + cmd_out);
    QList<CryptoSignData> SignsList; // список подписей
    for(auto &&block : cmdBlocksList) // рзбиваем на блоки, которые представляют из себя подписи (это строки между 1------ и 2----- и т.д.
    {
        if((block.contains("Subject") && block.contains("Serial") && block.contains("Not valid before") && block.contains("Not valid after")) ||
                (block.contains("Субъект") && block.contains("Серийный номер") && block.contains("Выдан") && block.contains("Истекает")))
        {
            log.addToLog("Обрабатываем блок: " + block);
            CryptoSignData SignCMD;
            QString host_name_and_patronymic = "";
            QString host_surname = "";
            QStringList cmdLinesList = block.split(SPLITTER_NEW_LINE, SPLITTER);
            for(auto &&line : cmdLinesList)
            {
                // ФИО + e-mail
                log.addToLog("Обрабатываем строку: " + line);
                if(line.contains("Subject") || line.contains("Субъект"))
                {
                    QString subline = line.remove("Subject             : ");
                    subline = subline.remove("Субъект             : ");
                    QStringList sublineList = subline.split(", ", SPLITTER);
                    for(auto &&sublineData : sublineList)
                    {
                        QStringList pairList = sublineData.split("=", SPLITTER);
                        if(pairList.size() == 2)
                        {
                            if(pairList.at(0) == "CN")
                            {
                                SignCMD.name = pairList.at(1);
                            }
                            if(pairList.at(0) == "E")
                            {
                                SignCMD.email = pairList.at(1);
                            }
                            if(pairList.at(0) == "G")
                            {
                                host_name_and_patronymic = pairList.at(1); // имя + отчество
                                SignCMD.name_and_patronymic = pairList.at(1);
                            }
                            if(pairList.at(0) == "SN")
                            {
                                host_surname = pairList.at(1); // фамилия
                                SignCMD.surname = pairList.at(1);
                            }
                        }
                    }
                }
                // серийный номер (сертификат)
                if(line.contains("Serial") || line.contains("Серийный номер"))
                {
                    QString subline = line.remove("Serial              : 0x");
                    subline = subline.remove("Серийный номер      : 0x");
//                    qDebug() << "Серийный номер сертификата: " << subline;
                    SignCMD.serial = subline;
                }
                // дата начала действия
                if(line.contains("Not valid before") || line.contains("Выдан"))
                {
                    QString subline = line.remove("Not valid before    : ");
                    subline = subline.remove("Выдан               : ");
                    QStringList sublinbeList = subline.split("  ", SPLITTER);
                    if(sublinbeList.size() >= 2) // 2 или больше. Нам важна только 1я часть, это дата
                    {
                        QString dateSting = sublinbeList.at(0);
                        QStringList dateList = dateSting.split("/", SPLITTER); // запись сейчас в формате ДД/ММ/ГГГГ, парсим
                        if(dateList.size() == 3)
                        {
                            QDate date(dateList.at(2).toInt(), dateList.at(1).toInt(), dateList.at(0).toInt());
//                            qDebug() << "Начало действия: " << date;
                            SignCMD.startDate = date;
                        }
                    }
                }
                // дата окончания действия
                if(line.contains("Not valid after") || line.contains("Истекает"))
                {
                    QString subline = line.remove("Not valid after     : ");
                    subline = subline.remove("Истекает            : ");
                    QStringList sublinbeList = subline.split("  ", SPLITTER);
                    if(sublinbeList.size() >= 2) // 2 или больше. Нам важна только 1я часть, это дата
                    {
                        QString dateSting = sublinbeList.at(0);
                        QStringList dateList = dateSting.split("/", SPLITTER); // запись сейчас в формате ДД/ММ/ГГГГ, парсим
                        if(dateList.size() == 3)
                        {
                            QDate date(dateList.at(2).toInt(), dateList.at(1).toInt(), dateList.at(0).toInt());
//                            qDebug() << "Конец действия: " << date;
                            SignCMD.finishDate = date;
                        }
                    }
                }
            }
            // добавляем информацию об ФИО
            if(host_surname != "")
            {
                SignCMD.subname = host_surname;
            }
            if(host_name_and_patronymic != "")
            {
                if(SignCMD.subname != "")
                {
                    SignCMD.subname.append(" ");
                    SignCMD.subname.append(host_name_and_patronymic);
                }
            }
            if(!SignCMD.name.isEmpty() && !SignCMD.email.isEmpty()) // если данные есть
            {
                bool contains = false;
                for(auto &&sign : SignsList)
                {
                    if(sign.serial == SignCMD.serial)
                    {
                        contains = true;
                        break;
                    }
                }
                if(contains)
                {
//                    log.addToLog("WARNING: Данный сертификат уже существует! Добавление в список невозможно! Сертификат: " + SignCMD.toString());
                }
                else
                {
                    SignsList.append(SignCMD); // добавляем в общий список
//                    log.addToLog("Добавляем подпись в список: " + QString::number(SignsList.size()) + " - " + SignCMD.toString());
                }

            }
            else
            {
//                log.addToLog("WARNING: Сертификат не был добавлен в список, т.к. он не имеет названия, либо emqil. Данные о сертфикате: " + SignCMD.toString());
            }

        }
        else
        {
//            qDebug() << "Subject :" << block.contains("Subject") << "Serial :" << block.contains("Serial") << "Not valid before :" << block.contains("Not valid before") << "Not valid after :" << block.contains("Not valid after");
//            qDebug() << "block : " << block;
        }
    }
    // для каждой подписи формируем информацию о её индексе для создания sig файла
    for(auto &&sign : SignsList)
    {
        sign.index = getSignIndex(SignsList, sign); // получаем информацию об индексе
    }
    log.addToLog("Список подписей сформирован. Количество: " + QString::number(SignsList.size()));
    for (int i=0; i<SignsList.size(); i++)
    {
        auto sertVal = SignsList.at(i);
        log.addToLog("Сертификат " + QString::number(i) + ": " + sertVal.toString());
    }
    return SignsList;
}

void CryptoPRO_CSP::s_certmgr::setCryptoProDirectory(const QString &value)
{
    runfile = value + runfile;
}

bool CryptoPRO_CSP::s_csptest::createSign(QString file, CryptoPRO_CSP::CryptoSignData sign)
{
    log.addToLog("Запускатся процесс подписи файла " + file);
    QFile csptest_bat_file(QDir::currentPath() + "/csptest_bat.bat");
    QString bat_text = QString("echo %2 | \"") + runfile + QString("\" -sfsign -sign -detached -add -in %1 -out %1.sig -my %3"); // универсальный текст батника

    csptest_bat_file.open(QIODevice::WriteOnly);
    csptest_bat_file.write(bat_text.toUtf8()); // создаём батник
    csptest_bat_file.close();

    QString filebatDir = csptest_bat_file.fileName();
    if(!QFile::exists(filebatDir))
    {
        qDebug() << "Файл не найден " << filebatDir;
        log.addToLog("Файл не найден " + filebatDir);
        return false;
    }

    QProcess csptest_bat;
    csptest_bat.setReadChannel(QProcess::StandardOutput);

    if(!QFile::exists(file))
    {
        qDebug() << "Файл для подписи не найден " + file;
        log.addToLog("Файл для подписи не найден " + file);
        return false;
    }
//    if(sigFile.exists())
//    {
//        sigFile.remove(); // удаляем sig файл, если таковой уже имеется
//    }

    QStringList params = QStringList() << file << QString::number(sign.index) << sign.email;
    csptest_bat.start(csptest_bat_file.fileName(), params); // запускаем батник с параметрами
    if (!csptest_bat.waitForStarted())
    {
        qDebug() << "The process didnt start" << csptest_bat.error();
        log.addToLog(&"The process didnt start " [ csptest_bat.error()]);
        csptest_bat_file.remove();
        return false;
    }
    QString cmd_out;
    while (!cmd_out.contains("[ErrorCode:") && !cmd_out.contains("An error occurred in running the program."))
    {
        csptest_bat.waitForReadyRead(); // ждём готовности к чтению
        while(csptest_bat.bytesAvailable())
        {
            cmd_out.append(csptest_bat.readLine());
        }
    }
    if (!csptest_bat.waitForFinished())
    {
        qDebug() << "The process didnt finished" << csptest_bat.error();
        log.addToLog(&"The process didnt finished " [ csptest_bat.error()]);
        csptest_bat_file.remove();
        return false;
    }
    csptest_bat_file.remove();
    QFile sigFile(file + ".sig");
    if(sigFile.exists() && cmd_out.contains("[ErrorCode: 0x00000000]")) //  && cmd_out.contains("[ErrorCode: 0x00000000]")
    {
        log.addToLog("Подпись успешно создана " + sigFile.fileName());
        return true;
    }
    else
    {
        log.addToLog("Не удалось создать подпись - файл не найден " + sigFile.fileName());
        return false;
    }
}

void CryptoPRO_CSP::s_csptest::setCryptoProDirectory(const QString &value)
{
    runfile = value + runfile;
}
