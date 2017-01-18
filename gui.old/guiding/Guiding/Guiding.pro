#-------------------------------------------------
#
# Project created by QtCreator 2013-12-26T13:26:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Guiding
TEMPLATE = app

INCLUDEPATH += /home/afm/Projects/AstroPhotography/gui/../control/root/include  -I/usr/local/include   -I/usr/local/include  

LIBS += -L/home/afm/Projects/AstroPhotography/gui/../control/root/lib64 -lcorbastro -lastro  -L/usr/local/lib64 -lomniORB4 -lomnithread   -L/usr/local/lib64 -lCOS4 -lomniORB4 -lomnithread  

SOURCES += main.cpp\
	logspinbox.cpp guidermonitordialog.cpp \
	guidehistorywidget.cpp connectiondialog.cpp \
	guidingconnectiondialog.cpp deviceselector.cpp \
	guideropendialog.cpp guiderwidget.cpp \
	calibrationmonitor.cpp errorwidget.cpp \
	calibrationpointwidget.cpp calibrationwidget.cpp

HEADERS  += \
	logspinbox.h guidermonitordialog.h \
	guidehistorywidget.h connectiondialog.h \
	guidingconnectiondialog.h deviceselector.h \
	guideropendialog.h guiderwidget.h \
	calibrationmonitor.h errorwidget.h \
	calibrationpointwidget.h calibrationwidget.h

FORMS    += \
	guidermonitordialog.ui connectiondialog.ui \
	guideropendialog.ui guiderwidget.ui \
	calibrationmonitor.ui
