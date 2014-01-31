#
# Makefile.am -- Makefile for the include files
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
if ENABLE_CORBA

CORBAHEADERS = types.hh exceptions.hh image.hh camera.hh module.hh \
	guider.hh tasks.hh

BUILT_SOURCES = $(CORBAHEADERS)

include_HEADERS = $(CORBAHEADERS)

noinst_HEADERS = NameService.h OrbSingleton.h Conversions.h \
	CorbaExceptionReporter.h DeviceLocatorAdapter.h 

endif
