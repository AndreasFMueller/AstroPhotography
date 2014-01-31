#
# build the basic library
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
if ENABLE_CORBA

lib_LTLIBRARIES = libcorbastro.la

corba_sources = typesSK.cc exceptionsSK.cc imageSK.cc cameraSK.cc moduleSK.cc \
	guiderSK.cc tasksSK.cc

BUILT_SOURCES = $(corba_sources)

libcorbastro_la_SOURCES = \
	Conversions.cpp NameService.cpp OrbSingleton.cpp \
	CorbaExceptionReporter.cpp DeviceLocatorAdapter.cpp \
	$(corba_sources)

endif
