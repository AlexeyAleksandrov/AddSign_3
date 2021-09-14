#ifndef FILES_STATUS_H
#define FILES_STATUS_H

#define COLOR_RED   QColor(255, 105, 105)
#define COLOR_GREEN QColor(158, 255, 105)
#define COLOR_GREY  QColor(190, 190, 190)
#define COLOR_YELLOW QColor(255, 255, 95)
#define COLOR_ORANGE QColor(255, 188, 155)
#define COLOR_WHITE Qt::white
#define COLOR_PURPLE QColor(85, 85, 255)
#define COLOR_FULL_RED QColor(255, 50, 50)

#include <QMap>
#include <QString>
#include <QColor>

#define TEMP_DIR "/temp"

class files_status
{
public:
    files_status();

    enum status // список возможных статусов
    {
        no_errors,
        error_no_open,
        error_new_page_added,
        error_new_page_no_added,
        error_pdf_no_export,
        error_file_signature_failed,
        error_password_failed,
        error_qpdf_no_image_added,
        error_no_tabels,
        error_no_tags,
        in_process,
        waiting,
        added,
        image_added,
        no_signed,
        no_supported,
        no_status
    };

    enum signStatus
    {
        signReady,
        signError
    };

    QString getStatusName(int st); // возвращает название статуса
    QColor getStatusColor(int st); // возвращает название статуса
    int getStatusNumberByName(QString name); // возвращет номер статуса по его названию


private:
    QMap<int, QString> status_name; // мап для хранения названий статусов
    QMap<int, QColor> status_color; // мап для хранения цветов статусов
};

#endif // FILES_STATUS_H
