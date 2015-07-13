#
# Makefile.am -- build the nice dirver
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
if ENABLE_ICE

pkglib_LTLIBRARIES = nice.la

noinst_HEADERS = NiceLocator.h

SRCFILES = NiceLocator.cpp

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libnice.la

libnice_la_SOURCES = $(SRCFILES)
libnice_la_DEPENDENCIES = $(tob_builddir)/lib/libastro.la
libnice_la_LIBADD = $(top_builddir)/lib/libastro.la

noinst_PROGRAMS = nicetests
nicetests_SOURCES = nicetests.cpp
nicetests_LDADD = -lcppunit $(top_builddir)/lib/libastro.la -L. -lnice
nicetests_DEPENDENCIES = $(top_builddir)/lib/libastro.la libnice.la

test:	nicetests
	./nicetests -d

endif

nice_la_SOURCES = $(SRFILES)
nice_la_LDFLAGS = -module
nice_la_DEPENDENCIES = $(top_builddir)/lib/libastro.la
nice_la_LIBADD = $(top_builddir)/lib/libastro.la

endif
