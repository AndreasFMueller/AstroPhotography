#
# Makefile.am -- build the Quantum Scientific Imaging driver
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_QSI

pkglib_LTLIBRARIES = qsi.la 
noinst_HEADERS = QsiLocator.h QsiCamera.h QsiCcd.h QsiCooler.h \
	QsiFilterWheel.h QsiGuidePort.h QsiUtils.h

SRCFILES = QsiLocator.cpp QsiCamera.cpp QsiCcd.cpp QsiCooler.cpp \
	QsiFilterWheel.cpp QsiGuidePort.cpp

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libqsi.la

libqsi_la_SOURCES = $(SRCFILES)
libqsi_la_DEPENDENCIES = $(top_builddir)/lib/libastro.la
libqsi_la_LIBADD = $(top_builddir)/lib/libastro.la

noinst_PROGRAMS = qsitests
qsitests_SOURCES = qsitests.cpp qsitest.cpp
qsitests_LDADD = -lcppunit $(top_builddir)/lib/libastro.la -L. -lqsi @QSILIBS@
qsitests_DEPENDENCIES = $(top_builddir)/lib/libastro.la libqsi.la

endif

qsi_la_SOURCES = $(SRCFILES)
qsi_la_LDFLAGS = -module
qsi_la_DEPENDENCIES = $(top_builddir)/lib/libastro.la
qsi_la_LIBADD = $(top_builddir)/lib/libastro.la @QSILIBS@

endif

