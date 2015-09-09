#
# Makefile.am -- subdirectory of image processing related classes
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_HEADERS =							\
	Miniball.hpp

noinst_LTLIBRARIES = libastroimage.la

libastroimage_la_SOURCES = 						\
	Adapters.cpp							\
	AiryImage.cpp							\
	Analyzer.cpp							\
	Background.cpp							\
	Blurr.cpp							\
	CalibrationFactory.cpp						\
	CircularImage.cpp						\
	Clamper.cpp							\
	ConnectedComponent.cpp						\
	ConvolutionOperator.cpp						\
	ConvolutionResult.cpp						\
	Corrector.cpp							\
	DarkCorrector.cpp						\
	Demosaic.cpp							\
	DiskImage.cpp							\
	DisplayConverter.cpp						\
	FITS.cpp							\
	FITSKeywords.cpp						\
	FITSdate.cpp							\
	FITSdirectory.cpp						\
	FITSexception.cpp						\
	FITSin.cpp							\
	FITSout.cpp							\
	Filters.cpp							\
	FlatCorrector.cpp						\
	FocusFilterfunc.cpp						\
	FourierImage.cpp						\
	Functions.cpp							\
	GaussImage.cpp							\
	Histogram.cpp							\
	HSLBase.cpp							\
	Image.cpp							\
	ImageBase.cpp							\
	ImageDatabaseDirectory.cpp					\
	ImageDirectory.cpp						\
	ImageIteratorBase.cpp						\
	ImageLine.cpp							\
	ImageMetadata.cpp						\
	ImagePersistence.cpp						\
	ImagePoint.cpp							\
	ImageProgramCallback.cpp					\
	ImageProperties.cpp						\
	ImageRectangle.cpp						\
	ImageSize.cpp							\
	Interpolation.cpp						\
	LevelMaskExtractor.cpp						\
	Masks.cpp							\
	Metavalue.cpp							\
	MinRadius.cpp							\
	MosaicType.cpp							\
	PhaseCorrelator.cpp						\
	Pixel.cpp							\
	Projection.cpp							\
	ProjectionCorrector.cpp						\
	Radon.cpp							\
	Residual.cpp							\
	RingImage.cpp							\
	Stacker.cpp							\
	StereographicProjection.cpp					\
	Subgrid.cpp							\
	Transform.cpp							\
	TransformAnalyzer.cpp						\
	Viewer.cpp							\
	ViewerPipeline.cpp

libastroimage_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

