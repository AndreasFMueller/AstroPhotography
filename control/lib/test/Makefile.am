#
# Makefile -- build the test subdirectory
# 
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
noinst_HEADERS = 

# stuff related to testing
noinst_PROGRAMS = tests usbtests  microtouchtests

tests_SOURCES = tests.cpp \
	PixelTest.cpp \
	ImagePointTest.cpp ImageSizeTest.cpp ImageRectangleTest.cpp \
	ImageBaseTest.cpp ImageIteratorBaseTest.cpp ImageLineTest.cpp \
	ImageTest.cpp RGBTest.cpp \
	FITSwriteTest.cpp FITSreadTest.cpp \
	BinningTest.cpp \
	DescriptorTest.cpp ModuleTest.cpp RepositoryTest.cpp
tests_LDADD = -lcppunit -L.. -lastro
tests_DEPENDENCIES = ../libastro.la

usbtests_SOURCES = usbtests.cpp \
	UVCCameraTest.cpp

#	USBContextTest.cpp USBDescriptorTest.cpp UVCDescriptorTest.cpp 
usbtests_LDADD = -lcppunit -L.. -lastro
usbtests_DEPENDENCIES = ../libastro.la

microtouchtests_SOURCES = microtouchtests.cpp \
	MicroTouchTest.cpp
microtouchtests_LDADD = -lcppunit -L.. -lastro
microtouchtests_DEPENDENCIES = ../libastro.la

test2_SOURCES = test2.cpp
test2_LDADD = -L.. -lastro
test2_DEPENDENCIES = ../libastro.la

EXTRA_DIST = test_module.la test_module.so

#all-am:	tests usbtests 
#	./usbtests

