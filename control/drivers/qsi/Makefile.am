#
# Makefile.am -- build the starlight express driver
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
if ENABLE_QSI

pkglib_LTLIBRARIES = qsi.la 
noinst_HEADERS = QsiLocator.h

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libqsi.la

libqsi_la_SOURCES = QsiLocator.cpp
libqsi_la_DEPENDENCIES = ../../lib/libastro.la
libqsi_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = qsitests
qsitests_SOURCES = qsitests.cpp qsitest.cpp
qsitests_LDADD = -lcppunit ../../lib/libastro.la -L. -lqsi
qsitests_DEPENDENCIES = ../../lib/libastro.la libqsi.la

endif

qsi_la_SOURCES = QsiLocator.cpp
qsi_la_LDFLAGS = -module
qsi_la_DEPENDENCIES = ../../lib/libastro.la
qsi_la_LIBADD = ../../lib/libastro.la

endif

