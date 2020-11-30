#
# Makefile.am -- build the qhyccd driver
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_QHY

pkglib_LTLIBRARIES = qhy.la 
noinst_HEADERS = QhyLocator.h QhyUtils.h QhyCamera.h QhyCcd.h QhyCooler.h

SRCFILES = QhyLocator.cpp QhyUtils.cpp QhyCamera.cpp QhyCcd.cpp QhyCooler.cpp

if USE_UDEV
if ENABLE_NONROOT
else
udevdir = /etc/udev/rules.d
udev_DATA = firmware/85-qhy.rules
endif
endif

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libqhy.la

libqhy_la_SOURCES = $(SRCFILES)
libqhy_la_DEPENDENCIES = ../../lib/libastro.la
libqhy_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = qhytests
qhytests_SOURCES = qhytests.cpp qhytest.cpp
qhytests_LDADD = -lcppunit ../../lib/libastro.la -L. -lqhy -lqhyccd
qhytests_DEPENDENCIES = ../../lib/libastro.la libqhy.la

endif

qhy_la_SOURCES = $(SRCFILES)
qhy_la_LDFLAGS = -module
qhy_la_DEPENDENCIES = ../../lib/libastro.la
qhy_la_LIBADD = ../../lib/libastro.la @QHYLIB0@

endif

