#
# Makefile -- build the test subdirectory
# 
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
noinst_HEADERS = 

if ENABLE_UNITTESTS

# files needed for the Unicap driver tests
if ENABLE_UNICAP
unicap_tests = unicaptests.cpp UnicapTest.cpp
unicap_cmds = unicaptests
else
unicap_tests =
unicap_cmds =
endif

# files needed for the UVC driver tests
if ENABLE_UVC
uvc_tests = uvctests.cpp UVCDescriptorTest.cpp UVCCameraTest.cpp
uvc_cmds = uvctests
else
uvc_tests =
uvc_cmds =
endif

# stuff related to testing
noinst_PROGRAMS = tests processtests catalogtests singletest \
	usbtests calimggen caltests simtest \
	guidetest tasktest aotest phasecorr $(uvc_cmds) $(unicap_cmds)

## general tests
tests_SOURCES = tests.cpp \
	StacktraceTest.cpp TrimTest.cpp \
	PixelTest.cpp ConcatenatorTest.cpp SplitterTest.cpp URLTest.cpp \
	DeviceNameTest.cpp PropertiesTest.cpp FITSdateTest.cpp \
	FITSKeywordTest.cpp ImageServerTablesTest.cpp \
	ImagePointTest.cpp ImageSizeTest.cpp ImageRectangleTest.cpp \
	ImageBaseTest.cpp ImageIteratorBaseTest.cpp ImageLineTest.cpp \
	ImageTest.cpp RGBTest.cpp MosaicTest.cpp \
	OperatorTest.cpp FilterTest.cpp TransformTest.cpp \
	TranslationTest.cpp FWHMTest.cpp \
	WindowAdapterTest.cpp ConvertingAdapterTest.cpp \
	PreviewAdapterTest.cpp \
	RawImageFileStepTest.cpp \
	WriteImageFileStepTest.cpp ImageBufferStepTest.cpp \
	PhaseCorrelatorTest.cpp StarDetectorTest.cpp \
	FITSwriteTest.cpp FITSreadTest.cpp MultiplaneTest.cpp \
	BinningTest.cpp \
	ModuleDescriptorTest.cpp ModuleTest.cpp RepositoryTest.cpp \
	LinearFunctionTest.cpp QuadraticFunctionTest.cpp BackgroundTest.cpp \
	TableTest.cpp UpdateSpecTest.cpp DatabaseTest.cpp \
	FocusComputeTest.cpp MinRadiusTest.cpp \
	AnalyzerTest.cpp CorrectorTest.cpp \
	GuiderFactoryTest.cpp \
	ImageCalibrationStepTest.cpp CalibrationProcessorStepTest.cpp \
	InterpolationStepTest.cpp 

# single test
singletest_SOURCES = singletest.cpp \
	ImageServerTest.cpp
singletest_LDADD = -lcppunit -L.. -lastro 
singletest_DEPENDENCIES = ../libastro.la 

single:	singletest
	./singletest -d 2>&1 | tee single.log

# tests related to the processing engine. These tests include stuff
processtests_SOURCES = processtests.cpp \
	ProcessingStepTest.cpp ProcessingThreadTest.cpp \
	ProcessingControllerTest.cpp 
processtests_LDADD = -lcppunit -L.. -lastro 
processtests_DEPENDENCIES = ../libastro.la 

# tests related to star catalogs and charting
catalogtests_SOURCES = catalogtests.cpp \
	ProjectionTest.cpp StereographicProjectionTest.cpp \
	SkyWindowTest.cpp \
	Ucac4Test.cpp Tycho2Test.cpp BSCTest.cpp HipparcosTest.cpp \
	NGCICTest.cpp ChartTest.cpp StereographicChartTest.cpp \
	ImageNormalizerTest.cpp SkyRectangleTest.cpp
catalogtests_LDADD = -lcppunit -L.. -lastro 
catalogtests_DEPENDENCIES = ../libastro.la 

# test for the image processing stuff (takes time)
tests_LDADD = -lcppunit -L.. -lastro 
tests_DEPENDENCIES = ../libastro.la 

# tests for USB library
usbtests_SOURCES = usbtests.cpp \
	USBContextTest.cpp USBDescriptorTest.cpp
usbtests_LDADD = -lcppunit -L.. -lastro
usbtests_DEPENDENCIES = ../libastro.la

calimggen_SOURCES = calimggen.cpp
calimggen_LDADD = -L.. -lastro
calimggen_DEPENDENCIES = ../libastro.la

caltests_SOURCES = caltests.cpp
caltests_LDADD = -lcppunit -L.. -lastro
caltests_DEPENDENCIES = ../libastro.la

simtest_SOURCES = simtest.cpp
simtest_LDADD = -L.. -lastro
simtest_DEPENDENCIES = ../libastro.la

guidetest_SOURCES = guidetest.cpp
guidetest_LDADD = -L.. -lastro
guidetest_DEPENDENCIES = ../libastro.la

# test program for the task queue
tasktest_SOURCES = tasktest.cpp
tasktest_LDADD = -L.. -lastro
tasktest_DEPENDENCIES = ../libastro.la

# test program for AO units
aotest_SOURCES = aotest.cpp
aotest_LDADD = -L.. -lastro
aotest_DEPENDENCIES = ../libastro.la

# test program for phase correlation of images
phasecorr_SOURCES = phasecorr.cpp
phasecorr_LDADD = -L.. -lastro
phasecorr_DEPENDENCIES = ../libastro.la

# tests for UVC cameras
if ENABLE_UVC
uvctests_SOURCES = $(uvc_tests)
uvctests_LDADD = -lcppunit -L.. -lastro
uvctests_DEPENDENCIES = ../libastro.la
endif

# tests for Unicap driver
if ENABLE_UNICAP
unicaptests_SOURCES = $(unicap_tests)
unicaptests_LDADD = -lcppunit -L.. -lastro
unicaptests_DEPENDENCIES = ../libastro.la
endif

# test2_SOURCES = test2.cpp
# test2_LDADD = -L.. -lastro
# test2_DEPENDENCIES = ../libastro.la

EXTRA_DIST = test_module.la test_module.so

#all-am:	tests usbtests 
#	./usbtests

testguide:	guidetest
	rm -f gt/*
	./guidetest -d -s "(380,152)" -r "64x64@(348,120)" -t 200 -T 3600 -i gt -C 2>&1 | tee testguide.log

endif
