#ifndef UIPRESET_H
#define UIPRESET_H

#include "uidatasaver.h"

class UiPreset : public UiDataSaver
{
public:
    UiPreset();

    void addNewPreset(QString presetName); // добавить шаблон
    void deletePreset(QString presetName); // удалить шаблон
    void deletePreset(int presetIndex); // удалить шаблон
    void applyPreset(QString presetName); // применить шаблон
    void updatePreset(QString lastPresetName, QString newPresetName); // обновить данные шаблона

    void searchPresets(); // поиск файлов пресетов
    QString getPresetFileNameByIndex(int index); // получить путь к файлу пресета по индексу в списке/combobox
    int getPresetIndexByName(QString presetName); // получить индекс в списке/combobox по названиюы

    QStringList getPresetsFilesList() const;

private:
    QStringList presetsFilesList; // список файлов пресетов
    const QString presetsFolder = "/presets/"; // папка, в которой храняться пресеты
    void renamePreset(QString lastname, QString nextName); // переименовать пресет

    void loadPresetFromFile(QString presetName); // загрузить и применить пресет из файла
    void savePresetToFile(QString presetName); // сохранить пресет в файл
};

#endif // UIPRESET_H
