#
# Makefile.am -- build the qhyccd driver
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_QHY2

pkglib_LTLIBRARIES = qhy2.la 
noinst_HEADERS = QhyLocator.h QhyUtils.h QhyCamera.h QhyCcd.h QhyCooler.h

SRCFILES = QhyLocator.cpp QhyUtils.cpp QhyCamera.cpp QhyCcd.cpp QhyCooler.cpp

if USE_UDEV
if ENABLE_NONROOT
else
udevdir = /etc/udev/rules.d
udev_DATA = 
endif
endif

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libqhy2.la

libqhy2_la_SOURCES = $(SRCFILES)
libqhy2_la_DEPENDENCIES = ../../lib/libastro.la
libqhy2_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = qhy2tests
qhy2tests_SOURCES = qhy2tests.cpp qhy2test.cpp
qhy2tests_LDADD = -lcppunit ../../lib/libastro.la -L. -lqhy2 -lqhyccd
qhy2tests_DEPENDENCIES = ../../lib/libastro.la libqhy2.la

endif

qhy2_la_SOURCES = $(SRCFILES)
qhy2_la_LDFLAGS = -module
qhy2_la_DEPENDENCIES = ../../lib/libastro.la
qhy2_la_LIBADD = ../../lib/libastro.la @QHYLIB@

endif

