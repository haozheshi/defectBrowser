QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    FileWin32.cpp \
    imdb.cpp \
    mdb.c \
    midl.c \
    PictureDelegate.cpp \
    batchloader.cpp \
    bookscrollbar.cpp \
    comboboxex.cpp \
    defectbrowser.cpp \
    defectfilterbook.cpp \
    defectpagepage.cpp \
    main.cpp \
    pageitempicture.cpp \
    pcie.cpp \
    queryresult.cpp \
    sqlite3.c \
    statistictable.cpp

HEADERS += \
    File.h \
    imdb.h \
    lmdb++.h \
    lmdb.h \
    midl.h \
    PictureDelegate.h \
    batchloader.h \
    bookscrollbar.h \
    comboboxex.h \
    defectbrowser.h \
    defectfilterbook.h \
    defectpagepage.h \
    pageitempicture.h \
    pcie.h \
    queryresult.h \
    sqlite3.h \
    statistictable.h

FORMS += \
    batchloader.ui \
    defectbrowser.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

RESOURCES += \
    icon.qrc
CONFIG(debug, debug|release){
    DESTDIR =$$_PRO_FILE_PWD_/bin/debug
}else{
    DESTDIR =$$_PRO_FILE_PWD_/bin/release
}
