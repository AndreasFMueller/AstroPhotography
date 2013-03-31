#
# Makefile -- build the test subdirectory
# 
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
noinst_HEADERS = 

# stuff related to testing
noinst_PROGRAMS = tests

tests_SOURCES = tests.cpp \
	PixelTest.cpp \
	ImagePointTest.cpp ImageSizeTest.cpp ImageRectangleTest.cpp \
	ImageBaseTest.cpp ImageIteratorBaseTest.cpp ImageLineTest.cpp \
	ImageTest.cpp RGBTest.cpp \
	FITSwriteTest.cpp FITSreadTest.cpp \
	BinningTest.cpp \
	DescriptorTest.cpp ModuleTest.cpp RepositoryTest.cpp \
	USBContextTest.cpp USBDescriptorTest.cpp UVCDescriptorTest.cpp

tests_LDADD = -lcppunit -L.. -lastro
tests_DEPENDENCIES = ../libastro.la

EXTRA_DIST = test_module.la test_module.so

all-am:	tests
	./tests
