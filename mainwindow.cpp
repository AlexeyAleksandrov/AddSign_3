#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "qstringview.h"

#include <QMenu>

#define SETTINGS_PASSWORD "oozioozi21" // worker21
#define PROGRAM_INFORMATION "Версия программы: 2.9.1 Beta\n\nРазработчики:\nАлександров А.С.\nБабуркина С.С.\n\nОтдел обеспечения защиты информации и управления информатизации\nРТУ МИРЭА - Российский Технологический Университет\nДата релиза: 27.05.2021"

#define SHOW_MSG_DEBUG // РАЗКОММЕНТИРВОАТЬ ДЛЯ ДЕБАГА
//#define CHECK_POBEL_IN_DIRECTORY_BY_ADD_FILE // проверять наличие пробелов в пути к файлу, при добалении через кнопку добавить
#define OPEN_WORD_BY_DOUBLECLICK // открывать вордовские файлы по даблклику

#define STACKED_PREVIEW 0
#define STACKED_EDIT_PREVIEW 5
#define STACKED_SHABLONI 4
#define STACKED_FILES 3
#define STACKED_SYSTEM_SETTINGS 2
#define STACKED_SIGN_SETTINGS 1
#define STACKED_SIGN_POSITION_PREVIEW 6

#define LABEL_CERTIFICATE 3
#define LABEL_SIGN_HOSTER 4
#define LABEL_VALID_TIME 5

#define TABLE_CERTIFICATE_ROW_NAME 0
#define TABLE_CERTIFICATE_ROW_SERIAL 1
#define TABLE_CERTIFICATE_ROW_DATE_START 2
#define TABLE_CERTIFICATE_ROW_DATE_END 3
#define TABLE_CERTIFICATE_ROW_SUBJECT 4


// автоматический define сплиттера, в зависимости от версии компилятора
// если не произошла проблема, исправь версию. Чтобы узнать версию используй QString::number(QT_VERSION, 16)
#if QT_VERSION >= 0x050f00 // версия Qt 5.15.0
#define SPLITTER Qt::SplitBehavior(Qt::SkipEmptyParts)
#else
#define SPLITTER QString::SkipEmptyParts
#endif

//#define REMOVE_OPENED_FILES_FROM_LIST_WIDGET // оставить в ListWidget только те файлы, которые получилось открыть


#define LOG_FILE "log_"+(QDate::currentDate()).toString("dd_MM_yyyy")+".txt"
#define TEMP_BAT_FILE (QString(TEMP_DIR) + QString("/temp.bat"))

#define PDF_FILES_EXTENSIONS

#ifdef PDF_FILES_EXTENSIONS
#define FILES_EXTENSIONS "Files for sign (*.docx *.doc *.rtf *.pdf *.xlsx *.xls);"    // поддерживаемые расширения файлов
#else
#define FILES_EXTENSIONS "Word Files (*.docx *.doc *.rtf);"    // поддерживаемые расширения файлов
#endif

#define SUBFOLDER_NAME "Подписанные файлы AddSign/" // дополнительно создаваемая директория

//#define QPDF_DIR "C:/Users/ASUS/Downloads/qpdf-10.3.1-bin-mingw32 (2)/bin/qpdf.exe"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    QDir dir(QDir::currentPath() + TEMP_DIR);
    //    qDebug() << dir.rmdir(QDir::currentPath() + TEMP_DIR);
#ifdef DEBUGGING
    qDebug() << dir.removeRecursively();
    qDebug() << dir.mkdir(QDir::currentPath() + TEMP_DIR);
#else
    dir.removeRecursively();te
    dir.mkdir(QDir::currentPath() + TEMP_DIR);
#endif

    log.writter = "MainWindow";
    log.autoDeleteLog();
    log.filelog.setFileName(LOG_FILE);
    ui->tableWidget_filestatus->horizontalHeader()->setStyleSheet("background-color: black; color: black;");
    log.addToLog("//////////////////////////////////////Начало работы программы//////////////////////////////////////////");

    m_widg = new m_widget(ui->sign_widget);

    QString qpdfdir;
    findQpdf(qpdfdir);
    ui->lineEdit_qpdfexe_dir->setText(qpdfdir);

    ui->pushButton_addsign->setEnabled(false);
//    connect(&cryptoObject, &CryptoCMD::cryptoSignListReady, this, &MainWindow::cryptoSignListReady);
//    cryptoObject.startSearchSigs();

    log.addToLog("Начат поиск подписей в списке доверенных сертификатов");

    connect(ui->tableWidget_filestatus, &my_tableWidget::mouseRightClick, this, &MainWindow::filesTableMouseRightClick); // связываем контекстное меню
    connect(ui->tableWidget_filestatus, &my_tableWidget::dropFiles, this, &MainWindow::addFiles); // связываем добавление файлов по drag and drop
    connect(ui->tableWidget_filestatus, &my_tableWidget::mouseDoubleClick, this, &MainWindow::filesTableMouseDoubleClick); // связываем дабл-клик на откртие файла

    /*
    ui->tableWidget_filestatus->setStyleSheet(
                "QHeaderView::section{backgroud-color:yellow} \
                QHeaderView::section:hover{Background-color:red}\
                QTableCornerButton::section{Background-color:blue}\
                QHeaderView{background-color:greeen}");
                */
//    ui->tableWidget_filestatus->setStyleSheet("QHeaderView::section{background-color:yellow} \
//                QHeaderView::section:hover{background-color:red}\
//                QTableCornerButton::section{background-color:blue}\
//                QHeaderView{background-color:green}");

//        ui->tableWidget_filestatus->setStyleSheet("QHeaderView::section{background-color:yellow} \
//                QHeaderView::section:hover{background-color:red}\
//                QTableCornerButton::section{background-color:blue}\
//                QHeaderView{background-color:green}");

    CryptoPRO.setCryptoProDirectory("");
//    auto sertificates = CryptoPRO.certmgr.getSertifactesList();
//    cryptoSignListReady(sertificates); // обрабатываем полученные сертификаты
}

MainWindow::~MainWindow()
{
    saveProgramData(); // сохраняем данные
    QDir dir(QDir::currentPath() + TEMP_DIR);
    #ifdef DEBUGGING
    qDebug() << dir.removeRecursively(); // очищаем временную папку
#else
//    dir.removeRecursively(); // очищаем временную папку
#endif
    //presets.updateFile();
    log.addToLog("-------------------------------------Завершение работы программы---------------------------------------");


    delete ui;
}

void MainWindow::customConstructor()
{
//    log.addToLog("Шаблоны из списка доверенных сертификатов загружены:", presets);
    ui->stackedWidget->setCurrentIndex(STACKED_PREVIEW);

    QTextCodec::setCodecForLocale(QTextCodec::codecForName("IBM 866"));

    labelList.append(ui->label); // документ подписан
    labelList.append(ui->label_2); // электронной подписбю
    labelList.append(ui->label_3); // пустая строка
    labelList.append(ui->label_4); // сертификат
    labelList.append(ui->label_5); // владелец
    labelList.append(ui->label_6); // действителен с до

    // настраиваем кастомный виджет
    for (auto &label : labelList)
    {
        label->setParent(m_widg);
        ui->widget_gridLayout->addWidget(label);
    }

    log.addToLog("Начато рисование виджета с превью");

    int scale_f = 2;    // устанавливаю геометрию
    int x_size = 192 * scale_f;
    int y_size = 80 * scale_f;

    for (auto &&image : m_widg->imagesList)
    {
        ui->comboBox_choseImageGerb->addItem(image.imageName);
    }
    QStringList listHosterNameTypes;
    listHosterNameTypes << "Имя владельца" << "Субъект подписи";
    ui->comboBox_signHost->addItems(listHosterNameTypes);

    ui->sign_widget->setMinimumHeight(y_size);
    ui->sign_widget->setMaximumHeight(y_size );
    ui->sign_widget->setMinimumWidth(x_size);
    ui->sign_widget->setMaximumWidth(x_size);

    ui->sign_widget->repaint();

    ui->tableWidget_filestatus->setColumnWidth(0, 270);
    ui->tableWidget_filestatus->setColumnWidth(1, 230);
    log.addToLog("Загрузка параметров");

    // составляем список checkBox, которые надо сохранить/загрузить
    uisaver.add(ui->checkBox_signingOut);
    uisaver.add(ui->checkBox_outputInSelectedPath);
    uisaver.add(ui->checkBox_exportWordToPDF);
    uisaver.add(ui->checkBox_createSubFolder);
    uisaver.add(ui->checkBox_signWordDocument);
    uisaver.add(ui->checkBox_subjectInfo);

    // составляем список lineEdit, колторые надо сохранить/загрузить
    uisaver.add(ui->lineEdit_wordDirectory);
    uisaver.add(ui->lineEdit_userOutputPath);
    uisaver.add(ui->lineEdit_tempFilesDirectory);
    uisaver.add(ui->lineEdit_qpdfexe_dir);

    // составляем список comboBox, колторые надо сохранить/загрузить
    uisaver.add(ui->comboBox_choseImageGerb); // comboBox_choseImageGerb
    uisaver.add(ui->comboBox_signHost); // выбор имя/субъекта для превью
    uisaver.add(ui->comboBox_changePreset); // пресет

    // добавляем spinBox
    uisaver.add(ui->doubleSpinBox_scalekoefficient); // масштаб текста подписи (масштаб системы)

    // добавляем слайдеры
    uisaver.add(ui->horizontalSlider); // слайдеры выбора позиции в ворде
    uisaver.add(ui->verticalSlider);

    // добавляем mainWindow
    uisaver.add(this);

    // пресеты
    presets.add(ui->verticalSlider);
    presets.add(ui->horizontalSlider);

    presets.add(ui->checkBox_exportWordToPDF);
    presets.add(ui->checkBox_signWordDocument);
    presets.add(ui->radioButton_usually_insert);
    presets.add(ui->radioButton_insert_in_exported_pdf);

    ui->radioButton_runSourceFile->setChecked(true);  // по умолчанию ставим, чтобы запускался исходный файл
    reshowSidebar(); // прячем бокове меню

    auto sertificates = CryptoPRO.certmgr.getSertifactesList();
    cryptoSignListReady(sertificates); // обрабатываем полученные сертификаты

    log.addToLog("Конструктор завершён");
}

