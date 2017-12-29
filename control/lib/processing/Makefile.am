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
	ColorclampStep.cpp						\
	ColorStep.cpp							\
	DarkImageStep.cpp						\
	DestarStep.cpp							\
	FileImageStep.cpp						\
	FlatImageStep.cpp						\
	ImageCalibrationStep.cpp					\
	ImageStep.cpp							\
	HDRStep.cpp							\
	ParseCalibrateStep.cpp						\
	ParseColorStep.cpp						\
	ParseColorclampStep.cpp						\
	ParseDarkimageStep.cpp						\
	ParseDestarStep.cpp						\
	ParseFileimageStep.cpp						\
	ParseFlatimageStep.cpp						\
	ParseHDRStep.cpp						\
	ParseRescaleStep.cpp						\
	ParseStackStep.cpp						\
	ParseWriteFileimageStep.cpp					\
	ParserSteps.cpp							\
	PreviewAdapter.cpp						\
	ProcessingStatic.cpp						\
	ProcessingStep.cpp						\
	ProcessingThread.cpp						\
	ProcessorFactory.cpp						\
	ProcessorNetwork.cpp						\
	ProcessorParser.cpp						\
	RescaleStep.cpp							\
	StackingStep.cpp						\
	WriteableFileImageStep.cpp

libastroprocessing_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

# test
