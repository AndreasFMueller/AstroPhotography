QT += widgets
TEMPLATE = app
TARGET = capture
DEPENDPATH += .
INCLUDEPATH += . -I /usr/local/include -I ../control/include
LIBS += -L../control/root/lib64 -L/usr/local/lib -L../control/lib -lastro -lcorbastro
QMAKE_CXXFLAGS += --std=c++11 -stdlib=libc++ -mmacosx-version-min=10.9

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