void MainWindow::on_pushButton_addsign_clicked()
{
    // === ПРОВЕРКИ ===

    if(signThread) // если поток уже запущен
    {
//        signProscessor->setClosing(true); // Отправляем, что нужно прервать
        log.addToLog("Прерывание обработки word");
        wordCancel = true;
        return;
    }

    QStringList listAddedFiles = getAddedFiles(); // получаем список добавленных файлов

#ifdef DEBUGGING
    qDebug() << "Список файлов: " << listAddedFiles;
#endif
    log.addToLog("Список добавленнных для обработки файлов",listAddedFiles);

    if(listAddedFiles.size() == 0)
    {
        QMessageBox::warning(this, "Ошибка", "Вы не добавили файлы!");
        log.addToLog("Ошибка, Вы не добавили файлы!");
        return;
    }

    QString tempFilesDir = ui->lineEdit_tempFilesDirectory->text();
    if(tempFilesDir.isEmpty())
    {
        log.addToLog("Ошибка, Не введена директория хранения временных файлов! Применена директория по умолчанию");
        tempFilesDir = QDir::currentPath();
    }
    if(tempFilesDir == QDir::currentPath())
    {
        tempFilesDir.append(TEMP_DIR);
    }
    QDir dir;
    dir.mkdir(tempFilesDir); // создаём директорию

    QString outputDir = ""; // директория, в которую будет произведёт вывод. Пустая = в дирекорию рядом с файлом
    if(ui->checkBox_outputInSelectedPath->isChecked()) // если стоит знаечние вывода в щзаданную пользователем папку
    {
        outputDir = ui->lineEdit_userOutputPath->text(); // получаем текст директории вывода
        if(outputDir == "") // если путь вывода пустой
        {
            QMessageBox::warning(this, "Ошибка", "Вы выбрали сохранение подписанных файлов в выбранную папку, но не указали её.\nПожалуйста, укажите, в какую папку сохранить подписанные файлы.");
            return;
        }
    }

    QString currentSertificate = getSignByIndex(ui->comboBox->currentIndex()).serial; // полулчаем текущий сертификат
    if(currentSertificate.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Вы не выбрали сертифакат!");
        return;
    }
    auto sertList = CryptoPRO.certmgr.getSertifactesList();
    if(sertList.size() <= ui->comboBox->currentIndex() || ui->comboBox->currentIndex() < 0)
    {
        QMessageBox::warning(this, "Ошибка", "Ошибка соответствия сертификата.\nПопробуйте перезагрузить список сертификатов.");
        return;
    }
    auto currentSign = sertList.at(ui->comboBox->currentIndex()); // пытаемся получить полную электронную подпись по данному сертификату
    if(!currentSign.serial.isEmpty() && currentSign.serial != currentSertificate) // если сертификат не пустой и не соответсвует искомому, значит какая-то ошибка
    {
        QMessageBox::warning(this, "Ошибка", "Ошибка соответствия сертификата.\nПопробуйте перезагрузить список сертификатов.");
        return;
    }

    bool containsWordFiles = false;
    bool containsPDFFiles = false;
    for (auto &&file : listAddedFiles)
    {
        if(file.endsWith(".docx") || file.endsWith(".doc") || file.endsWith(".rtf"))    // если хоть один файл вордовский
        {
            containsWordFiles = true; // ставим флаг
        }
        if(file.endsWith(".pdf") || file.endsWith(".xlc") || file.endsWith(".xlcx"))    // если хоть один файл вордовский
        {
            containsPDFFiles = true; // ставим флаг
        }
        if(containsWordFiles && containsPDFFiles)
        {
            break; // и выходим, дальше искать нет смысла
        }
    }

    bool exportToWord = ui->checkBox_signWordDocument->isChecked();
    bool exportToPDF = ui->checkBox_exportWordToPDF->isChecked();

    int signType = SignProcessor::insert_standart; // тип подписи вордовского файла
    if(ui->radioButton_usually_insert->isChecked()) signType = SignProcessor::insert_standart;
    else if (ui->radioButton_insert_in_exported_pdf->isChecked()) signType = SignProcessor::insert_in_exported_pdf;

    if(containsWordFiles) // если содержится хотя бы 1 файл word
    {
        if(!exportToWord && (signType == SignProcessor::insert_standart) && !exportToPDF)
        {
            QMessageBox::warning(this, "Ошибка", "Не выбраны действия с Word'ом");
            return;
        }
    }

    QString qpdf_dir = ui->lineEdit_qpdfexe_dir->text();
    if(containsPDFFiles || signType == SignProcessor::insert_in_exported_pdf) // если содержатся PDF файлы, то ищем QPDF
    {
        if(qpdf_dir == "")
        {
            if(!findQpdf(qpdf_dir))
            {
                QMessageBox::warning(this, "Ошибка", "Ошибка! Не указан путь к qpdf.exe");
                log.addToLog("Ошибка! Не указан путь к qpdf.exe");
                return;
            }
        }
        if(!QFile::exists(qpdf_dir))
        {
            if(!findQpdf(qpdf_dir))
            {
                QMessageBox::warning(this, "Ошибка", "Ошибка! Файл qpdf.exe не найден!");
                log.addToLog("Ошибка! Файл qpdf.exe не найден!");
                return;
            }
        }
    }

    // страый метод
//    cryptoObject.setCurrentSign(currentSign); // устанавливаем актуальную подпись

    // === ПОДГОТОВКА К ЗАПУСКУ ===

    log.addToLog("Обработка превью превью в картинку");
    //    // получавем картинку с виджета
    QPixmap pixmap(ui->sign_widget->size());
    ui->sign_widget->render(&pixmap);

    imageDir = QDir::current().currentPath() + TEMP_DIR + "/image.jpg"; // создаем директорию для хранения файла картинки
    pixmap.save(imageDir);

    log.addToLog("Получена картинка с виджета");

    for (auto &&file : listAddedFiles)
    {
        setFileStatus(file, files_status::waiting); // ставим всем файлам статус ожидания
    }

    dir.mkpath(outputDir); // создаём директорию вывода
    log.addToLog("Запуск Word");
    countFilesReady = 0; // обнуляем счётчик обработанных файлов

    clearAllOutputsToolTipData(); // очищаем пути вывода для всех файлов в таблице

    SignProcessor::WordParams word_settings;
//    word_settings.files = listAddedFiles; // передаём файлы для обработки
    word_settings.tempdir = tempFilesDir + "/"; // директория для хранения временных файлов
    word_settings.outputdir = outputDir; // директория, в которую будем выводить
    word_settings.imageDir = imageDir; // устанавливаем директорию, где находится картинка
    word_settings.ignoreMovingToNextList = ui->checkBox_signingOut->isChecked(); // устанавливаем, игнорировать переходы на новую стриницу или нет
    word_settings.exportToWord = exportToWord; // передаём, нужно-ли подписывать документ Word
    word_settings.exportToPDF = exportToPDF; // устанавдиваем, нужно-ли экспортировать в PDF
    word_settings.insertType =(ui->radioButton_usually_insert->isChecked() ? SignProcessor::insert_standart : SignProcessor::insert_in_exported_pdf); // подставляем тип вставки картинки

    SignProcessor::signPreset pdfSignPreset;
    pdfSignPreset.alignment = ui->horizontalSlider->value();
    pdfSignPreset.paragraphOffset = ui->verticalSlider->value();

    SignProcessor::PDFParams PDF_settings;
    PDF_settings.image_dir = ":/images/MIREA_logo_resize.png";
    PDF_settings.qpdf_dir = qpdf_dir;
    PDF_settings.drawLogo = isDrawGerb();
    PDF_settings.pdf_preset = pdfSignPreset;
    PDF_settings.htmlParams.lineSertificate = labelList.at(LABEL_CERTIFICATE)->text();
    PDF_settings.htmlParams.lineOwner = labelList.at(LABEL_SIGN_HOSTER)->text();
    PDF_settings.htmlParams.lineDate = labelList.at(LABEL_VALID_TIME)->text();

    SignProcessor::CryptoPROParams CryptoPRO_settings;
    CryptoPRO_settings.sign = currentSign;

    ui->closeButton->setDisabled(true);

    // настраиваем обработчик подписей
//    signProscessor = new fileSignProcessor;

//    signProscessor->processor.setWordOptions(word_settings);
//    signProscessor->processor.setPDFOptions(PDF_settings);
//    signProscessor->processor.setCryptoPROOptions(CryptoPRO_settings);
//    signProscessor->processor.setFilesList(listAddedFiles);

    signProscessor = new fileSignProcessor;

    processor.setWordOptions(word_settings);
    processor.setPDFOptions(PDF_settings);
    processor.setCryptoPROOptions(CryptoPRO_settings);
    processor.setFilesList(listAddedFiles);

    signThread = new QThread(this);
    signProscessor->moveToThread(signThread);

    // логика запуска потоков
//    connect(signThread, &QThread::started, signProscessor, &fileSignProcessor::run); // когдазаустился Word, начинаем обработку файлов
//    connect(signProscessor, &fileSignProcessor::ready, signThread, &QThread::quit); // как только поток подписи завершился, отправляем его закрываться
//    connect(signThread, &QThread::finished, this, &MainWindow::threadFinished); // как только поток Word завершился удаляем его
    //взаимодействие с файлами
//    connect(&signProscessor->processor, &SignProcessor::newFileStatus, this, &MainWindow::fileReady); // сигнал о полной обработке файла (заменить на обработчик потока подписи)
//    connect(signProscessor, &fileSignProcessor::newFileStatus, this, &MainWindow::updateFileStatus); // сигнал обновления статуса файла

    connect(&processor, &SignProcessor::newFileStatus, this, &MainWindow::fileReady);
    connect(&processor, &SignProcessor::procesingFinished, this, &MainWindow::threadFinished);


    log.addToLog("Запущена обработка файлов");
    ui->pushButton_addsign->setText("Отмена");

    log.addToLog("Запущен поток word");
//    signThread->start();
    processor.runProcessing();
}

