#include "files_status.h"

#include "files_status.h"

files_status::files_status() // инициализация в конструкторе
{
    status_name.insert(status::no_errors, "Завершено");
    status_name.insert(status::error_no_open, "Не удалось открыть файл");
    status_name.insert(status::error_new_page_added, "Переход на новую страницу. Подпись добавлена");
    status_name.insert(status::error_new_page_no_added, "Переход на новую страницу. Подпись не добавлена");
    status_name.insert(status::in_process, "В процессе");
    status_name.insert(status::waiting, "В очереди");
    status_name.insert(status::added, "Добавлен");
    status_name.insert(status::no_status, "Без статуса");
    status_name.insert(status::error_pdf_no_export, "Не удалось создать PDF");
    status_name.insert(status::image_added, "Картинка добавлена");
    status_name.insert(status::no_signed, "Не удалось создать подпись");
    status_name.insert(status::error_file_signature_failed, "Файл повреждён");
    status_name.insert(status::error_password_failed, "Пароль неверный");
    status_name.insert(status::error_qpdf_no_image_added, "Ошибка QPDF");


    status_color.insert(status::no_errors, COLOR_GREEN);
    status_color.insert(status::error_no_open, COLOR_RED);
    status_color.insert(status::error_new_page_added, COLOR_ORANGE);
    status_color.insert(status::error_new_page_no_added, COLOR_RED);
    status_color.insert(status::in_process, COLOR_YELLOW);
    status_color.insert(status::waiting, COLOR_GREY);
    status_color.insert(status::added, COLOR_WHITE);
    status_color.insert(status::no_status, COLOR_WHITE);
    status_color.insert(status::error_pdf_no_export, COLOR_RED);
    status_color.insert(status::image_added, COLOR_PURPLE);
    status_color.insert(status::no_signed, COLOR_RED);
    status_color.insert(status::error_file_signature_failed, COLOR_FULL_RED);
    status_color.insert(status::error_password_failed, COLOR_RED);
    status_color.insert(status::error_qpdf_no_image_added, COLOR_RED);
}

QString files_status::getStatusName(int st)
{
    return status_name[st]; // вызвращаем название по индексу
}

QColor files_status::getStatusColor(int st)
{
    return status_color[st]; // вызвращаем цвет по индексу
}

int files_status::getStatusNumberByName(QString name)
{
    for(int i=0; i<status::no_status; i++) // перебериаем все статусы
    {
        if(status_name[i] == name) // если название совпаданиет, возвращаем
        {
            return i;
        }
    }
    return status::no_status;
}
