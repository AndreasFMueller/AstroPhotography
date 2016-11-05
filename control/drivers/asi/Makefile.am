#
# Makefile.am -- build the asi driver, based on the ASI library
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_ASI

noinst_HEADERS = AsiLocator.h utils.h AsiCamera.hh AsiCcd.h AsiGuidePort.h \
	AsiCooler.h AsiStream.h
pkglib_LTLIBRARIES = asi.la 

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libasi.la

libasi_la_SOURCES = AsiLocator.cpp utils.cpp AsiCamera.cpp AsiCcd.cpp  \
	AsiGuidePort.cpp AsiCooler.cpp AsiStream.cpp
libasi_la_DEPENDENCIES = ../../lib/libastro.la
libasi_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = asitests 
asitests_SOURCES = asitests.cpp asitest.cpp
asitests_LDADD = -lcppunit ../../lib/libastro.la -L. -lasi @ASILIBPATH@ @ASILIBS@
asitests_DEPENDENCIES = ../../lib/libastro.la libasi.la

endif

asi_la_SOURCES = AsiLocator.cpp utils.cpp AsiCamera.cpp AsiCcd.cpp \
	AsiGuidePort.cpp AsiCooler.cpp AsiStream.cpp
asi_la_LDFLAGS = -module
asi_la_DEPENDENCIES = ../../lib/libastro.la
asi_la_LIBADD = ../../lib/libastro.la @ASILIBPATH@ @ASILIBS@

endif