void MainWindow::updateCurrentHostName()
{
    int index = ui->comboBox_signHost->currentIndex();
//    auto currentSign = cryptoObject.getSignBySertificate(presets.getCurrentCertificate()); // получаем текущий сертификаты
    auto currentSign = getCurrentSign(); // получаем текущий сертификат
    labelList.at(LABEL_CERTIFICATE)->setText("Сертификат: " + currentSign.serial);
    switch (index)
    {
        case COMBOBOX_HOSTER_SIGN_NAME:
        {
            labelList.at(LABEL_SIGN_HOSTER)->setText("Владелец: " + currentSign.name);
            break;
        }
        case COMBOBOX_HOSTER_SIGN_SUBJECT:
        {
            labelList.at(LABEL_SIGN_HOSTER)->setText("Владелец: " + currentSign.subname);
            break;
        }
    }
    // Действителен с 01.01.2021 по 31.02.2022
    QString dateStart = getCurrentSign().startDate.toString("dd.MM.yyyy");
    QString dateEnd = getCurrentSign().finishDate.toString("dd.MM.yyyy");
    QString dateDo = "Действителен с " + dateStart + " по " + dateEnd;
    labelList.at(LABEL_VALID_TIME)->setText(dateDo);
}

void MainWindow::setSignsInCombobox(QList<CryptoPRO_CSP::CryptoSignData> list, int nameOrSubjectState, QComboBox *combobox)
{
    if(!combobox)
    {
        return;
    }
    if(list.size() == 0)
    {
        qDebug() << "Нет подписей!";
        return;
    }
//    int index = ui->checkBox_subjectInfo->isChecked();
    QStringList signsData;
    for (auto &&sign : list)
    {
        QString name = sign.name;
        QString subname = sign.subname;
        if(subname == "")
        {
            subname = "- " + name;
        }
        if(name == "")
        {
            name = "-";
        }
        qDebug() << "name = " << name << "subname = " << subname << "nameOrSubjectState = " << nameOrSubjectState;
        switch (nameOrSubjectState)
        {
            case 0:
            {
                signsData.append(name);
                break;
            }
            default:
            {
                signsData.append(subname);
                break;
            }
        }
    }
    if(signsData.size() == 0)
    {
        qDebug() << "signsData = 0";
        return;
    }
//    qDebug() << "signsData = " << signsData;
    combobox->clear(); // очищаем
    combobox->addItems(signsData); // выводим информацию в комбокс

    for (int i=0; i<combobox->count(); i++)
    {
        combobox->setItemData(i, QBrush(QColor(170, 255, 255)), Qt::TextColorRole);
    }
}

void MainWindow::setCertifacateToTablePriview(CryptoPRO_CSP::CryptoSignData &certificate)
{
    auto setText = [=](int row, QString text) // функция вставки текста в ячейку
    {
        QTableWidgetItem *item = ui->tableWidget_certificateInfo->item(row, 0);
        if(item == nullptr)
        {
            item = new QTableWidgetItem;
            ui->tableWidget_certificateInfo->setItem(row, 0, item);
        }
        item->setText(text);
        item->setForeground(QBrush(QColor(170, 255, 255)));
    };

    setText(TABLE_CERTIFICATE_ROW_NAME, certificate.name);
    setText(TABLE_CERTIFICATE_ROW_SERIAL, certificate.serial);
    setText(TABLE_CERTIFICATE_ROW_DATE_START, certificate.startDate.toString("dd.MM.yyyy"));
    setText(TABLE_CERTIFICATE_ROW_DATE_END, certificate.finishDate.toString("dd.MM.yyyy"));
    setText(TABLE_CERTIFICATE_ROW_SUBJECT, certificate.subname);
}

void MainWindow::updatePrewiew()
{
    QPixmap sign_pixmap(ui->sign_widget->size());
    ui->sign_widget->render(&sign_pixmap);
    QImage sign_image = sign_pixmap.toImage();
    ui->previewWidget->setRectImage(sign_image);
    ui->previewWidget->drawSmallRect(ui->horizontalSlider->value(), ui->verticalSlider->value());
}

CryptoPRO_CSP::CryptoSignData MainWindow::getCurrentSign()
{
    int index = ui->comboBox->currentIndex();
    if(index >=0 && sertificatesList.size() > index)
    {
        return sertificatesList.at(index); // получаем подпись по активному индексу комбобокса
    }
    else
    {
        return CryptoPRO_CSP::CryptoSignData();
    }
}

CryptoPRO_CSP::CryptoSignData MainWindow::getSignByIndex(int index)
{
    if(index >=0 && sertificatesList.size() > index)
    {
        return sertificatesList.at(index);
    }
    else
    {
        return CryptoPRO_CSP::CryptoSignData();
    }
}

void MainWindow::setRowColor(QTableWidget *table, int row, QColor color)
{
    if(table) // если указатель на таблицу не пустой
    {
        if(row < table->rowCount() && row >= 0) // если номер строки норпмальный
        {
            for (int i=0; i<table->columnCount(); i++) // проходим по всем столбцам таблицы
            {
                QTableWidgetItem *item = table->item(row, i); // получаем указатель на элемент
                if(!item) // если память не выделена
                {
                    item = new QTableWidgetItem(" "); // выделяем память
                }
                item->setBackground(QBrush(color)); // задаём цвет фона
            }
        }
    }
}

void MainWindow::setFileStatus(QString file_dir, int status)
{
    int row = -1;
    row = getFileIndexByDir(file_dir); // получаем индекс файла в таблице

    if(row == -1)
    {
        qDebug() << "Не удалось найти нужнуый файл в таблице";
        return;
    }
    QString str_status = f_status.getStatusName(status); // поулчаем название статуса // f_status
    QColor itog_color = f_status.getStatusColor(status); // поулчаем цвет статуса

    auto item = ui->tableWidget_filestatus->item(row, 1);
    if(item)
    {
        item->setText(str_status);
    }
    else
    {
        item = new QTableWidgetItem(str_status);
        ui->tableWidget_filestatus->setItem(row, 1, item);
    }
    QString log_text = "Изменён статус файла " + file_dir + str_status;
    log.addToLog(log_text);
//    ui->tableWidget_filestatus->setItem(row, 1, item);
    setRowColor(ui->tableWidget_filestatus, row, itog_color);
    ui->tableWidget_filestatus->scrollToItem(ui->tableWidget_filestatus->item(row, 0));
}

int MainWindow::getFileStatus(QString fileDir)
{
    int row = -1;
    row = getFileIndexByDir(fileDir); // получаем индекс файла в таблице

    if(row == -1)
    {
        qDebug() << "Не удалось найти нужнуый файл в таблице";
        return files_status::no_status;
    }

    QString statusText = ui->tableWidget_filestatus->item(row, 1)->text();
    return f_status.getStatusNumberByName(statusText);
}

