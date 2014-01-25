#-------------------------------------------------
#
# Project created by QtCreator 2014-01-25T10:45:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Tasks
TEMPLATE = app

INCLUDEPATH += -I /Users/afm/Projects/Astro/AstroPhotography/control/root/include -I /Users/afm/Projects/AstroPhotography/control/root/include -I /usr/local/include

LIBS += -L/Users/afm/Projects/Astro/AstroPhotography/control/root/lib -L/Users/afm/Projects/AstroPhotography/control/root/lib -lcorbastro -lastro -L/usr/local/lib -lomniCodeSets4 -lomniConnectionMgmt4 -lomniDynamic4 -lomniORB4 -lomnithread

SOURCES += main.cpp \
	connectiondialog.cpp taskconnectiondialog.cpp \
        mainwindow.cpp \
    taskmainwindow.cpp

HEADERS  += mainwindow.h \
	connectiondialog.h taskconnectiondialog.h \
    taskmainwindow.h

FORMS    += mainwindow.ui \
	connectiondialog.ui \
    taskmainwindow.ui
