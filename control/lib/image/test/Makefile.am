#
# Makefile.am -- tests for images
# 
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
noinst_HEADERS = 

test_ldadd = -lcppunit 							\
	-L$(top_builddir)/lib/image -lastroimage			\
	-L$(top_builddir)/lib/utils -lastroutils
test_dependencies = 							\
	$(top_builddir)/lib/utils/libastroutils.la  			\
	$(top_builddir)/lib/image/libastroimage.la  

if ENABLE_UNITTESTS

noinst_PROGRAMS = tests singletest

# single test
singletest_SOURCES = singletest.cpp \
	DeconvolveTest.cpp
#	FITSreadTest.cpp
#	FITSdateTest.cpp
#	DebayerTest.cpp
#	FWHMTest.cpp
#	EuclideanDisplacementTest.cpp
#	ConvolutionAdapterTest.cpp
#	DeconvolveTest.cpp
#	PhaseCorrelatorTest.cpp 
#	PeakFinderTest.cpp 
singletest_LDADD = $(test_ldadd)
singletest_DEPENDENCIES = $(test_dependencies)

.PHONY:	single
single:	singletest
	./singletest -d 2>&1 | tee single.log

## general tests
tests_SOURCES = tests.cpp 						\
	AdapterTest.cpp							\
	AnalyzerTest.cpp						\
	BackgroundTest.cpp						\
	ConvertingAdapterTest.cpp					\
	ConvolveTest.cpp						\
	ConvolutionAdapterTest.cpp					\
	DebayerTest.cpp							\
	DeconvolveTest.cpp						\
	NoiseTest.cpp							\
	EuclideanDisplacementTest.cpp					\
	FITSKeywordTest.cpp						\
	FITSdateTest.cpp						\
	FITSwriteTest.cpp						\
	FITSreadTest.cpp						\
	FilterTest.cpp							\
	FWHMTest.cpp							\
	HSLTest.cpp							\
	ImageBaseTest.cpp						\
	ImageBufferTest.cpp						\
	ImageIteratorBaseTest.cpp					\
	ImageLineTest.cpp						\
	ImagePointTest.cpp						\
	ImageRectangleTest.cpp						\
	ImageSizeTest.cpp						\
	ImageTest.cpp							\
	LinearFunctionTest.cpp						\
	MedianRadiusAdapterTest.cpp					\
	MinRadiusTest.cpp						\
	MosaicTest.cpp							\
	MultiplaneTest.cpp						\
	OperatorTest.cpp						\
	PhaseCorrelatorTest.cpp						\
	PixelTest.cpp							\
	PeakFinderTest.cpp						\
	QuadraticFunctionTest.cpp					\
	RGBTest.cpp							\
	RadonTest.cpp							\
	TransformTest.cpp						\
	TranslationTest.cpp						\
	WindowAdapterTest.cpp						\
	VectorFieldTest.cpp

tests_LDADD = $(test_ldadd)
tests_DEPENDENCIES = $(test_dependencies)

test:	tests
	./tests -d 2>&1 | tee test.log

endif
