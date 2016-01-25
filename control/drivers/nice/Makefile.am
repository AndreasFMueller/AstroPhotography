#
# Makefile.am -- build the nice dirver
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

ICELIBS = @ICELIBS@
ICELDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/ice/lib -liceastro $(ICELIBS)
ICECPPFLAGS = -I$(top_builddir)/ice/include -I$(top_builddir)/ice/slice \
	@ICEINCLUDE@
ICEDEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/ice/lib/libiceastro.la

pkglib_LTLIBRARIES = nice.la

noinst_HEADERS = NiceDevice.h NiceLocator.h NiceCamera.h NiceGuiderPort.h \
	NiceFilterWheel.h NiceCcd.h NiceCooler.h NiceAdaptiveOptics.h \
	NiceFocuser.h

SRCFILES = NiceDevice.cpp \
	NiceLocator.cpp NiceCamera.cpp NiceGuiderPort.cpp NiceCcd.cpp \
	NiceFilterWheel.cpp NiceCooler.cpp NiceAdaptiveOptics.cpp \
	NiceFocuser.cpp

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libnice.la

libnice_la_SOURCES = $(SRCFILES)
libnice_la_DEPENDENCIES = $(ICEDEPENDENCIES)
libnice_la_LIBADD = $(ICELDADD)
libnice_la_CPPFLAGS = $(ICECPPFLAGS)

noinst_PROGRAMS = nicetests
nicetests_SOURCES = nicetests.cpp LocatorTest.cpp CameraTest.cpp
nicetests_LDADD = -lcppunit $(ICELDADD) -L. -lnice
nicetests_DEPENDENCIES = $(ICEDPENDENCIES) libnice.la
nicetests_CPPFLAGS = $(ICECPPFLAGS)

test:	nicetests
	./nicetests -d

endif

nice_la_SOURCES = $(SRCFILES)
nice_la_LDFLAGS = -module
nice_la_DEPENDENCIES = $(ICEDEPENDENCIES)
nice_la_LIBADD = $(ICELDADD)
nice_la_CPPFLAGS = $(ICECPPFLAGS)

