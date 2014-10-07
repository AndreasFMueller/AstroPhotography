#
# Makefile.am -- build the starlight express driver
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
if ENABLE_SX

pkglib_LTLIBRARIES = sx.la 

noinst_HEADERS = sx.h SxUtils.h SxLocator.h SxCamera.h SxCcd.h SxDemux.h \
	SxGuiderPort.h SxAO.h

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libsx.la

libsx_la_SOURCES = SxUtils.cpp SxLocator.cpp SxCamera.cpp SxCcd.cpp \
	SxCooler.cpp SxCcdM26C.cpp SxDemux.cpp SxGuiderPort.cpp SxAO.cpp
libsx_la_DEPENDENCIES = ../../lib/libastro.la
libsx_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = sxtests aotest

sxtests_SOURCES = sxtests.cpp sxtest.cpp
sxtests_LDADD = -lcppunit ../../lib/libastro.la -L. -lsx
sxtests_DEPENDENCIES = ../../lib/libastro.la libsx.la

aotest_SOURCES = aotest.cpp
aotest_LDADD = -lcppunit ../../lib/libastro.la -L. -lsx
aotest_DEPENDENCIES = ../../lib/libastro.la libsx.la

endif

sx_la_SOURCES = SxUtils.cpp SxLocator.cpp SxCamera.cpp SxCcd.cpp \
	SxCooler.cpp SxCcdM26C.cpp SxDemux.cpp SxGuiderPort.cpp SxAO.cpp
sx_la_LDFLAGS = -module
sx_la_DEPENDENCIES = ../../lib/libastro.la
sx_la_LIBADD = ../../lib/libastro.la

endif
