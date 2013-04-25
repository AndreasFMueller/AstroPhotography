#
# Makefile.am -- build the starlight express driver
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
pkglib_LTLIBRARIES = sx.la 
noinst_LTLIBRARIES = libsx.la
noinst_HEADERS = sxhw.h

libsx_la_SOURCES = sxhw.cpp
libsx_la_DEPENDENCIES = ../../lib/libastro.la
libsx_la_LIBADD = ../../lib/libastro.la

sx_la_SOURCES = sxhw.cpp
sx_la_LDFLAGS = -module
sx_la_DEPENDENCIES = ../../lib/libastro.la
sx_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = sxtests
sxtests_SOURCES = sxtests.cpp sxtest.cpp
sxtests_LDADD = -lcppunit ../../lib/libastro.la -L. -lsx
sxtests_DEPENDENCIES = ../../lib/libastro.la
