QT += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = capture

TEMPLATE = app

INCLUDEPATH += . -I /Users/afm/Projects/Astro/AstroPhotography/control/root/include -I /usr/local/include

LIBS += -L/Users/afm/Projects/Astro/AstroPhotography/control/root/lib -lcorbastro -lastro -L/usr/local/lib -lomniCodeSets4 -lomniConnectionMgmt4 -lomniDynamic4 -lomniORB4 -lomnithread

QMAKE_CXXFLAGS += -mmacosx-version-min=10.9 -std=c++11 -stdlib=libc++

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