void MainWindow::setTableWidgetFiles(QStringList listParamFiles, int status)
{
    int size = listParamFiles.size();
    if(size == 0)
    {
        return;
    }
    ui->tableWidget_filestatus->setRowCount(size);
    filesDirInTable.clear(); // очищаем список с директориями
    for (int i=0; i<size; i++)
    {
        QFile file(listParamFiles.at(i));
        QDir direct = file.fileName();

        filesDirInTable.append(listParamFiles.at(i)); // добавляем директорию в список
        ui->tableWidget_filestatus->setItem(i, 0, new QTableWidgetItem(direct.dirName()));
        setFileStatus(listParamFiles.at(i), status);


    }
    log.addToLog("Обновлен список директорий ", listParamFiles);
}

int MainWindow::getFileIndexByDir(QString file_dir)
{
    int row = -1;
    QStringList list = getAddedFiles();
    int rows = list.size(); // получаем количество строк
    for (int i=0; i<rows; i++)
    {
        if(list.at(i) == file_dir)   // если ToolTip совпадает с нужным (это такой костыль)
        {
            row = i;
            //            qDebug() << "Найдено: " << i;
            break;
        }
    }
    return row;
}

QString MainWindow::getFileDirByIndex(int index)
{
    QStringList list = getAddedFiles();
    int rows = list.size(); // получаем количество строк
    QString fileDir;
    if(index >= 0 && index < rows) // если индекс попадает в диапазон
    {
        fileDir = list.at(index);
    }
    else
    {
        return "";
    }
    return fileDir;
}

void MainWindow::saveProgramData()
{
    if(sertificatesList.size() > 0)
    {
        uisaver.setLastCert(getCurrentSign().serial); // передаём текущий сертификат
    }
    uisaver.saveProgramData();
}

void MainWindow::loadProgramData()
{
    uisaver.loadProgramData();
}

bool MainWindow::ifFileContains(QString file)
{
    if(file.isEmpty()) // если параметр пустой
    {
        log.addToLog("Файла нет в директории");
        return false;
    }
    QFile fileSearch(file);
    return fileSearch.exists(); // возвращаем, есть-ли файл в директории, или нет
}

MainWindow::fileToolTip MainWindow::getFileToolTip(int index)
{
    if(index < 0)
    {
        return fileToolTip();
    }
    auto table = ui->tableWidget_filestatus;
    int cols = table->colorCount();
    if(cols == 0)
    {
        return fileToolTip();
    }
    int rows = table->rowCount();
    if(index >= rows || rows == 0)
    {
        return fileToolTip();
    }
    auto item = table->item(index, 0);
    if(item == nullptr)
    {
        return fileToolTip();
    }
    return fileToolTip(item->toolTip()); // получаем tooltip и возвращаем его
}

MainWindow::fileToolTip MainWindow::getFileToolTip(QString sourceFile)
{
    auto table = ui->tableWidget_filestatus;
    int cols = table->colorCount();
    if(cols == 0)
    {
        return fileToolTip();
    }
    int rows = table->rowCount();
    if(rows == 0)
    {
        return fileToolTip();
    }
    for (int i=0; i<rows; i++)
    {
        fileToolTip toolTip = getFileToolTip(i); // получаем toolTip
        if(toolTip.sourceFile == sourceFile) // если нашли совпадающий
        {
            return toolTip; // возвращаем найденный toolTip
        }
    }
    return fileToolTip(); // если ничего не нашли, то возвращаем пустоту
}

void MainWindow::setFileToolTip(int index, MainWindow::fileToolTip toolTip)
{
    auto table = ui->tableWidget_filestatus;
    int cols = table->colorCount();
    if(cols == 0)
    {
        return;
    }
    int rows = table->rowCount();
    if(index >= rows || rows == 0)
    {
        return;
    }
    auto item = table->item(index, 0);
    if(item == nullptr)
    {
        return;
    }
    item->setToolTip(toolTip.getToolTip()); // устанавыливаем toolTip
}

void MainWindow::setFileToolTip(QString sourceFile, MainWindow::fileToolTip toolTip)
{
    int index = getFileIndexByDir(sourceFile); // получаем индекс файла
    setFileToolTip(index, toolTip); // вызываем установку ToolTip
}

void MainWindow::clearAllOutputsToolTipData()
{
    auto table = ui->tableWidget_filestatus; // получаем указатель на таблицу
    int rows = table->rowCount(); // получам текущее количество строк
    for (int i=0; i<rows; i++)
    {
        fileToolTip fileTip = getFileToolTip(i); // получаем fileToolTip
        fileTip.signedFile.clear(); // очищаем
        fileTip.signedPdfFile.clear();
        setFileToolTip(i, fileTip); // устанавливаем
    }
}

QStringList MainWindow::getAddedFiles()
{
    auto table = ui->tableWidget_filestatus; // получаем указатель на таблицу
    int rows = table->rowCount(); // получам текущее количество строк
    QStringList listFiles;
    for (int i=0; i<rows; i++)
    {
        QString toolTip = table->item(i, 0)->toolTip(); // получаем директорию файла по индексу
        fileToolTip fileTip(toolTip); // создаем структуру по toolTip
        listFiles.append( fileTip.sourceFile ); // добавляем директорию
    }

    return listFiles;
}

void MainWindow::addFile(QString fileName)
{
    if(!fileName.endsWith(".doc") && !fileName.endsWith(".docx") && !fileName.endsWith(".rtf") && !fileName.endsWith(".pdf") && !fileName.endsWith(".xlsx") && !fileName.endsWith(".xls") )
    {
#ifdef DEBUGGING
    qDebug() << "Нельзя добавить файл = " << fileName << " т.к. его расширение не соответствует заданным";
#endif
        return;
    }
    auto table = ui->tableWidget_filestatus; // получаем указатель на таблицу
    int rows = table->rowCount(); // получам текущее количество строк
    table->setRowCount(rows+1); // увеличиваем количество строк на 1
    rows++;

    QFile file(fileName);
    QDir direct = file.fileName();

    QString f_name = direct.dirName(); // чисто имя файла
    log.addToLog("Добавлен новый файл"+f_name);
    auto item = table->item(rows-1, 0);
    if(item == nullptr)
    {
        item = new QTableWidgetItem;
        table->setItem(rows-1, 0, item);
    }
    item->setText(f_name);
//    table->setItem(rows-1, 0, new QTableWidgetItem(f_name)); // в таблицу добавляем название файла
    fileToolTip fileTip; // создаём структуру для toolTip
    fileTip.sourceFile = fileName; // передаём файлу путь к исходнику
    //    table->item(rows-1, 0)->setToolTip(fileName); // заносим директорию к файлу в ToolTip
    table->item(rows-1, 0)->setToolTip( fileTip.getToolTip() ); // заносим директорию к файлу в ToolTip
    setFileStatus(fileName, files_status::added);

#ifdef DEBUGGING
    qDebug() << "ToolTip = " << table->item(rows-1, 0)->toolTip();
#endif
}

void MainWindow::addFiles(QStringList files)
{
    for (auto &&file : files)
    {
        addFile(file);
    }
}

void MainWindow::removeFile(QString fileDir)
{
    int index = getFileIndexByDir(fileDir); // получаем номер строки с файлом
    ui->tableWidget_filestatus->removeRow(index);  // удаляем строку
    log.addToLog("Удален файл "+fileDir);
}

void MainWindow::removeFiles(QStringList files)
{
    for (auto &&file : files)
    {
        removeFile(file);
    }
}

QStringList MainWindow::getSelectedFiles()
{
    auto table = ui->tableWidget_filestatus; // получаем указатель на таблицу
    int rows = table->rowCount(); // получам текущее количество строк
    int cols = table->columnCount(); // получаем количество столбцов
    QStringList list; // список файлов
    for (int i=0; i<rows; i++)
    {
        bool selected = false; // флаг, что строка выбрана
        for (int j=0; j<cols; j++)
        {
            auto item = table->item(i, j);
            if(item)
            {
                if(item->isSelected()) // если хотя бы один item выбран
                {
                    selected = true;
                }
            }
            else
            {
                qDebug() << "item пустой " << i << j;
            }
        }
        if(selected) // если в строке что-то выбрано
        {
            QString file = getFileDirByIndex(i); // получаем директорию файла по индексу
            list.append(file); // Добавляем файл в список
        }
    }
    return list;
}

void MainWindow::deleteSelectedFiles()
{
    QStringList selectedFiles = getSelectedFiles(); // получаем список выбранных файлов
    removeFiles(selectedFiles); // удаляем файлы
}

int MainWindow::getSignFileStatus(QString fileDir)
{
    QString status = getSignFileStatusName(fileDir); // получаем название статуса
    return f_status.getStatusNumberByName(status); // возвращаем номер статуса
}

QString MainWindow::getSignFileStatusName(QString fileDir)
{
    int row = getFileIndexByDir(fileDir); // получаем номер строки, в которой лежит файл
    auto item = ui->tableWidget_filestatus->item(row, 1); // получаем указатель на ячейку
    QString text = "";
    if(item)
    {
        text = item->text(); // получаем текст статуса
    }
    return text;
}

