#
# Makefile.am -- build directory for the ICE stuff
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
if ENABLE_ICE

lib_LTLIBRARIES = libiceastro.la

slice_sources = camera.cpp exceptions.cpp guider.cpp image.cpp device.cpp \
	tasks.cpp types.cpp

BUILT_SOURCES = $(slice_sources)

SOURCES = RemoteInstrument.cpp CommunicatorSingleton.cpp

libiceastro_la_SOURCES = \
	$(slice_sources) $(SOURCES)
libiceastro_la_CPPFLAGS = -I$(top_builddir)/ice/include -I@ICEDIR@/include

endif
