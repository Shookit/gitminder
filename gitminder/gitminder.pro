#-------------------------------------------------
#
# Project created by QtCreator 2013-02-09T17:12:23
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = gitminder
TEMPLATE = app

SOURCES += main.cpp\
        mainwindow.cpp \
    notifytimer.cpp \
    filewatcher.cpp \
    git.cpp

HEADERS  += mainwindow.h \
    notifytimer.h \
    filewatcher.h \
    git.h

FORMS    += mainwindow.ui

OTHER_FILES += \

RESOURCES += \
    resource.qrc

RC_FILE = gitminder.rc

win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libgit2/windows/lib/ -lgit2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libgit2/windows/lib/ -lgit2
else:unix: LIBS += -L$$PWD/libgit2/linux_64/lib/ -lgit2

INCLUDEPATH += $$PWD/libgit2/windows/include
DEPENDPATH += $$PWD/libgit2/windows/include
