QT += widgets
TEMPLATE = app
TARGET = capture
DEPENDPATH += .
INCLUDEPATH += . -I ../control/include
LIBS += -L../control/lib -lastro

HEADERS += \ 
    capturewindow.h
FORMS += \ 
    capturewindow.ui
SOURCES = capture.cpp \ 
    capturewindow.cpp
