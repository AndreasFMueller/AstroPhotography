QT += widgets
TEMPLATE = app
TARGET = fitsviewer
DEPENDPATH += .
INCLUDEPATH += . -I /usr/local/include -I ../../control/include
LIBS += -L/usr/local/lib -L../../control/lib -lastro

HEADERS += FITSViewerWindow.h histogramwidget.h
    
FORMS += fitsviewerwindow.ui

SOURCES = fitsviewer.cpp histogramwidget.cpp \
	FITSViewerWindow.cpp
