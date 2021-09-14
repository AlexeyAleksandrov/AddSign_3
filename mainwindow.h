#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
//#include <my_widget.h>
//#include "wordeditor.h"
//#include "wordparams.h"
#include <QThread>
#include <QTableWidget>
#include <QDate>
#include <QProcess>
//#include <QUrl>
#include <QApplication>
#include <QCloseEvent>
//#include <preset.h>
#include <QTextCodec>
#include "logclass.h"
#include "files_status.h"
#include "uidatasaver.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QDebug>
#include <QInputDialog>
#include <QPainter>
#include <QPaintEvent>
#include "m_widget.h"
//#include <filesignprocessor.h>
#include <imagetransformer.h>
#include "uipreset.h"
#include <signprocessor.h>

//#define DEBUGGING
//#define INSERT_IN_CENTER

//#define SHOW_CENTRAL_GEOMERTY
//#define SHOW_TABLEFILES_GEOMERTY

#define COMBOBOX_HOSTER_SIGN_NAME 0
#define COMBOBOX_HOSTER_SIGN_SUBJECT 1


QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void customConstructor(); // кастомный конструктор


    Ui::MainWindow *ui;

    void setArgs(const QStringList &value);

private slots:

    void on_pushButton_addsign_clicked();

//    void on_pushButton_3_clicked();

    void on_pushButton_addFile_clicked();

    void on_pushButton_removeFile_clicked();

private:
    // =================================================================================
    // ==== БЛОК ОБРАБОТКИ WORD =====
    // =================================================================================
//    WordEditor *word = nullptr; // редактор Word, каждый раз создаётся новый, для перемещения в поток
//    QThread *wordThread = nullptr; // поток для обработки Word
//    QThread *signThread = nullptr; // поток для обработки подписей
    bool SigningInProcess = false;
//    int countFilesReady = 0;
    QString imageDir; // директория, где лежит файл картинки, полученной из виджета
    // =================================================================================
    // ==== БЛОК СТАТУСОВ ФАЙЛОВ =====
    // =================================================================================
    files_status f_status; // обработчик статусов файлов
    void setRowColor(QTableWidget *table, int row, QColor color); // устанавливает цвет строки в таблице
    void setFileStatus(QString file_dir, int status); // устанавливает статус
    int getFileStatus(QString fileDir); // получает статус файла
    QString getDesktopDirectory(); // получить директорию рабочего стола
    // =================================================================================
    // ==== БЛОК ИНТЕРФЕЙСА =====
    // =================================================================================
    QList<QLabel*> labelList;
    QList<QLineEdit*> lineEditList;

    QStringList filesDirInTable; // список, хранящий директории файлов в таблице
    int getFileIndexByDir(QString filedir); // возвращет номер файла в таблице, по его пути
    QString getFileDirByIndex(int index); // возвращет путь к файлу, по его индексу

    UiDataSaver uisaver; // сохранение и загрузка Данных об интерфейсе из файла
    void saveProgramData(); // сохранить данные о настройках программы
    void loadProgramData(); // загрузить данные о настройках программы
    void loadWordDirectories(); // выполняет поиск Word по директориям

    bool isDrawGerb(); // возращает, рисовать-ли герб
    // =================================================================================
    // ==== БЛОК ОБРАБОТКИ ФАЙЛОВ ====
    // =================================================================================
    bool isClosing = false; // флаг того, что программа закрывается и нужно всё сворачивать
    bool closeOnEnd = false;
//    bool wordCancel = false;

    SignProcessor processor;

    bool ifFileContains(QString file); // проверяет, существет-ли файл в директории
    void setTableWidgetFiles(QStringList listParamFiles, int status = files_status::waiting); // устанавливает файлы в таблице

    struct fileToolTip
    {
        QString sourceFile;
        QString signedFile;
        QString signedPdfFile;
        fileToolTip() {};
        fileToolTip(QString sourceFile, QString signedFile, QString signedPdfFile);
        fileToolTip(QString toolTip);
        QString getToolTip();

    };
    fileToolTip getFileToolTip(int index); // возвращает fileToolTip по индексу
    fileToolTip getFileToolTip(QString sourceFile); // возвращает fileToolTip по исходному файлу
    void setFileToolTip(int index, fileToolTip toolTip); // устанавливает fileToolTip по индексу в таблице
    void setFileToolTip(QString sourceFile, fileToolTip toolTip); // устанавливает fileToolTip по названию файла в таблице
    void clearAllOutputsToolTipData(); // очищает данные о выводе у всех файлов, которые нужно подписать (нужно, когда один и тот же файл подписывают несколько раз и чтобы не было некорректных запусков)
    QStringList getAddedFiles(); // возвращает список добаленных файлов для редактирования
    void addFile(QString fileName); // добавляет файл в таблицу для редактирования
    void addFiles(QStringList files); // добавляет список файлов в таблицу для редактирования
    void removeFile(QString fileDir); // удаляет файл из таблицы
    void removeFiles(QStringList files); // удаляет список файлов из таблицы
    QStringList getSelectedFiles(); // получает список выбранных файлов
    void deleteSelectedFiles(); // удалить выбранные в таблице файлы
    int getSignFileStatus(QString fileDir); // возвращет индекс статуса файла по его директории
    QString getSignFileStatusName(QString fileDir); // возвращает название статуса файла по его директории
    QStringList getSignedFiles(); // получить список подписанных файлов
    // =================================================================================
    // ==== БЛОК ПРЕВЬЮ ====
    // =================================================================================
    m_widget *m_widg;
