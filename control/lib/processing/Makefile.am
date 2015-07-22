#
# Makefile.am -- subdirectory of image processing related classes
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_HEADERS =

noinst_LTLIBRARIES = libastroprocessing.la

libastroprocessing_la_SOURCES =						\
	CalibrationImageStep.cpp					\
	CalibrationProcessorStep.cpp					\
	ImageBufferStep.cpp						\
	ImageCalibrationStep.cpp					\
	ImageProcessing.cpp						\
	ImageStep.cpp							\
	InterpolationStep.cpp						\
	PreviewAdapter.cpp						\
	ProcessingController.cpp					\
	ProcessingStep.cpp						\
	ProcessingThread.cpp						\
	RawImageFileStep.cpp						\
	WriteImageStep.cpp

libastroprocessing_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