QStringList MainWindow::getSignedFiles()
{
    auto table = ui->tableWidget_filestatus; // получаем указатель на таблицу
    int rows = table->rowCount(); // получам текущее количество строк
    QStringList list; // список файлов
    QStringList acceptStatusList; // список успешных статусов
    acceptStatusList.append(f_status.getStatusName(files_status::no_errors)); // успешно добавлено
    acceptStatusList.append(f_status.getStatusName(files_status::error_new_page_added)); // добавлено, но произошёл переход на новую страницу

    for (int i=0; i<rows; i++)
    {
        QString fileDir = getFileDirByIndex(i); // получаем директорию файла
        QString status = getSignFileStatusName(fileDir); // получаем название статуса
        if(acceptStatusList.contains(status)) // если список успешных статусов содержит статус выбранного файла
        {
            list.append(fileDir);
        }
    }
    return list;
}

void MainWindow::runFile(QString file)
{
    QString program;
    QStringList args;
    if(QSysInfo::productType()=="windows")
    {
        program = "cmd";
        //        file = "/C start " + file + ""; // добавляем префикс для запуска из командной строки
    }
    args << "/C" << "start" << file;
    //    args = file.split(" ", Qt::SplitBehavior(Qt::SkipEmptyParts));
    qDebug() << "command: " + program << args;
    QProcess t_proc; // создаём процесс
    t_proc.startDetached(program, args); // запускаем файл в отдельном независящем потоке
    log.addToLog("Запуск командной строки");
}

void MainWindow::runWordFile(QString wordFile)
{
    log.addToLog("Открытие файла в Word " + wordFile);
    QProcess *process = new QProcess(this);
    connect(process, SIGNAL(finished(int,QProcess::ExitStatus)), this, SLOT(processFinished(int,QProcess::ExitStatus)));
    QString file; // путь к ворду
    //    file = "C:/Program Files/Microsoft Office/Office16/WINWORD.EXE"; // путь к ворду
    file = ui->lineEdit_wordDirectory->text();
    if(file.isEmpty())
    {
        QMessageBox::warning(this, "Ошибка", "Не указан путь к файлу Word!");
        log.addToLog("Ошибка! Не указан путь к файлу Word!");
        return;
    }
    if(!ifFileContains(file)) // Если файл не найден в директории (ворд был перемещён или директорию удалили)
    {
        QMessageBox::warning(this, "Ошибка", "Word по установленной директории не найден. Выберите другую директорию");
        log.addToLog("Ошибка! Word по установленной директории не найден. Выберите другую директорию");
        return;
    }

    file = "\"" + file + "\"";
    wordFile = "\"" + wordFile + "\"";

    QFile temp_file(QDir::currentPath() + TEMP_BAT_FILE);
    temp_file.open(QIODevice::WriteOnly);
    QString temp_text = "start " + file + " " + wordFile;
    temp_file.write(QString::fromUtf8(temp_text.toUtf8()).toLocal8Bit().data());
    temp_file.close();

    log.addToLog("Запускаем Word: " + wordFile);

    process->setProgram(temp_file.fileName());
    process->startDetached();
}

bool MainWindow::findQpdf(QString &qpdf_dir)
{
    QString searchDir = QDir::currentPath() + "/qpdf/qpdf.exe";
//    QMessageBox::information(this, "", searchDir);
    if(QFile::exists(searchDir))   // если файл найден в стандартной директории
    {
        qpdf_dir = searchDir;
        ui->lineEdit_qpdfexe_dir->setText(qpdf_dir);
        return true;
    }
    else
    {
        return false;
    }
}

void MainWindow::loadWordDirectories()
{
    QStringList directories;
    // Word 2016
    directories.append("C:/Program Files/Microsoft Office/Office16/WINWORD.EXE");
    directories.append("C:/Program Files (x86)/Microsoft Office/Office16/WINWORD.EXE");
    // Word 2019
    directories.append("C:/Program Files/Microsoft Office/root/Office16/WINWORD.EXE");
    directories.append("C:/Program Files (x86)/Microsoft Office/root/Office16/WINWORD.EXE");

    // C:\Program Files\Microsoft Office\root\Office16

    bool exist_flag = false;
    for (auto &&wordDir : directories )
    {
        if (ifFileContains(wordDir))
        {
            ui->lineEdit_wordDirectory->setText(wordDir);
            log.addToLog("Word найден в " + wordDir);
            saveProgramData();
            exist_flag = true;
            break;
        }
    }
    if(!exist_flag)
    {
        log.addToLog("Word не найден в стандартных директориях");
    }
}

bool MainWindow::isDrawGerb()
{
    return ui->comboBox_choseImageGerb->currentIndex() == 0; // если 0, значит рисуем герб
}

void MainWindow::setScale(double scalefactor)
{    
    QString groupStyleshhet;
    for (auto &&label  : labelList)
    {
        QFont font = label->font();
        auto size = font.pointSizeF();
        if(scale != 1.0) // если текущий масштаб не равен 1
        {
            size *= scale; // пересчитываем масштаб в нормальный (т.е. 100%)
        }
        bool isBold = font.bold();
        font.setPointSizeF(size / scalefactor); // ставим новый размер
        double fsize = size / scalefactor;
        QString styleSheet = "QLabel#" + label->objectName()
                + "{\
                    font: " + QString::number(fsize) + "pt \"MS Shell Dlg 2\";\
                    color: rgb(0, 0, 0);";
        if (isBold)
        {
            styleSheet.append("font-weight: bold;");
        }
        styleSheet.append("}");
        groupStyleshhet.append(styleSheet + "\r\n");
    }
    ui->groupBox_10->setStyleSheet(groupStyleshhet);
    scale = scalefactor; // сохраняем текущий масштаб
    qDebug() << "Изменен масштаб на " << scalefactor;
}

void MainWindow::reshowSidebar()
{
    if(ui->scrollArea_settings->property("hided").toInt() == 1)
    {
        ui->scrollArea_settings->show();
        ui->scrollArea_settings->setProperty("hided", 0);
    }
    else
    {
        ui->scrollArea_settings->hide();
        ui->scrollArea_settings->setProperty("hided", 1);
    }
}

void MainWindow::filesTableMouseRightClick(QTableWidgetItem *item)
{
    QMenu menu;

    // добавление файлов и папок
    QAction *menuAddFileAction = menu.addAction("Добавить файл");
    QAction *menuAddFolderAction = menu.addAction("Добавить папку");
    QObject::connect(menuAddFileAction, &QAction::triggered, this, [this]() { on_pushButton_addFile_clicked(); });
    QObject::connect(menuAddFolderAction, &QAction::triggered, this, [this]() { on_addFolderPushButton_clicked(); });

    // добавлем кнопку удалить выбранный файл
    QString filename;
    if(item != nullptr)
    {
        filename = item->text();
        QAction *menuDeleteFile = menu.addAction("Удалить " + filename);
        QObject::connect(menuDeleteFile, &QAction::triggered, this, [=]() { removeFile(getFileDirByIndex(item->row())); }); // удаление файла
    }

    if(getSelectedFiles().size() > 0) // если хотя бы 1 файл выделен мышкой
    {
        QAction *menuDeleteSelectedAction = menu.addAction("Удалить выбранные");
        QObject::connect(menuDeleteSelectedAction, &QAction::triggered, this, [this]() { on_pushButton_removeFile_clicked(); });
    }
    if(getSelectedFiles().size() < ui->tableWidget_filestatus->rowCount()) // если хотя бы 1 файл ещё не выделен мышкой
    {
        QAction *menuReSelectedAllAction = menu.addAction("Выбрать все");
        QObject::connect(menuReSelectedAllAction, &QAction::triggered, this, [this]() { ui->tableWidget_filestatus->setSelectedForAllItems(true); });
    }
    if(getSelectedFiles().size() > 0) // если хотя бы 1 файл выделен мышкой
    {
        QAction *menuReSelectedAllAction = menu.addAction("Снять выделение со всех");
        QObject::connect(menuReSelectedAllAction, &QAction::triggered, this, [this]() { ui->tableWidget_filestatus->setSelectedForAllItems(false); });
    }
    if(getSignedFiles().size() > 0) // если хотя бы один файл имеет подпись
    {
        QAction *menuDeleteSignedAction = menu.addAction("Удалить подписанные");
        QObject::connect(menuDeleteSignedAction, &QAction::triggered, this, [this]() { on_pushButton_removeSignedFiles_clicked(); });
    }
    if(item != nullptr)
    {
        fileToolTip fileData = getFileToolTip(item->row()); // получаем tooltip файла

        QAction *menuOpenFileAction = menu.addAction("Открыть " + QFileInfo(fileData.sourceFile).fileName());
        QObject::connect(menuOpenFileAction, &QAction::triggered, this, [this, fileData]() { runWordFile(fileData.sourceFile); });

        if(fileData.signedFile != "")
        {
            QAction *menuOpenFileAction = menu.addAction("Открыть подписанный WORD файл");
            QObject::connect(menuOpenFileAction, &QAction::triggered, this, [this, fileData]() { runWordFile(fileData.signedFile); });
        }
        if(fileData.signedPdfFile != "")
        {
            QAction *menuOpenFileAction = menu.addAction("Открыть подписанный PDF файл");
            QObject::connect(menuOpenFileAction, &QAction::triggered, this, [this, fileData]() { runWordFile(fileData.signedPdfFile); });
        }
    }

    menu.exec(QCursor::pos());
}