//    void textUpdate(QString *errList = nullptr);
//    preset presets; // пресеты
    void updateCurrentHostName(); // обновляет информацию на превью в засисимости от того, какой тип сейчас выбран
    void setSignsInCombobox(QList<CryptoPRO_CSP::CryptoSignData> list, int nameOrSubjectState, QComboBox *combobox); // заполняет comboBox подписями
    void setCertifacateToTablePriview(CryptoPRO_CSP::CryptoSignData &certificate); // устанавливает сертификат в таблицу превью
private slots:
    void updatePrewiew(); // слот обноления превью
private:
    // =================================================================================
    // ==== БЛОК КРИПТЫ ====
    // =================================================================================
//    fileSignProcessor *signProscessor = nullptr;
//    CryptoCMD cryptoObject;
    CryptoPRO_CSP CryptoPRO;
    QList<CryptoPRO_CSP::CryptoSignData> sertificatesList; // список сертификатов
    CryptoPRO_CSP::CryptoSignData getCurrentSign(); // получить текущую подпись
    CryptoPRO_CSP::CryptoSignData getSignByIndex(int index); // возвращает сертификат по номеру
    // =================================================================================
    // ==== БЛОК ЗАПУСКА ФАЙЛОВ ====
    // =================================================================================
    void runFile(QString file); // запускает файл из под винды (DEMO)
//    void runWordFile(QString wordFile); // запускает Word файл
    // =================================================================================
    logClass log; // лог
    bool findQpdf(QString &qpdf_dir);
    // =================================================================================
    double scale = 1.0;
    void setScale(double scalefactor); // меняет масштаб текста в виджете превью
    bool checkBox_exportWordToPDF_oldState = false; // сохраняет предыдущее состояние
    bool checkBox_signWordDocument_oldState = false;
    void reshowSidebar(); // показыванет/скрывает боковое меню
    // контекстное меню
private slots:
    void filesTableMouseRightClick(QTableWidgetItem *item);
    void filesTableMouseDoubleClick(QTableWidgetItem *item);
private:
    // =================================================================================
    // ==== БЛОК ПРЕСЕТОВ ====
    // =================================================================================

    UiPreset presets; // загрузчик пресетов

    void addNewPreset(QString presetName); // добавить шаблон
    void deletePreset(QString presetName); // удалить шаблон
    void setCurrentPreset(QString presetName); // уставновить выбранный пресет
    void setPresetsToComboBox(QComboBox *combobox); // вывод пресетов в combobox

    // =================================================================================
    // ==== БЛОК ПРЕСЕТОВ ====
    // =================================================================================
    QStringList args; // список аргументов запуска

    // =================================================================================
    // ==== БЛОК АВТОМАТИЧЕСКОГО ТЕСТИРОВАНИЯ ====
    // =================================================================================
    enum automationTest_types
    {
        simple_insert_in_word,
        insert_in_word_with_next_page,
        insert_in_word_with_export_pdf,
        insert_in_word_with_export_pdf_next_page,
        standart_insert_in_pdf,
        standart_insert_in_pdf_with_next_page,
        standart_insert_in_excel,
        standart_insert_in_excel_with_next_page,
        insert_in_coords_word,
        insert_in_coords_excel,
        insert_in_coords_pdf,
        insert_by_tag,
        size
    };
    struct automationTest_example_files
    {
        struct file
        {
            QString standart;
            QString full;
        };

        struct word : public file
        {
            QString with_tag;
        };

        word docx;
        word doc;
        word rtf;

        file pdf;
        file xls;
        file xlsc;
    };

    struct automationTest_ui_settings
    {
        int insert_type = SignProcessor::insert_standart;
        bool ignoreMovingToNextPage = false;
        bool exportToPdf = false;
        bool signWordFile = false;
    };

    automationTest_example_files automationTesting_exapmleFiles;    // добавить инициализацию файлов

    QVector<QStringList> automationTest_sourceFiles;    // массив списка файлов, которые необходимо использовать для каждого из типа тестирования
    QVector<int> automationTest_chosedTests;    // массив выбранных пользователем типов тестирования
    QVector<automationTest_ui_settings> automationTest_settings;    // список настроек для каждого из видов тестирования

    bool isAutomationTesting = false; // происходит-ли сейчас автоматическое тестирование программы
    int current_automationTest_type = -1;
    void automationTest_runTest_step(int step);    // запуск этапа автоматического тестирования
    void automationTest_step_finished();    // этап автоматического тестирования завершён
