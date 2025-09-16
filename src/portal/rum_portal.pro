#-------------------------------------------------
#
# Project created by QtCreator 2012-02-02T23:21:22
#
#-------------------------------------------------

QT += core gui sql network

TARGET = portal
TEMPLATE = app

DEFINES *= QT_USE_QSTRINGBUILDER

SOURCES += ui/mainwindow.cpp \
    ui/gameentry.cpp \
    ui/gamedownload.cpp

HEADERS  += ui/mainwindow.h \
    ui/mainwindow.h \
    ui/gameentry.h \
    ui/gamedownload.h

FORMS    += ui/mainwindow.ui \
    ui/gameentry.ui \
    ui/gamedownload.ui

RESOURCES += rum_portal.qrc
