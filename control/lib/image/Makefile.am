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
	AmplifierGlowImage.cpp						\
	Analyzer.cpp							\
	Background.cpp							\
	BasicDeconvolutionOperator.cpp					\
	Binning.cpp							\
	Blurr.cpp							\
	CalibrationFactory.cpp						\
	CentralProjection.cpp						\
	CGFilter.cpp							\
	Circle.cpp							\
	CircularImage.cpp						\
	Clamper.cpp							\
	ConnectedComponent.cpp						\
	ConvolutionOperator.cpp						\
	ConvolutionResult.cpp						\
	Corrector.cpp							\
	DarkCorrector.cpp						\
	DarkNoiseAdapter.cpp						\
	DeconvolutionOperator.cpp					\
	Demosaic.cpp							\
	DiskImage.cpp							\
	DisplayConverter.cpp						\
	EuclideanDisplacement.cpp					\
	EuclideanDisplacementConvolve.cpp				\
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
	GaussNoiseAdapter.cpp						\
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
	Layer.cpp							\
	LevelMaskExtractor.cpp						\
	Masks.cpp							\
	Metavalue.cpp							\
	MinRadius.cpp							\
	MosaicType.cpp							\
	NoiseAdapter.cpp						\
	PhaseCorrelator.cpp						\
	PeakFinder.cpp							\
	Pixel.cpp							\
	Projection.cpp							\
	ProjectionCorrector.cpp						\
	Radon.cpp							\
	Residual.cpp							\
	RingImage.cpp							\
	SphereProjection.cpp						\
	Stack.cpp							\
	StackingAdapter.cpp						\
	Stacker.cpp							\
	StereographicProjection.cpp					\
	Subgrid.cpp							\
	SmallConvolve.cpp						\
	Transform.cpp							\
	TransformAnalyzer.cpp						\
	VanCittertOperator.cpp						\
	Viewer.cpp							\
	ViewerPipeline.cpp						\
	WeightingAdapter.cpp

libastroimage_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

