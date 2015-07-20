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

# stuff related to testing
noinst_PROGRAMS = tests processtests singletest \
	calimggen caltests simtest \
	phasecorr

## general tests
tests_SOURCES = tests.cpp \
	ImageRepoTablesTest.cpp ImageRepoTest.cpp \
	PreviewAdapterTest.cpp \
	RawImageFileStepTest.cpp \
	WriteImageFileStepTest.cpp ImageBufferStepTest.cpp \
	ProjectTableTest.cpp GlobalTableTest.cpp ConfigurationTest.cpp \
	DeviceMapTableTest.cpp DeviceMapperTest.cpp \
	CorrectorTest.cpp \
	ImageCalibrationStepTest.cpp CalibrationProcessorStepTest.cpp \
	InterpolationStepTest.cpp \
	InstrumentComponentTest.cpp

# single test
singletest_SOURCES = singletest.cpp \
	ConfigurationTest.cpp 
#	InstrumentComponentTest.cpp
#	InstrumentTest.cpp
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

# test for the image processing stuff (takes time)
tests_LDADD = -lcppunit -L.. -lastro 
tests_DEPENDENCIES = ../libastro.la 

calimggen_SOURCES = calimggen.cpp
calimggen_LDADD = -L.. -lastro
calimggen_DEPENDENCIES = ../libastro.la

caltests_SOURCES = caltests.cpp
caltests_LDADD = -lcppunit -L.. -lastro
caltests_DEPENDENCIES = ../libastro.la

simtest_SOURCES = simtest.cpp
simtest_LDADD = -L.. -lastro
simtest_DEPENDENCIES = ../libastro.la

# test program for phase correlation of images
phasecorr_SOURCES = phasecorr.cpp
phasecorr_LDADD = -L.. -lastro
phasecorr_DEPENDENCIES = ../libastro.la

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
