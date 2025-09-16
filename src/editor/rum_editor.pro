#-------------------------------------------------
#
# Project created by QtCreator 2012-02-02T23:21:22
#
#-------------------------------------------------

QT += core gui opengl network widgets

TARGET = editor
TEMPLATE = app

DEFINES *= QT_USE_QSTRINGBUILDER

SOURCES += ui/mainwindow.cpp \
    ui/assetmanager.cpp \
    ui/assetpicker.cpp \
    ui/combopicker.cpp \
    ui/datatablemanager.cpp \
    ui/languagetranslator.cpp \
    ui/mapeditor.cpp \
    ui/mapgoto.cpp \
    ui/newmap.cpp \
    ui/newproject.cpp \
    ui/newrelease.cpp \
    ui/newreleaseaction.cpp \
    ui/newscript.cpp \
    ui/output.cpp \
    ui/packagemanager.cpp \
    ui/patchmanager.cpp \
    ui/propertymanager.cpp \
    ui/qtFindReplace/finddialog.cpp \
    ui/qtFindReplace/findform.cpp \
    ui/qtFindReplace/findreplacedialog.cpp
    ui/qtFindReplace/findreplaceform.cpp \
    ui/releasemanager.cpp \
    ui/scriptdialog.cpp \
    ui/scripteditor.cpp \
    ui/scriptgoto.cpp \
    ui/smMapWidget.cpp \
    ui/smTabWidget.cpp \
    ui/smTextEdit.cpp \
    ui/smTreeWidget.cpp \
    ui/stringmanager.cpp \
    ui/stringtokenpicker.cpp \

HEADERS += ui/mainwindow.h \
    ui/assetmanager.h \
    ui/assetpicker.h \
    ui/combopicker.h \
    ui/datatablemanager.h \
    ui/languagetranslator.h \
    ui/mapeditor.h \
    ui/mapgoto.h \
    ui/newmap.h \
    ui/newproject.h \
    ui/newrelease.h \
    ui/newreleaseaction.h \
    ui/newscript.h \
    ui/output.cpp \
    ui/packagemanager.h \
    ui/patchmanager.h \
    ui/propertymanager.h \
    ui/qtFindReplace/finddialog.h \
    ui/qtFindReplace/findform.h \
    ui/qtFindReplace/findreplace_global.h
    ui/qtFindReplace/findreplacedialog.h \
    ui/qtFindReplace/findreplaceform.h \
    ui/releasemanager.h \
    ui/scriptdialog.h \
    ui/scripteditor.h \
    ui/scriptgoto.h \
    ui/sharedglwidget.h \
    ui/smMapWidget.h \
    ui/smTabWidget.h \
    ui/smTextEdit.h \
    ui/smTreeWidget.h \
    ui/stringmanager.h \
    ui/stringtokenpicker.h \

FORMS += ui/mainwindow.ui \
    ui/assetmanager.ui \
    ui/assetpicker.ui \
    ui/combopicker.ui \
    ui/datatablemanager.ui \
    ui/languagetranslator.ui \
    ui/mapeditor.ui \
    ui/mapgoto.ui \
    ui/newmap.ui \
    ui/newproject.ui \
    ui/newrelease.ui \
    ui/newreleaseaction.ui \
    ui/newscript.ui \
    ui/output.ui \
    ui/packagemanager.ui \
    ui/patchmanager.ui \
    ui/propertymanager.ui \
    ui/qtFindReplace/findreplacedialog.ui
    ui/qtFindReplace/findreplaceform.ui \
    ui/releasemanager.ui \
    ui/scriptdialog.ui \
    ui/scripteditor.ui \
    ui/scriptgoto.ui \
    ui/stringmanager.ui \
    ui/stringtokenpicker.ui \

RESOURCES += rum_editor.qrc
