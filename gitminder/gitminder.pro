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
    gitwatcher.cpp \
    notifytimer.cpp

HEADERS  += mainwindow.h \
    gitwatcher.h \
    notifytimer.h

FORMS    += mainwindow.ui

OTHER_FILES += \

RESOURCES +=


win32:CONFIG(release, debug|release): LIBS += -L$$PWD/libgit2/lib/ -lgit2
else:win32:CONFIG(debug, debug|release): LIBS += -L$$PWD/libgit2/lib/ -lgit2
else:unix: LIBS += -L$$PWD/libgit2/lib/ -lgit2

INCLUDEPATH += $$PWD/libgit2/include
DEPENDPATH += $$PWD/libgit2/include
