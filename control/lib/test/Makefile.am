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

# files needed for the MicroTouch driver tests
if ENABLE_MICROTOUCH
microtouch_tests = microtouchtests.cpp MicroTouchTest.cpp
microtouch_cmds = microtouchtests
else
microtouch_tests =
microtouch_cmds =
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
noinst_PROGRAMS = tests usbtests calimggen caltests \
	$(uvc_cmds) $(microtouch_cmds) $(unicap_cmds)

# general tests
tests_SOURCES = tests.cpp \
	PixelTest.cpp \
	ImagePointTest.cpp ImageSizeTest.cpp ImageRectangleTest.cpp \
	ImageBaseTest.cpp ImageIteratorBaseTest.cpp ImageLineTest.cpp \
	ImageTest.cpp RGBTest.cpp MosaicTest.cpp \
	OperatorTest.cpp FilterTest.cpp TransformTest.cpp \
	FITSwriteTest.cpp FITSreadTest.cpp \
	BinningTest.cpp \
	DescriptorTest.cpp ModuleTest.cpp RepositoryTest.cpp
tests_LDADD = -lcppunit -L.. -lastro
tests_DEPENDENCIES = ../libastro.la

# tests for USB library
usbtests_SOURCES = usbtests.cpp \
	USBContextTest.cpp USBDescriptorTest.cpp
usbtests_LDADD = -lcppunit -L.. -lastro
usbtests_DEPENDENCIES = ../libastro.la

calimggen_SOURCES = calimggen.cpp
calimggens_LDADD = -L.. -lastro
calimggen_DEPENDENCIES = ../libastro.la

caltests_SOURCES = caltests.cpp
caltests_LDADD = -lcppunit -L.. -lastro
caltests_DEPENDENCIES = ../libastro.la

# tests for UVC cameras
if ENABLE_UVC
uvctests_SOURCES = $(uvc_tests)
uvctests_LDADD = -lcppunit -L.. -lastro
uvctests_DEPENDENCIES = ../libastro.la
endif

# tests for the MicroTouch driver
if ENABLE_MICROTOUCH
microtouchtests_SOURCES = $(microtouch_tests)
microtouchtests_LDADD = -lcppunit -L.. -lastro
microtouchtests_DEPENDENCIES = ../libastro.la
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

endif
