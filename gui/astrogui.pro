QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = capture

TEMPLATE = app

INCLUDEPATH += . /home/afm/Projects/AstroPhotography/gui/../control/root/include  -I/usr/local/include   -I/usr/local/include  

LIBS += -L/home/afm/Projects/AstroPhotography/gui/../control/root/lib64 -lcorbastro -lastro  -L/usr/local/lib64 -lomniORB4 -lomnithread   -L/usr/local/lib64 -lCOS4 -lomniORB4 -lomnithread  

QMAKE_CXXFLAGS += -g -Wall -O2 --std=c++11 -D__x86_64__ -D__linux__ -D__OSVERSION__=2   -D__x86_64__ -D__linux__ -D__OSVERSION__=2  

HEADERS += \ 
    capturewindow.h \
    exposurewidget.h \
    ExposureWorker.h \
    LogSpinBox.h
FORMS += \ 
    capturewindow.ui \
    exposurewidget.ui
SOURCES = capture.cpp \ 
    capturewindow.cpp \
    exposurewidget.cpp \
    ExposureWorker.cpp \
    LogSpinBox.cpp
