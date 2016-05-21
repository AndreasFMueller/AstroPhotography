#
# Makefile.am -- build directory for the ICE stuff
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

SOURCES = TypesConversions.cpp DeviceConversions.cpp CcdConversions.cpp \
	FilterwheelConversions.cpp GuiderConversions.cpp TaskConversions.cpp \
	MountConversions.cpp FocusingConversions.cpp RepositoryConversions.cpp \
	InstrumentComponentConversions.cpp ImageConversions.cpp \
	ParameterConversions.cpp EventConversions.cpp ConfigConversions.cpp

noinst_LTLIBRARIES = libconversion.la 

libconversion_la_SOURCES = $(SOURCES)
libconversion_la_CPPFLAGS = -I$(top_builddir)/ice/include \
	-I$(top_builddir)/ice/slice @ICEINCLUDE@

