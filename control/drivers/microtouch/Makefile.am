#
# Makefile.am -- build the microtouch camera
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
if ENABLE_MICROTOUCH

noinst_HEADERS = MtLocator.h MtFocuser.h MicroTouch.h
pkglib_LTLIBRARIES = microtouch.la

SRCFILES = MtLocator.cpp MtFocuser.cpp MicroTouch.cpp

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libmicrotouch.la

libmicrotouch_la_SOURCES = $(SRCFILES)
libmicrotouch_la_DEPENDENCIES = ../../lib/libastro.la
libmicrotouch_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = microtouchtests
if ENABLE_MICROTOUCH
microtouchtests_SOURCES = microtouchtests.cpp MicroTouchTest.cpp
microtouchtests_LDADD = -lcppunit ../../lib/libastro.la -L. -lmicrotouch
microtouchtests_DEPENDENCIES = ../../lib/libastro.la libmicrotouch.la
endif

test:	microtouchtests
	./microtouchtests -d

endif

microtouch_la_SOURCES = $(SRCFILES)
microtouch_la_LDFLAGS = -module
microtouch_la_DEPENDENCIES = ../../lib/libastro.la
microtouch_la_LIBADD = ../../lib/libastro.la

endif
