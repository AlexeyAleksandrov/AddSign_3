QT       += core gui printsupport network
win32: QT += axcontainer

#DEFINES += QT_NO_DEBUG_OUTPUT

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    libs/documnetsigncreator.cpp \
    libs\cryptopro_csp.cpp \
    dialogsearchsertificate.cpp \
    old_libs\exeleditor.cpp \
    libs\files_status.cpp \
    libs\imagetransformer.cpp \
    old_libs\libpoi.cpp \
    old_libs\libreoffice.cpp \
    m_widget.cpp \
    main.cpp \
    mainwindow.cpp \
    my_tablewidget.cpp \
    old_libs\pdfcreator.cpp \
    old_libs\pdftopngconverter.cpp \
    old_libs\qpdf_cmd.cpp \
    libs\signprewiew.cpp \
    old_libs\signprocessor.cpp \
    libs\systemwin32.cpp \
    libs\uidatasaver.cpp \
    libs\uipreset.cpp \
    old_libs\wordeditor.cpp \
    libs\logclass.cpp


HEADERS += \
    libs/documnetsigncreator.h \
    libs\cryptopro_csp.h \
    dialogsearchsertificate.h \
    old_libs\exeleditor.h \
    libs\files_status.h \
    libs\imagetransformer.h \
    old_libs\libpoi.h \
    old_libs\libreoffice.h \
    m_widget.h \
    mainwindow.h \
    my_tablewidget.h \
    old_libs\pdfcreator.h \
    old_libs\pdftopngconverter.h \
    old_libs\qpdf_cmd.h \
    libs\signprewiew.h \
    old_libs\signprocessor.h \
    libs\systemwin32.h \
    libs\uidatasaver.h \
    libs\uipreset.h \
    old_libs\wordeditor.h \
    libs\logclass.h

FORMS += \
    dialogsearchsertificate.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    source.qrc

RC_FILE += icon.rc
#RC_ICONS += mirea_icon.ico
#ICON = MIREA_logo.ico
#RC_ICONS = mirea_icon.ico
