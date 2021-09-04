QT       += core gui printsupport network
win32: QT += axcontainer

#DEFINES += QT_NO_DEBUG_OUTPUT

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    cryptopro_csp.cpp \
    exeleditor.cpp \
    files_status.cpp \
    imagetransformer.cpp \
    m_widget.cpp \
    main.cpp \
    mainwindow.cpp \
    my_tablewidget.cpp \
    pdfcreator.cpp \
    pdftopngconverter.cpp \
    qpdf_cmd.cpp \
    signprewiew.cpp \
    signprocessor.cpp \
    systemwin32.cpp \
    uidatasaver.cpp \
    uipreset.cpp \
    wordeditor.cpp \
    logclass.cpp

HEADERS += \
    cryptopro_csp.h \
    exeleditor.h \
    files_status.h \
    imagetransformer.h \
    m_widget.h \
    mainwindow.h \
    my_tablewidget.h \
    pdfcreator.h \
    pdftopngconverter.h \
    qpdf_cmd.h \
    signprewiew.h \
    signprocessor.h \
    systemwin32.h \
    uidatasaver.h \
    uipreset.h \
    wordeditor.h \
    logclass.h

FORMS += \
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
