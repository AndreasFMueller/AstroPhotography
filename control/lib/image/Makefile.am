#
# Makefile.am -- subdirectory of image processing related classes
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_HEADERS =							\
	ImageMean.h							\
	LevelExtractor.h						\
	Miniball.hpp							\
	TransformBuilder.h						\
	LowerBoundDegreeNFunction.h					\
	LowerBoundLinearFunction.h					\
	LowerBoundQuadraticFunction.h					\
	LuminanceFunctions.h

noinst_LTLIBRARIES = libastroimage.la

libastroimage_la_SOURCES = 						\
	absolute.cpp							\
	Adapters.cpp							\
	AffineTransformBuilder.cpp					\
	AiryImage.cpp							\
	AmplifierGlowImage.cpp						\
	Analyzer.cpp							\
	Background.cpp							\
	BackProjection.cpp						\
	BasicAdapter.cpp						\
	Binning.cpp							\
	Blurr.cpp							\
	CalibrationFrameFactory.cpp					\
	CalibrationFrameProcess.cpp					\
	Calibrator.cpp							\
	CentralProjection.cpp						\
	CGFilter.cpp							\
	Circle.cpp							\
	CircularImage.cpp						\
	Clamper.cpp							\
	ColorBalance.cpp						\
	ColorTransform.cpp						\
	ColorScaling.cpp						\
	ComponentBase.cpp						\
	ConnectedComponentBase.cpp					\
	ConvolutionOperator.cpp						\
	ConvolutionResult.cpp						\
	Corrector.cpp							\
	Cut.cpp								\
	DarkCorrector.cpp						\
	DarkFrameFactory.cpp						\
	DarkFrameProcess.cpp						\
	DarkNoiseAdapter.cpp						\
	DeconvolutionOperator.cpp					\
	Deemphasize.cpp							\
	Demosaic.cpp							\
	Destar.cpp							\
	DiskImage.cpp							\
	DisplayConverter.cpp						\
	duplicate.cpp							\
	EuclideanDisplacement.cpp					\
	EuclideanDisplacementConvolve.cpp				\
	FastVanCittertOperator.cpp					\
	FITS.cpp							\
	FITShdu.cpp							\
	FITSKeywords.cpp						\
	FITSdate.cpp							\
	FITSdirectory.cpp						\
	FITSfile.cpp							\
	FITSexception.cpp						\
	FITSin.cpp							\
	FITSinfile.cpp							\
	FITSout.cpp							\
	FITSoutfile.cpp							\
	Filters.cpp							\
	FlatCorrector.cpp						\
	FlatFrameFactory.cpp						\
	FlatFrameProcess.cpp						\
	Flip.cpp							\
	FocusFilterfunc.cpp						\
	Format.cpp							\
	FormatReductionBase.cpp						\
	FormatReduction.cpp						\
	FormatReductionRGB.cpp						\
	FourierDeconvolutionOperator.cpp				\
	FourierImage.cpp						\
	Functions.cpp							\
	FWHM.cpp							\
	GammaTransformBase.cpp						\
	GammaTransform.cpp						\
	GaussImage.cpp							\
	GaussNoiseAdapter.cpp						\
	HDR.cpp								\
	Histogram.cpp							\
	HotPixelBase.cpp						\
	HSLBase.cpp							\
	Image.cpp							\
	ImageBase.cpp							\
	ImageBuffer.cpp							\
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
	JPEG.cpp							\
	Layer.cpp							\
	LevelExtractor.cpp						\
	LevelMaskExtractor.cpp						\
	LinearLogLuminanceFactor.cpp					\
	LogImage.cpp							\
	Luminance.cpp							\
	LuminanceExtractor.cpp						\
	LuminanceFunction.cpp						\
	LuminanceFunctionFactory.cpp					\
	LuminanceFunctions.cpp						\
	luminancemapping.cpp						\
	LuminanceStretchingAdapter.cpp					\
	Masks.cpp							\
	Maxima.cpp							\
	Metavalue.cpp							\
	MinRadius.cpp							\
	MosaicType.cpp							\
	negative.cpp							\
	NoiseAdapter.cpp						\
	NormFilterfunc.cpp						\
	PhaseCorrelator.cpp						\
	PeakFinder.cpp							\
	Pixel.cpp							\
	positive.cpp							\
	PNG.cpp								\
	Projection.cpp							\
	ProjectionCorrector.cpp						\
	PseudoDeconvolutionOperator.cpp					\
	PsfExtractor.cpp						\
	Radon.cpp							\
	Rescale.cpp							\
	Residual.cpp							\
	RingImage.cpp							\
	RigidTransformBuilder.cpp					\
	SphereProjection.cpp						\
	Stack.cpp							\
	StackingAdapter.cpp						\
	Stacker.cpp							\
	Star.cpp							\
	StarExtractor.cpp						\
	StereographicProjection.cpp					\
	Subgrid.cpp							\
	SmallConvolve.cpp						\
	ThresholdExtractor.cpp						\
	TopLeftRectangle.cpp						\
	Transform.cpp							\
	TransformAnalyzer.cpp						\
	TransformBuilder.cpp						\
	TransformFactory.cpp						\
	TriangleAnalyzer.cpp						\
	Triangle.cpp							\
	TriangleSet.cpp							\
	TriangleSetFactory.cpp						\
	UnsharpMask.cpp							\
	VanCittertOperator.cpp						\
	VectorField.cpp							\
	Viewer.cpp							\
	ViewerPipeline.cpp						\
	WienerDeconvolutionOperator.cpp					\
	WeightingAdapter.cpp

libastroimage_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

