#ifndef UIDATASAVER_H
#define UIDATASAVER_H

#include <QString>
#include <QStringList>
#include <QList>
#include <QFile>
#include <QDebug>
#include "logclass.h"
#include <QCheckBox>
#include <QLineEdit>
#include <QComboBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QMainWindow>
#include <QRadioButton>

#define NEW_SPLIT
#ifdef NEW_SPLIT
#if QT_VERSION >= 0x050f00 // версия Qt 5.15.2
    #define SPLITTER Qt::SplitBehavior(Qt::SkipEmptyParts)
#else
    #define SPLITTER QString::SkipEmptyParts
#endif
#endif

//#define DEBUGGING

class UiDataSaver : public QObject
{
    Q_OBJECT
public:
    explicit UiDataSaver(QObject *parent = nullptr);
    ~UiDataSaver();

    bool forsedDefaultFlag; // Флаг, указывающий, установил ли пользователь значение по умолчанию
    bool defaultFlag;

    void saveProgramData(); // сохранить данные о настройках программы
    void loadProgramData(); // загрузить данные о настройках программы

    void add(QCheckBox *checkBox); // добавляет checkBox в список
    void add(QLineEdit *lineEdit); // добавляет checkBox в список
    void add(QComboBox *comboBox); // добавляет checkBox в список
    void add(QSpinBox *spinBox); // добавляет checkBox в список
    void add(QDoubleSpinBox *doubleSpinBox); // добавляет checkBox в список
    void add(QSlider *slider);
    void add(QMainWindow *mainWindow);
    void addRb(QRadioButton *radioButton);

//    preset *getPres() const;
//    void setPres(preset *value);

    void setDefaultCert(const QString &value);
    void setLastCert(const QString &value);

    QString getDefaultCert() const;
    QString getLastCert() const;

    void setSaveFile(const QString &value);

signals:
    //    void acceptCurrentIndexSignComboBox(); // сигнал о том, что форме надо применить подпись по текущему выбранному состоянию из comboBox

private:
    logClass log;

    // === СПИСКИ ВИДЖЕТОВ ===
    QList<QCheckBox*> checkBoxesList; // список checkBox, состояние которых нужно сохранить
    QList<QLineEdit*> lineEditList; // список lineEdit, состояние которых нужно сохранить
    QList<QComboBox*> comboBoxList; // список comboBox, состояние которых нужно сохранить
    QList<QSpinBox*> spinBoxList; // список comboBox, состояние которых нужно сохранить
    QList<QDoubleSpinBox*> doubleSpinBoxList; // список comboBox, состояние которых нужно сохранить
    QList<QSlider*> sliderList;
    QList<QMainWindow*> mainWindowList;
    QList<QRadioButton*> radioButtonList;

//    preset *pres = nullptr;
    QString defaultCert;
    QString lastCert;

    QString saveFile = "/settings.ini"; // файл, где находятся сохранения

    // =======================

    // параметры для структуры чтения/записи в файл
    const QString stringSeparator = "|";
    const QString keyDataSeparator = "=";
    const QString lineDataSeparator = ":";


    // === СВОЙСТВА ОБЪЕКТА ===
    struct objectProperty
    {
        const QString propertySeparator = "&";
        const QString propertyInfoSeparator = "#";
        void addProperty(QString propertyName, QString value); // добавить свойтсво
        QString getPropertySaveString(); // получить строку свойств
        void setPropertyList(QString readedObjectState); // создать список свойств по считанной строке свойств
        QString getPropertyInfo(QString propertyName); // получить значение свойства

        private:
        QMap<QString, QString> propertyList; // мап для свойств
    };

    // === БЛОК ОБРАБОТКИ СОХРАНЕНИЯ ДАННЫХ ===
    // шаблонная функция для записи состояния списка виджетов в строку
    template<typename T> // функция сделана шаблонной, чтобы быть универсальной
    QString createSaveString(QString widgetType, QList <T> &list, QString (*stateFunc)(T)); // возвращает строку, сформированную из списка виджетов, обработанных переданной функцией

    // функции обработки состояния конкретного виджета. Виджет, переданный в функцию, будет обработан этой функцией
    // если нужно будет добавить обработку нового типа виджета, сделать такую-же функция, заменив LineEdit На свой виджет
    static QString getLineEditState(QLineEdit *lineEdit); // сохранение статуса lineEdit
    static QString getCheckBoxState(QCheckBox *checkBox); // сохранение статуса checkBox
    static QString getComboBoxState(QComboBox *comboBox); // сохранение статуса comboBox
    static QString getSpinBoxState(QSpinBox *spinBox); // сохранение статуса spinBox
    static QString getDoubleSpinBoxState(QDoubleSpinBox *doubleSpinBox); // сохранение статуса doubleSpinBox
    static QString getSliderState(QSlider *slider); // сохранение статуса slider
    static QString getMainWindowState(QMainWindow *mainWindow); // сохранение статуса MainWindow
    static QString getRadioButtonState(QRadioButton *radioButton); // сохранение статуса radioButton

    // =========================================

    // === БЛОК ОБРАБОТКИ ЗАГРУЗКИ ДАННЫХ ИЗ ФАЙЛА ===
    template<typename T> // функция сделана шаблонной, чтобы быть универсальной
    void loadFromSaveString(QString savedString, QList<T> &list, void (*setStateFunc)(T, QString state)); // берёт сохранённую в файле строку и обрабатывает переданный ей список виджетов функцией setStateFunc

    // функции обработки установления состояния конкретного виджета. Виджет, переданный в функцию, будет обработан этой функцией
    // если нужно будет добавить обработку нового типа виджета, сделать такую-же функция, заменив LineEdit на свой виджет
    static void setLineEditState(QLineEdit *lineEdit, QString state); // установка состояния lineEdit на основе данных из файла
    static void setCheckBoxState(QCheckBox *checkBox, QString state); // установка состояния checkBox на основе данных из файла
    static void setComboBoxState(QComboBox *comboBox, QString state); // установка состояния comboBox на основе данных из файла
    static void setSpinBoxState(QSpinBox *spinBox, QString state); // установка состояния spinBox на основе данных из файла
    static void setDoubleSpinBoxState(QDoubleSpinBox *doubleSpinBox, QString state); // установка состояния doubleSpinBox на основе данных из файла
    static void setSliderState(QSlider *slider, QString state); // установка состояния slider на основе данных из файла
    static void setMainWindowState(QMainWindow *mainWindow, QString state); // установка состояния MainWindow на основе данных из файла
    static void setRadioButtonState(QRadioButton *radioButton, QString state); // установка состояния radioButton на основе данных из файла


    // =========================================

};

#endif // UIDATASAVER_H