void MainWindow::filesTableMouseDoubleClick(QTableWidgetItem *item)
{
    if(item != nullptr)
    {
        on_tableWidget_filestatus_itemDoubleClicked(item);
    }
}

void MainWindow::setPresetsToComboBox(QComboBox *combobox)
{
    if(combobox)
    {
        combobox->clear();
        combobox->addItems(presets.getPresetsFilesList()); // записываем все пресеты
    }
}

void MainWindow::setArgs(const QStringList &value)
{
    args = value;
}

void MainWindow::addNewPreset(QString presetName)
{
    if(presets.getPresetsFilesList().contains(presetName))
    {
        return;
    }
    presets.addNewPreset(presetName); // Добавляем пресет
    setPresetsToComboBox(ui->comboBox_changePreset);
    setCurrentPreset(presetName); // устанавливаем выбранным (добавленный) пресет
}

void MainWindow::deletePreset(QString presetName)
{
    int index = presets.getPresetIndexByName(presetName); // получаем номер прсета
    if(index >= 0)
    {
        presets.deletePreset(index);
        setPresetsToComboBox(ui->comboBox_changePreset);

        if(index < presets.getPresetsFilesList().size())
        {
            ui->comboBox_changePreset->setCurrentIndex(index); // ставим следующий индекс
        }
        else if (presets.getPresetsFilesList().size() > 0)
        {
             ui->comboBox_changePreset->setCurrentIndex(index-1); // ставим следующий индекс
        }
    }
}

void MainWindow::setCurrentPreset(QString presetName)
{
    int index = presets.getPresetIndexByName(presetName);
    ui->comboBox_changePreset->setCurrentIndex(index);
}


void MainWindow::fileReady(SignProcessor::FileForSign file, int status)
{
    Q_UNUSED(status);
    //    Q_UNUSED(outputFiles);
#ifdef DEBUGGING
    qDebug() << "fileReady = " << file.sourceFile;
#endif
//    int file_status = status; // получаем статус файла
//    if(file_status == files_status::no_status || file_status == files_status::image_added)
//    {
//        setFileStatus(fileDir, files_status::no_errors);
//    }
//    countFilesReady++;
//    fileToolTip fileTip = getFileToolTip(fileDir); // получаем fileToolTip полученного файла
//    for (auto &&file : outputFiles)
//    {
//        if(file.endsWith(".pdf")) // если файл заканчивается на PDF
//        {
//            fileTip.signedPdfFile = file; // устанавливаем директорию PDF файла
//        }
//        else
//        {
//            fileTip.signedFile = file; // устанавливаем директорию подписанного файла
//        }
//    }
    setFileStatus(file.sourceFile, status);
    if(file.isSigned)
    {
        fileToolTip fileTip = getFileToolTip(file.sourceFile);
        fileTip.signedFile = file.signWordFile;
        fileTip.signedPdfFile = file.signPDFFile;
        setFileToolTip(file.sourceFile, fileTip); // устанавливаем fileToolTip
    }
//    log.addToLog("Завершена обработка файла " + fileDir);
    //    QApplication::processEvents();
}

void MainWindow::threadFinished()
{
    log.addToLog("Завершён поток Word'а");
#ifdef DEBUGGING
    qDebug() << "Завершён поток Word'а";
#endif


#ifdef DEBUGGING
    qDebug() << "Установлен parent для cryptoObject - this";
#endif
    delete signProscessor;
    signProscessor = nullptr;

#ifdef DEBUGGING
    qDebug() << "Удален signProscessor";
#endif
    delete signThread;
    signThread = nullptr;
#ifdef DEBUGGING
    qDebug() << "Завершен поток подписи";
#endif

    log.addToLog("Завершен поток подписи");

    countFilesReady = 0;

    // заменяем статусы необработанных файлов
    int rows = ui->tableWidget_filestatus->rowCount();
    for (int i=0; i<rows; i++)
    {
        int status = f_status.getStatusNumberByName(ui->tableWidget_filestatus->item(i, 1)->text()); // получаем номер статуса
        if(status == files_status::waiting) // Если статус "в оченреди"
        {
            setFileStatus(getFileDirByIndex(i), files_status::added); // устанавливаем статус добавлен всем файлам, которые оказались в процессе
        }
    }

    ui->pushButton_addsign->setText("Добавить подпись");
    if(!isClosing && !wordCancel)
    {
        QMessageBox::information(this, "", "Готово");
    }
    ui->closeButton->setDisabled(false);

}

void MainWindow::updateFileStatus(QString fileDir, int status)
{
    setFileStatus(fileDir, status);
}

void MainWindow::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
#ifdef DEBUGGING
    qDebug() << "Процесс завершился. Код: " << exitCode << "Статус: " << exitStatus;
#else
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);
#endif
    sender()->deleteLater();
    QFile temp_file(TEMP_BAT_FILE);
    temp_file.remove(); // удаляем временный bat-ник
    log.addToLog("Процесс завершен. Удален временный bat файл");
}

void MainWindow::signProcessFinished()
{

}

void MainWindow::cryptoSignListReady(QList<CryptoPRO_CSP::CryptoSignData> list)
{
    log.addToLog("В MainWindow получен список подписей");
//    Q_UNUSED(list);
    sertificatesList = list; // сохряняем список
    ui->pushButton_addsign->setEnabled(true);
    ui->pushButton_addsign->setText("Добавить подпись");
#ifdef DEBUGGING
    qDebug() << "Обновляем comboBox";
#endif
//    presets.updateComboBox();
#ifdef DEBUGGING
    qDebug() << "Загружаем данные программы";
#endif
    setSignsInCombobox(list, ui->checkBox_subjectInfo->isChecked(), ui->comboBox); // устанавливаем данные в комбобокс
    setSignsInCombobox(list, ui->checkBox_subjectInfo->isChecked(), ui->comboBox_viewCertificate); // устанавливаем данные в комбобокс
    presets.searchPresets(); // ищем пресеты
    setPresetsToComboBox(ui->comboBox_changePreset);
    loadProgramData();
    if(ui->comboBox_changePreset->count() > 0)
    {
        presets.applyPreset(ui->comboBox->currentText()); // применяем шаблон
    }
    QString defaultCert = uisaver.getDefaultCert(); // получаем дефолтный сертификат
    QString currentCert; // выбранный сертификат, который мы установим
    if(defaultCert != "-" && defaultCert != "")
    {
        currentCert = defaultCert;
    }
    else
    {
        currentCert = uisaver.getLastCert(); // получаем последний сертификат
    }
    if(currentCert != "" && currentCert != "-")
    {
//        CryptoCMD::CryptoSignData certifacate = cryptoObject.getSignBySertificate(currentCert); // получаем подпись по сертификату
//        if(certifacate.serial != "") // если подпись не пустая
//        {
//            int certIndex = cryptoObject.getSignNumber(certifacate); // получаем номер подписи
//            ui->comboBox->setCurrentIndex(certIndex);
//        }
        for(int i=0; i<sertificatesList.size(); i++)
        {
            if(sertificatesList.at(i).serial == currentCert) // если совпадает с искомым
            {
                ui->comboBox->setCurrentIndex(i);   // устанавливем его в выбранный
                break;
            }
        }
    }
//    updateCurrentHostName();    // обновляем данные на форме
    //    customConstructor(); // после полкчения данных вызываем кастомный конструктор
    // ищем директории
#ifdef DEBUGGING
    qDebug() << "Ищем директории";
#endif
    QString tempFilePath = ui->lineEdit_tempFilesDirectory->text();
    if(tempFilePath.isEmpty()) // если загруженная директория пустая
    {
        QString currentPath = QDir::currentPath(); // получаем текущую директорию
        if(currentPath.contains(" "))
        {
            // Точно ли нужна проверка на пробелы? они загружают ее с рабочего
            // стола, и вроде работает
            // Предлагаю заменить простой строчкой в лог
            QMessageBox::critical(this, "Внимание!", "Программа загружена из директории, в пути которой есть пробелы!\nДля корректной работы программы необходима директория\nдля хранения временных файлов.\nПожалуйста выберите директорию для хранения временных файлов,\nне содержащую пробелы.");
            log.addToLog("Попытка загрузки программы из директории с пробелами");
            return;
        }
        else
        {
#ifdef DEBUGGING
            qDebug() << "сохраняем";
#endif
            ui->lineEdit_tempFilesDirectory->setText(currentPath); // устанавливаем директорию временных файлов текущей
            saveProgramData();
        }
    }
#ifdef DEBUGGING
    qDebug() << "Загружаем ворд из директории";
#endif
    loadWordDirectories(); // загружаем ворд из директории

    if(args.count() > 0)
    {
        if(args.at(0) == "-signfile" && args.count() >= 2)
        {
            addFile(args.at(1)); // добавляем файл в таблицу
            qDebug() << "Добавили файл " << args.at(1);
            if(args.count() >= 3) // дополнительный аргумент
            {
                if(args.at(2) == "addsign")
                {
                    ui->pushButton_addsign->click(); // вызываем функцию клика
                }
            }
        }
        else
        {
            qDebug() << "Не удалось определить команду запуска!" << args;
        }
    }
    else
    {
        qDebug() << "Недостаточно аргументов запуска!" << args;
    }
    log.addToLog("Завершен слот cryptoSignListReady");
}

