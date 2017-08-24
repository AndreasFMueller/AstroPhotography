#
# Makefile.am -- subdirectory of image processing related classes
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_HEADERS =							\
	ProcessorParser.h

noinst_LTLIBRARIES = libastroprocessing.la

libastroprocessing_la_SOURCES =						\
	CalibrationImageStep.cpp					\
	CalibrationProcessorStep.cpp					\
	DarkImageStep.cpp						\
	FileImageStep.cpp						\
	FlatImageStep.cpp						\
	ImageBufferStep.cpp						\
	ImageCalibrationStep.cpp					\
	ImageProcessing.cpp						\
	ImageStep.cpp							\
	InterpolationStep.cpp						\
	PreviewAdapter.cpp						\
	ProcessingController.cpp					\
	ProcessingStatic.cpp						\
	ProcessingStep.cpp						\
	ProcessingThread.cpp						\
	ProcessorFactory.cpp						\
	ProcessorNetwork.cpp						\
	ProcessorParser.cpp						\
	ParserSteps.cpp							\
	RawImageFileStep.cpp						\
	StackingStep.cpp						\
	WriteImageStep.cpp						\
	WriteableFileImageStep.cpp

libastroprocessing_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

# test
