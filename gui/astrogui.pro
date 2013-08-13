QT += widgets
TEMPLATE = app
TARGET = capture
DEPENDPATH += .
INCLUDEPATH += . -I /usr/local/include -I ../control/include
LIBS += -L/usr/local/lib -L../control/lib -lastro

HEADERS += \ 
    capturewindow.h \
    exposurewidget.h \
    ExposureWorker.h
FORMS += \ 
    capturewindow.ui \
    exposurewidget.ui
SOURCES = capture.cpp \ 
    capturewindow.cpp \
    exposurewidget.cpp \
    ExposureWorker.cpp