void MainWindow::on_pushButton_addFile_clicked()
{
    // директория, где находится файл
    QStringList fileDiretory = QFileDialog::getOpenFileNames(this,tr("OpenFile"),tr("downloads"),tr(FILES_EXTENSIONS)); // делаем выборку сразу нескольких файлов
    if(fileDiretory.size() == 0) // если не было выбрано ни одного файла
    {
        //        QMessageBox::information(this, "Внимание", "Вы не выбрали файл");
        return;
    }

    QStringList noAddedFiles; // список файлов, которые не будут добавлены
    QStringList addedFiles = getAddedFiles(); // поулчает список уже добавленных файлов
    for(int f = 0; f<fileDiretory.size(); f++) // проходим по всем файлам, которые мы получили
    {
        for (int i=0; i<addedFiles.size(); i++) // проходим по всем элементам ListWidget
        {
            auto item = addedFiles.at(i);
            if(fileDiretory.at(f) == item) // если элемент совпадает с уже существующим
            {
                noAddedFiles.append(fileDiretory.at(f)); // добавляем в список не добавленных
                fileDiretory.removeAt(f); // удаляем его из списка
            }
        }
    }

    addFiles(fileDiretory); // добавляем файлы в таблицу

    if(noAddedFiles.size() > 0)
    {
        QString text;
        text.append("Один или несколько файлов уже добавлены в список для подписи:\n");
        for (auto &&fileDir : noAddedFiles)
        {
            text.append("\n" + fileDir);
        }
        text.append("\n\nПовторное добавление невозможно.");
        QMessageBox::warning(this, "Внимание!", text);
        log.addToLog(text);
    }
}

void MainWindow::on_pushButton_removeFile_clicked()
{
    deleteSelectedFiles();
}

void MainWindow::on_tableWidget_filestatus_itemDoubleClicked(QTableWidgetItem *item)
{
#ifdef OPEN_WORD_BY_DOUBLECLICK
    QString fileForRun = getFileDirByIndex(item->row());
#ifdef DEBUGGING
    qDebug() << "Открываемый файл: " << fileForRun;
#endif
    fileToolTip fileTip = getFileToolTip( fileForRun ); // получаем toolTip по названию файла
#ifdef DEBUGGING
    qDebug() << "toolTip = " << fileTip.getToolTip();
#endif
    if(ui->radioButton_runSourceFile->isChecked())
    {
#ifdef DEBUGGING
        qDebug() << "Запуск исходного файла";
#endif
        fileForRun = fileTip.sourceFile;
        log.addToLog("Открытие исходного файла"+fileForRun);
    }

    else if(ui->radioButton_runSignedFile->isChecked())

    {
#ifdef DEBUGGING
        qDebug() << "Запуск подписанного файла";
#endif
        fileForRun = fileTip.signedFile;
        log.addToLog("Открытие подписаного файла"+fileForRun);
        if(fileForRun.isEmpty())
        {
            QMessageBox::warning(this, "Ошибка", "Для данного файла отсутствует подписанный WORD файл!");
            return;
        }
    }
    else if(ui->radioButton_runSignedFilePDF->isChecked())
    {
#ifdef DEBUGGING
        qDebug() << "Запуск PDF файла";
#endif
        fileForRun = fileTip.signedPdfFile;
        log.addToLog("Открытие подписаного pdf"+fileForRun);

        if(fileForRun.isEmpty())
        {
            QMessageBox::warning(this, "Ошибка", "Для данного файла отсутствует подписанная PDF версия!");
            return;
        }
    }
    if(!fileForRun.isEmpty())
    {
        runWordFile(fileForRun);
    }
    else
    {
#ifdef DEBUGGING
        qDebug() << "Путь открываемого файла пустой!";
#endif
        log.addToLog("Не удалось запустить файл - путь пустой");
    }
#else
    Q_UNUSED(item);
#endif
}


void MainWindow::on_pushButton_7_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    "C:/Program Files/Microsoft Office/WINWORD.EXE",
                                                    tr("WINWORD (*.exe)"));
    if(!fileName.isEmpty())
    {
        ui->lineEdit_wordDirectory->setText(fileName);
        saveProgramData();
    }
}

void MainWindow::on_pushButton_8_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(!dir.isEmpty())
    {
        if(dir.contains(" "))
        {
            ui->lineEdit_tempFilesDirectory->clear();
            QMessageBox::warning(this, "Ошибка", "Директория хранения временных файлов не должна содержать пробелы!");
        }
        else
        {
            ui->lineEdit_tempFilesDirectory->setText(dir);
            saveProgramData();
        }

    }
}

void MainWindow::on_pushButton_9_clicked()
{
    QMessageBox::information(this, "Информация", PROGRAM_INFORMATION);
}


void MainWindow::on_pushButton_5_clicked()
{

}



void MainWindow::on_advancedSettingsButton_clicked()
{
    log.addToLog("Попытка доступа в файл настроек");
    QString password = QInputDialog::getText(this,"Введите пароль", "", QLineEdit::Password);

    if (password == SETTINGS_PASSWORD)
    {
        ui->stackedWidget->setCurrentIndex(STACKED_SYSTEM_SETTINGS);
        ui->advancedSettingsButton->hide();
    }
    else if (password == "")
    {
        log.addToLog("Введён пустой пароль");
    }
    else
    {
        QMessageBox::information(this,"Ошибка","Неверный пароль");
        log.addToLog("Неверный пароль");
    }
}

void MainWindow::on_pushButton_chose_files_clicked()
{
    ui->stackedWidget->setCurrentIndex(STACKED_FILES);
}

void MainWindow::on_pushButton_edit_preview_clicked()
{
    ui->stackedWidget->setCurrentIndex(STACKED_EDIT_PREVIEW);
}

void MainWindow::on_pushButton_chose_shablon_clicked()
{
    ui->stackedWidget->setCurrentIndex(STACKED_SHABLONI);
}

void MainWindow::on_pushButton_back_to_menu_clicked()
{
    ui->advancedSettingsButton->show();
    ui->stackedWidget->setCurrentIndex(STACKED_PREVIEW);
}

void MainWindow::on_closeButton_clicked()
{
    if (QMessageBox::question(this,"Выход","Закрыть программу?", QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes)==QMessageBox::Yes)
    {
        log.addToLog("Инициируем закрытие программы");
        close();
    }
}

void MainWindow::on_pushButton_removeSignedFiles_clicked()
{
    QStringList signedFiles = getSignedFiles(); // получаем список файлов, которые были подписаны
    log.addToLog("Подписаны файлы",signedFiles);
    removeFiles(signedFiles); // удаляем их списка
}

void MainWindow::on_checkBox_signingOut_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    saveProgramData();
}

void MainWindow::on_checkBox_createSubFolder_stateChanged(int arg1)
{
    QString currentOutputPath = ui->lineEdit_userOutputPath->text(); // текущая выбранная директория
    if(currentOutputPath.isEmpty()) // если путь не введён
    {
        return;
    }
    if(arg1)
    {
        if(!currentOutputPath.contains(SUBFOLDER_NAME))
        {
            currentOutputPath.append(SUBFOLDER_NAME);
        }
    }
    else
    {
        currentOutputPath.remove(SUBFOLDER_NAME);
    }
    ui->lineEdit_userOutputPath->setText(currentOutputPath);
}

void MainWindow::on_pushButton_selectOutputPath_clicked()
{
    // открываем диалог выбора файла
    QString dir = QFileDialog::getExistingDirectory(this, tr("Open Directory"),
                                                    "/",
                                                    QFileDialog::ShowDirsOnly
                                                    | QFileDialog::DontResolveSymlinks);
    if(!dir.isEmpty())
    {
        dir.append("/");
        if(ui->checkBox_createSubFolder->isChecked())
        {
            dir.append(SUBFOLDER_NAME);
        }
        ui->lineEdit_userOutputPath->setText(dir);
    }
}

void MainWindow::on_checkBox_outputInSelectedPath_stateChanged(int arg1)
{
    //    qDebug() << "Состояние установлено на " << arg1;
    ui->checkBox_createSubFolder->setEnabled(arg1);
    ui->label_outputPath->setEnabled(arg1);
    ui->lineEdit_userOutputPath->setEnabled(arg1);
    ui->pushButton_selectOutputPath->setEnabled(arg1);
}

void MainWindow::paintEvent(QPaintEvent *)
{
#ifdef SHOW_CENTRAL_GEOMERTY
    qDebug() << "Высота: " << this->height() << "Ширина: " << this->width();
#endif
#ifdef SHOW_TABLEFILES_GEOMERTY
    qDebug() << ui->tableWidget_filestatus->height() << ui->tableWidget_filestatus->width();
#endif
    if(ui->tableWidget_filestatus->columnCount() >= 2)
    {
        int sizeMax = ui->tableWidget_filestatus->width();
        int sizeMin = sizeMax/100;
        int size60 = sizeMin*60;
        int size40 = sizeMax - size60 - 2;
        ui->tableWidget_filestatus->setColumnWidth(0, size60);
        ui->tableWidget_filestatus->setColumnWidth(1, size40);
    }
}

