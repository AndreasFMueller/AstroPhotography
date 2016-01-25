#
# Makefile.am -- build directory for the ICE stuff
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

SOURCES = ImageCallbackI.cpp

noinst_LTLIBRARIES = libserver.la 

libserver_la_SOURCES = $(SOURCES)
libserver_la_CPPFLAGS = -I$(top_builddir)/ice/include \
	-I$(top_builddir)/ice/slice @ICEINCLUDE@

