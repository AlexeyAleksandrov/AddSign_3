#include "uipreset.h"

#define presetfile(presetName) presetsFolder + presetName + ".ini"
#define absolutePresetfile(presetName) QDir::currentPath() + presetsFolder + presetName + ".ini"


UiPreset::UiPreset()
{

}

void UiPreset::addNewPreset(QString presetName)
{
    if(presetsFilesList.contains(presetName))
    {
        return;
    }
    presetsFilesList.append(presetName);
    QDir().mkdir(QDir::currentPath() + presetsFolder); // создаём директорию
    setSaveFile(presetfile(presetName));
    saveProgramData();
}

void UiPreset::deletePreset(QString presetName)
{
    int index = getPresetIndexByName(presetName); // получаем номер прсета
    deletePreset(index);
}

void UiPreset::deletePreset(int presetIndex)
{
    if(presetIndex >= 0)
    {
        QString presetName = getPresetFileNameByIndex(presetIndex); // получаем название
        presetsFilesList.removeAt(presetIndex); // удаляем из списка
        QFile file(absolutePresetfile(presetName));
        qDebug() << "Файл " << file.fileName();
        if(file.exists())
        {
            qDebug() << "Удаляем " << file.fileName();
            file.remove();
        }
    }
}

void UiPreset::applyPreset(QString presetName)
{
    setSaveFile(presetfile(presetName));
    loadProgramData();
}

void UiPreset::updatePreset(QString lastPresetName, QString newPresetName)
{
    if(lastPresetName != newPresetName) // если названия разные
    {
        renamePreset(lastPresetName, newPresetName); // переименовываем
    }
    setSaveFile(presetfile(newPresetName));
    saveProgramData();
}

void UiPreset::searchPresets()
{
    presetsFilesList.clear();
    QDirIterator ligIter(QDir::currentPath() + presetsFolder,{"*.ini"},QDir::Files);
    while (ligIter.hasNext())
    {
        QFile presetsFile(ligIter.next()); // получаем файл
        QString presetsFileName = QFileInfo(presetsFile).baseName(); // получаем имя файла без расширения - оно же будет названием прсетеа
        presetsFilesList.append(presetsFileName); // добавляем пресет в список
    }
}

QString UiPreset::getPresetFileNameByIndex(int index)
{
    if(index >= 0 && index < presetsFilesList.size())
    {
        return presetsFilesList.at(index); // вызвращаем название пресета по индексу
    }
    else
    {
        return "";
    }
}

int UiPreset::getPresetIndexByName(QString presetName)
{
    for (int i=0; i<presetsFilesList.size(); i++)
    {
        if(presetsFilesList.at(i) == presetName)
        {
            return i;
        }
    }
    return -1;
}

QStringList UiPreset::getPresetsFilesList() const
{
    return presetsFilesList;
}

void UiPreset::renamePreset(QString lastname, QString nextName)
{
    QFile lastFile(QDir::currentPath() + presetfile(lastname));
    QFile nextFile(QDir::currentPath() + presetfile(nextName));
    if(!lastFile.exists() || nextFile.exists()) // если старог офайла нет, или новый уже существует
    {
        return;
    }
    if(!lastFile.open(QIODevice::ReadOnly) || !nextFile.open(QIODevice::WriteOnly)) // если файлы не открываются
    {
        return;
    }
    nextFile.write(lastFile.readAll()); // записываем
    lastFile.close();
    nextFile.close();
    lastFile.remove();
    presetsFilesList.replace(getPresetIndexByName(lastname), nextName); // обновляем данные в списке
}

void UiPreset::loadPresetFromFile(QString presetName)
{
    QFile file(QDir::currentPath() + presetfile(presetName));
    if(!file.exists())
    {
        #ifdef DEBUGGING
        qDebug() << "Файл с пресетом не найден = " << presetName;
        #endif
    return;
    }
    setSaveFile(presetfile(presetName)); // устанавливаем файл поиска
    loadProgramData(); // загружаем данные из файла
}

void UiPreset::savePresetToFile(QString presetName)
{
    setSaveFile(presetfile(presetName)); // устанавливаем файл сохранения
    saveProgramData(); // загружаем данные из файла
}
