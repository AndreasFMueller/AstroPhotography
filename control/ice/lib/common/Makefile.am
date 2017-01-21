#
# Makefile.am -- build directory for the ICE stuff
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

SOURCES =								\
	camera.cpp							\
	device.cpp							\
	exceptions.cpp							\
	focusing.cpp							\
	guider.cpp							\
	image.cpp							\
	instruments.cpp							\
	repository.cpp							\
	tasks.cpp							\
	types.cpp

BUILT_SOURCES = $(SOURCES)

noinst_LTLIBRARIES = libcommon.la 

libcommon_la_SOURCES = $(SOURCES)
libcommon_la_CPPFLAGS = -I$(top_builddir)/ice/include \
	-I$(top_builddir)/ice/slice @ICEINCLUDE@

