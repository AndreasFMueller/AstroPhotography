#
# Makefile.am -- build directory for the ICE stuff
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

SOURCES =								\
	CcdConversions.cpp						\
	ConfigConversions.cpp						\
	DeviceConversions.cpp						\
	EventConversions.cpp						\
	FilterwheelConversions.cpp					\
	FocusingConversions.cpp						\
	GuiderConversions.cpp						\
	ImageConversions.cpp						\
	InstrumentComponentConversions.cpp				\
	MountConversions.cpp						\
	ParameterConversions.cpp					\
	RepositoryConversions.cpp					\
	TaskConversions.cpp						\
	TypesConversions.cpp

noinst_LTLIBRARIES = libconversion.la 

libconversion_la_SOURCES = $(SOURCES)
libconversion_la_CPPFLAGS = -I$(top_builddir)/ice/include \
	-I$(top_builddir)/ice/slice @ICEINCLUDE@