private slots:
    void on_pushButton_automationTest_clicked();    // слот обработки кнопки автотеста



    // =================================================================================


private slots:
    void fileReady(SignProcessor::FileForSign file, int status);
//    void threadFinished();
//    void updateFileStatus(QString fileDir, int status);
//    void processFinished(int exitCode, QProcess::ExitStatus exitStatus);
//    void signProcessFinished(); // слот завершения потока подписи
    void cryptoSignListReady(QList<CryptoPRO_CSP::CryptoSignData> list); // слот, получающий информацию о том, что список подписей загружен
//    void setCurrentSignByComboBox(); // устанавливает шаблон по комбо-боксу. Слот нужен для вызова из файла класса загрузки

//    void signProcessReady();

    void on_tableWidget_filestatus_itemDoubleClicked(QTableWidgetItem *item);
    void on_pushButton_7_clicked();
    void on_pushButton_8_clicked();

    void on_pushButton_9_clicked();

    void paintEvent(QPaintEvent *);

    void closeEvent(QCloseEvent *event)
    {
        if(SigningInProcess)
        {
            if (QMessageBox::question(this,"Выход","Внимание! Идёт процесс подписи, принудительное закрытие программы может привести к повреждению файлов. Используйте принудительное закрытие только в том случае, если программа зависла\nЗакрыть программу принудительно?", QMessageBox::Yes|QMessageBox::No,QMessageBox::Yes)==QMessageBox::Yes)
            {
                log.addToLog("Принудительно закрываем программу");
                event->accept();
//                close();
            }
            else
            {
                event->ignore();
            }
        }
    }
//    void on_pushButton_clicked();
//    void on_pushButton_4_clicked();
//    void on_pushButton_5_clicked();
//    void on_pushButton_6_clicked();
    void on_advancedSettingsButton_clicked();
//    void on_pushButton_chose_files_clicked();
//    void on_pushButton_edit_preview_clicked();
//    void on_pushButton_chose_shablon_clicked();
//    void on_pushButton_add_new_shablon_clicked();
    void on_pushButton_back_to_menu_clicked();
    void on_closeButton_clicked();
    void on_pushButton_removeSignedFiles_clicked();
    void on_checkBox_signingOut_stateChanged(int arg1);
    void on_checkBox_createSubFolder_stateChanged(int arg1);
    void on_pushButton_selectOutputPath_clicked();
    void on_checkBox_outputInSelectedPath_stateChanged(int arg1);

//    void paintEvent(QPaintEvent *e);
//    void on_pushButton_2_clicked();
    void on_comboBox_choseImageGerb_currentIndexChanged(const QString &arg1);
//    void on_comboBox_activated(int index);
//    void on_pushButton_moveToSigning_clicked();
//    void on_pushButton_backToChoseFiles_clicked();
    void on_comboBox_currentIndexChanged(int index);
    void on_comboBox_signHost_currentIndexChanged(int index);
//    void on_comboBox_viewPresetInfo_currentIndexChanged(int index);
    void on_checkBox_subjectInfo_stateChanged(int arg1);
    void on_pushButton_chose_qpdfexe_dir_clicked();
    void on_addFolderPushButton_clicked();
//    void on_pushButton_apply_scale_clicked();
    void on_doubleSpinBox_scalekoefficient_valueChanged(double arg1);
//    void on_paramButton_clicked();
    void on_verticalSlider_valueChanged(int value);
    void on_horizontalSlider_valueChanged(int value);
//    void on_horizontalSlider_sliderMoved(int position);
//    void on_comboBox_2_activated(const QString &arg1);
//    void on_comboBox_linesInterval_currentIndexChanged(const QString &arg1);
//    void on_pushButton_backToExportParams_clicked();
//    void on_verticalSlider_sliderMoved(int position);
    void on_radioButton_usually_insert_clicked(bool checked);
    void on_radioButton_insert_in_exported_pdf_clicked(bool checked);
//    void on_toolButton_triggered(QAction *arg1);
    void on_pushButton_settings_shower_clicked();
    void on_stackedWidget_currentChanged(int arg1);
//    void on_comboBox_viewCertificate_activated(int index);
    void on_comboBox_viewCertificate_currentIndexChanged(int index);
    void on_pushButton_acceptCertifacte_clicked();
    void on_pushButton_makeDefaultCertifacate_clicked();
    void on_pushButton_createNewPreset_clicked();
    void on_pushButton_updatePresetData_clicked();
    void on_comboBox_changePreset_currentIndexChanged(const QString &arg1);
    void on_pushButton_deletePreset_clicked();
    void on_pushButton_acceptCurrentPreset_clicked();
//    void on_comboBox_choseImageGerb_currentIndexChanged(int index);
    void on_pushButton_moveToSigning_clicked();
//    void on_radioButton_signByTag_clicked();
    void on_radioButton_signByTag_clicked(bool checked);
};
#endif // MAINWINDOW_H
