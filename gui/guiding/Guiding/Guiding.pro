#-------------------------------------------------
#
# Project created by QtCreator 2013-12-26T13:26:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Guiding
TEMPLATE = app

INCLUDEPATH += -I /Users/afm/Projects/Astro/AstroPhotography/control/root/include -I /usr/local/include

LIBS += -L/Users/afm/Projects/Astro/AstroPhotography/control/root/lib -lcorbastro -lastro -L/usr/local/lib -lomniCodeSets4 -lomniConnectionMgmt4 -lomniDynamic4 -lomniORB4 -lomnithread

SOURCES += main.cpp\
        mainwindow.cpp \
    guiderdialog.cpp

HEADERS  += mainwindow.h \
    guiderdialog.h

FORMS    += mainwindow.ui \
    guiderdialog.ui
