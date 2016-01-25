#
# Makefile.am -- build directory for the ICE stuff
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

SOURCES = camera.cpp exceptions.cpp focusing.cpp guider.cpp image.cpp \
	device.cpp tasks.cpp types.cpp  repository.cpp instruments.cpp

BUILT_SOURCES = $(SOURCES)

noinst_LTLIBRARIES = libcommon.la 

libcommon_la_SOURCES = $(SOURCES)
libcommon_la_CPPFLAGS = -I$(top_builddir)/ice/include \
	-I$(top_builddir)/ice/slice @ICEINCLUDE@