void MainWindow::on_comboBox_choseImageGerb_currentIndexChanged(const QString &arg1)
{
    m_widg->setCurrentImage(arg1);
}

MainWindow::fileToolTip::fileToolTip(QString sourceFile, QString signedFile, QString signedPdfFile)
{
    this->sourceFile = sourceFile;
    this->signedFile = signedFile;
    this->signedPdfFile = signedPdfFile;
}

MainWindow::fileToolTip::fileToolTip(QString toolTip)
{
    if(!toolTip.isEmpty())
    {
        QStringList toolTipList = toolTip.split("|", SPLITTER); // разбиваем на блоки
        for (auto &&tip : toolTipList)
        {
            if(tip.contains("=")) // если в блоке есть разделитель
            {
                QStringList list = tip.split("="); // разбиваем на блоки по =
                if(list.at(0) == "sourceFile") // если значение слева от = равно
                {
                    sourceFile = list.at(1); // записываем значение
                }
                else if(list.at(0) == "signedFile") // если значение слева от = равно
                {
                    signedFile = list.at(1); // записываем значение
                }
                else if(list.at(0) == "signedPdfFile") // если значение слева от = равно
                {
                    signedPdfFile = list.at(1); // записываем значение
                }
            }
        }
    }
}

QString MainWindow::fileToolTip::getToolTip()
{
    return "sourceFile=" + sourceFile + "|" + "signedFile=" + signedFile + "|" + "signedPdfFile=" + signedPdfFile;
}

void MainWindow::on_pushButton_moveToSigning_clicked()
{
    if(signThread) // если поток уже запущен
    {
//        signProscessor->setClosing(true); // Отправляем, что нужно прервать
        log.addToLog("Прерывание обработки word");
        wordCancel = true;
        return;
    }
    else
    {
        ui->stackedWidget->setCurrentIndex(STACKED_SIGN_SETTINGS); // переходим к подписи
    }
}

void MainWindow::on_pushButton_backToChoseFiles_clicked()
{
    ui->stackedWidget->setCurrentIndex(STACKED_FILES); // переходим назад к выбору файлов
}

void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    updateCurrentHostName();
    ui->comboBox_viewCertificate->setCurrentIndex(index);
}

void MainWindow::on_comboBox_signHost_currentIndexChanged(int index)
{
    Q_UNUSED(index);
    updateCurrentHostName();
}

void MainWindow::on_checkBox_subjectInfo_stateChanged(int arg1)
{
    Q_UNUSED(arg1);
    setSignsInCombobox(sertificatesList, arg1, ui->comboBox);
    setSignsInCombobox(sertificatesList, arg1, ui->comboBox_viewCertificate);
}

void MainWindow::on_pushButton_chose_qpdfexe_dir_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),
                                                    QDir::currentPath() + "/qpdf/bin/qpdf.exe",
                                                    tr("qpdf (*.exe)"));
    if(!fileName.isEmpty())
    {
        ui->lineEdit_qpdfexe_dir->setText(fileName);
        saveProgramData();
    }
}

void MainWindow::on_addFolderPushButton_clicked()
{
    QString folder = QFileDialog::getExistingDirectory(this,"Выберите папку","C:/",QFileDialog::ShowDirsOnly);
    if(folder == ""){return;}
    QDirIterator foldIter(folder,{"*.rtf","*.doc","*.docx","*.pdf"},QDir::Files);
    QStringList addedFiles = getAddedFiles();
    while (foldIter.hasNext())
    {
        QString file = foldIter.next();
        bool allowAdding = true;
        for (int i = 0; i<addedFiles.length();i++)
        {
            if(file == addedFiles.at(i))
            {
                allowAdding = false;
                break;
            }
        }
        if(allowAdding)
        {
            addFile(file);
        }
    }
}

void MainWindow::on_doubleSpinBox_scalekoefficient_valueChanged(double arg1)
{
    setScale(arg1);
}

void MainWindow::on_paramButton_clicked()
{
    ui->stackedWidget->setCurrentIndex(STACKED_SIGN_POSITION_PREVIEW);
    updatePrewiew();
    ui->previewWidget->rescale();
    ui->verticalSlider->setMaximum(ui->previewWidget->getDrawLinesCount()); // ставим максимум в число строк на превью
}

void MainWindow::on_verticalSlider_valueChanged(int value)
{
    ui->previewWidget->drawSmallRect(ui->horizontalSlider->value(),value);
    ui->previewWidget->update();
}

void MainWindow::on_horizontalSlider_valueChanged(int value)
{
    ui->previewWidget->drawSmallRect(value, ui->verticalSlider->value());
    ui->previewWidget->update();
}

void MainWindow::on_pushButton_backToExportParams_clicked()
{
    ui->stackedWidget->setCurrentIndex(STACKED_SIGN_SETTINGS);
}


void MainWindow::on_radioButton_usually_insert_clicked(bool checked)
{
    ui->checkBox_exportWordToPDF->setEnabled(checked);
    ui->checkBox_signWordDocument->setEnabled(checked);
    ui->checkBox_exportWordToPDF->setChecked(checkBox_exportWordToPDF_oldState);
    ui->checkBox_signWordDocument->setChecked(checkBox_signWordDocument_oldState);
}

void MainWindow::on_radioButton_insert_in_exported_pdf_clicked(bool checked)
{
    ui->checkBox_exportWordToPDF->setEnabled(!checked);
    ui->checkBox_signWordDocument->setEnabled(!checked);
    checkBox_exportWordToPDF_oldState = ui->checkBox_exportWordToPDF->isChecked();
    checkBox_signWordDocument_oldState = ui->checkBox_signWordDocument->isChecked();
    ui->checkBox_exportWordToPDF->setChecked(!checked);
    ui->checkBox_signWordDocument->setChecked(!checked);
}



void MainWindow::on_pushButton_settings_shower_clicked()
{
    reshowSidebar();
}

void MainWindow::on_stackedWidget_currentChanged(int arg1)
{
    if(arg1 == STACKED_SIGN_POSITION_PREVIEW || arg1 == STACKED_SIGN_SETTINGS)
    {
        updatePrewiew();
        ui->previewWidget->rescale();
        ui->verticalSlider->setMaximum(ui->previewWidget->getDrawLinesCount()); // ставим максимум в число строк на превью
    }
}

void MainWindow::on_comboBox_viewCertificate_currentIndexChanged(int index)
{
    CryptoPRO_CSP::CryptoSignData selectedSign = getSignByIndex(index); // получаем подпись по индексу
    setCertifacateToTablePriview(selectedSign); // выводим информацию в таблицу
}

void MainWindow::on_pushButton_acceptCertifacte_clicked()
{
    int index = ui->comboBox_viewCertificate->currentIndex(); // получаем номер выбранного сертификата
    ui->comboBox->setCurrentIndex(index); // устанавливаем такой-же индекс комбоксу на превью главного меню
}

void MainWindow::on_pushButton_makeDefaultCertifacate_clicked()
{
    if(sertificatesList.size() > 0)
    {
        QString serial = getSignByIndex(ui->comboBox_viewCertificate->currentIndex()).serial;
        uisaver.setDefaultCert(serial); // записываем дефолтный
        qDebug() << "Установлен дефолтный сертьификат " << uisaver.getDefaultCert();
        saveProgramData();
    }
}

void MainWindow::on_pushButton_createNewPreset_clicked()
{
    QString presetName = "Новый пресет 1";
    QStringList presetsFilesList = presets.getPresetsFilesList(); // получаем список пресетов
    for (int i=1; presetsFilesList.contains(presetName); i++)
    {
        presetName = "Новый пресет " + QString::number(i);
    }
    addNewPreset(presetName);
    ui->lineEdit_presetName->setText(presetName);
}

void MainWindow::on_pushButton_updatePresetData_clicked()
{
    if (QMessageBox::question(this,"Обновить данные шаблона","Обновить данные шаблона?", QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes)==QMessageBox::Yes)
    {
        QComboBox *combobox_preset = ui->comboBox_changePreset;
        QString lastname = combobox_preset->currentText(); // получаем старое название
        QString nextname = ui->lineEdit_presetName->text(); // получаем новое название
        int index = combobox_preset->currentIndex(); // поулчаем текущий индекс
        presets.updatePreset(lastname, nextname); // обновляем данные пресета
        setPresetsToComboBox(combobox_preset); // выводим новые данные в combobox
        combobox_preset->setCurrentIndex(index); // выставляем снова последний выбранный индекс
    }
}

void MainWindow::on_comboBox_changePreset_currentIndexChanged(const QString &arg1)
{
    ui->lineEdit_presetName->setText(arg1);
}

void MainWindow::on_pushButton_deletePreset_clicked()
{
    if (QMessageBox::question(this,"Удаление шаблона","Удалить выбранный шаблон?", QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes)==QMessageBox::Yes)
    {
        deletePreset(ui->comboBox_changePreset->currentText()); // удаляем пресет
    }
}

void MainWindow::on_pushButton_acceptCurrentPreset_clicked()
{
    presets.applyPreset(ui->comboBox_changePreset->currentText()); // примняем выбранный шаблон
}

