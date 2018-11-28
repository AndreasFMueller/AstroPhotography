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
	ColorStep.cpp							\
	ColorclampStep.cpp						\
	DarkImageStep.cpp						\
	DestarStep.cpp							\
	FileImageStep.cpp						\
	FlatImageStep.cpp						\
	HDRStep.cpp							\
	ImageCalibrationStep.cpp					\
	ImagePlaneStep.cpp						\
	ImageStep.cpp							\
	ImageTransformationStep.cpp					\
	LRGBStep.cpp							\
	LayerImageStep.cpp						\
	LuminanceStretchingStep.cpp					\
	NodePaths.cpp							\
	ParseCalibrateStep.cpp						\
	ParseColorStep.cpp						\
	ParseColorclampStep.cpp						\
	ParseDarkimageStep.cpp						\
	ParseDestarStep.cpp						\
	ParseFileimageStep.cpp						\
	ParseFlatimageStep.cpp						\
	ParseHDRStep.cpp						\
	ParseImage.cpp							\
	ParseImagePlaneStep.cpp						\
	ParseLayerImageStep.cpp						\
	ParseLRGBStep.cpp						\
	ParseLuminanceStretchingStep.cpp				\
	ParseRGBStep.cpp						\
	ParseRescaleStep.cpp						\
	ParseStackStep.cpp						\
	ParseSumStep.cpp						\
	ParseTransformationStep.cpp					\
	ParseWriteFileimageStep.cpp					\
	ParserSteps.cpp							\
	PreviewAdapter.cpp						\
	ProcessingStatic.cpp						\
	ProcessingStep.cpp						\
	ProcessingThread.cpp						\
	ProcessorFactory.cpp						\
	ProcessorNetwork.cpp						\
	ProcessorParser.cpp						\
	RGBStep.cpp							\
	RescaleStep.cpp							\
	StackingStep.cpp						\
	StepPath.cpp							\
	SumStep.cpp							\
	WritableFileImageStep.cpp

libastroprocessing_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

# test
