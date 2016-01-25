#
# Makefile.am -- build directory for the ICE stuff
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

SOURCES = RemoteInstrument.cpp CommunicatorSingleton.cpp \
	CommonClientTasks.cpp IceDiscovery.cpp

noinst_LTLIBRARIES = libclient.la 

libclient_la_SOURCES = $(SOURCES)
libclient_la_CPPFLAGS = -I$(top_builddir)/ice/include \
	-I$(top_builddir)/ice/slice @ICEINCLUDE@

