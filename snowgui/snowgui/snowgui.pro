#
# snowgui.pro -- qt configuration file for the snowgui project
#
# (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

QT       += core gui widgets

TARGET = snowgui
TEMPLATE = app

# Directories for ICE and the astrophotography library
ICEDIR = /Library/Developer/Ice-3.5.1
ASTROPATH = /Users/afm/Projects/AstroPhotography/snowgui/../control/root

LIBS += -L$${ASTROPATH}/lib -lastro -liceastro
LIBS +=	-L$${ICEDIR}/lib/c++11 -Wl,-rpath,$${ICEDIR}/lib/c++11 -lIce -lIceUtil

INCLUDEPATH += $${ASTROPATH}/include
INCLUDEPATH += $${ICEDIR}/include

SOURCES += main.cpp \
        mainwindow.cpp \
	connectiondialog.cpp \
	serverselectiondialog.cpp \
	instrumentselectiondialog.cpp \
	previewwindow.cpp

HEADERS  += mainwindow.h \
	connectiondialog.h \
	serverselectiondialog.h \
	instrumentselectiondialog.h \
	previewwindow.h

FORMS    += mainwindow.ui \
	connectiondialog.ui \
	serverselectiondialog.ui \
	instrumentselectiondialog.ui \
	previewwindow.ui

