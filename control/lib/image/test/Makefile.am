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
	ConvolutionAdapterTest.cpp
#	DeconvolveTest.cpp
#	PhaseCorrelatorTest.cpp 
#	PeakFinderTest.cpp 
singletest_LDADD = $(test_ldadd)
singletest_DEPENDENCIES = $(test_dependencies)

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
	DeconvolveTest.cpp						\
	NoiseTest.cpp							\
	FITSKeywordTest.cpp						\
	FITSdateTest.cpp						\
	FITSwriteTest.cpp						\
	FITSreadTest.cpp						\
	FilterTest.cpp							\
	FWHMTest.cpp							\
	HSLTest.cpp							\
	ImageBaseTest.cpp						\
	ImageIteratorBaseTest.cpp					\
	ImageLineTest.cpp						\
	ImagePointTest.cpp						\
	ImageRectangleTest.cpp						\
	ImageSizeTest.cpp						\
	ImageTest.cpp							\
	LinearFunctionTest.cpp						\
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
	WindowAdapterTest.cpp

tests_LDADD = $(test_ldadd)
tests_DEPENDENCIES = $(test_dependencies)

test:	tests
	./tests -d 2>&1 | tee test.log

endif
