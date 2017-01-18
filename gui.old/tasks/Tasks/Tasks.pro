#-------------------------------------------------
#
# Project created by QtCreator 2014-01-25T10:45:28
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Tasks
TEMPLATE = app

INCLUDEPATH += /home/afm/Projects/AstroPhotography/gui/../control/root/include  -I/usr/local/include   -I/usr/local/include  

LIBS += -L/home/afm/Projects/AstroPhotography/gui/../control/root/lib64 -lcorbastro -lastro  -L/usr/local/lib64 -lomniORB4 -lomnithread   -L/usr/local/lib64 -lCOS4 -lomniORB4 -lomnithread  

SOURCES += main.cpp \
	connectiondialog.cpp taskconnectiondialog.cpp \
	deviceselector.cpp \
        mainwindow.cpp \
	taskmainwindow.cpp TaskMonitor_impl.cpp taskitem.cpp taskcreator.cpp \
	downloaddialog.cpp downloadparameters.cpp downloadreportdialog.cpp

HEADERS  += mainwindow.h \
	connectiondialog.h taskconnectiondialog.h \
	deviceselector.h \
	taskmainwindow.h taskitem.h taskcreator.h \
	downloaddialog.h downloadparameters.h downloadreportdialog.h

FORMS    += mainwindow.ui \
	connectiondialog.ui \
	taskmainwindow.ui taskcreator.ui \
	downloaddialog.ui downloadreportdialog.ui
