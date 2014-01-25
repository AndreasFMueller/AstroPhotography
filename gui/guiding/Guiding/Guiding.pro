#-------------------------------------------------
#
# Project created by QtCreator 2013-12-26T13:26:49
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = Guiding
TEMPLATE = app

INCLUDEPATH += -I /Users/afm/Projects/Astro/AstroPhotography/control/root/include -I /Users/afm/Projects/AstroPhotography/control/root/include -I /usr/local/include

LIBS += -L/Users/afm/Projects/Astro/AstroPhotography/control/root/lib -L/Users/afm/Projects/AstroPhotography/control/root/lib -lcorbastro -lastro -L/usr/local/lib -lomniCodeSets4 -lomniConnectionMgmt4 -lomniDynamic4 -lomniORB4 -lomnithread

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
