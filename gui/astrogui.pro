QT += widgets
TEMPLATE = app
TARGET = capture
DEPENDPATH += .
INCLUDEPATH += . -I ../control/include
LIBS += -L../control/lib -lastro

HEADERS += \ 
    capturewindow.h \
    exposurewidget.h
FORMS += \ 
    capturewindow.ui \
    exposurewidget.ui
SOURCES = capture.cpp \ 
    capturewindow.cpp \
    exposurewidget.cpp
