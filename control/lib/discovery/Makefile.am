#
# Makefile.am -- Build the service discovery portion of the library
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . test

lib_LTLIBRARIES = libastrodiscovery.la

if ENABLE_AVAHI
avahi_files = 								\
	AvahiBase.cpp							\
	AvahiDiscovery.cpp						\
	AvahiPublisher.cpp						\
	AvahiResolver.cpp						\
	AvahiServiceSubset.cpp						\
	AvahiThread.cpp
endif

if ENABLE_BONJOUR
bonjour_files = 							\
	BonjourDiscovery.cpp						\
	BonjourPublisher.cpp						\
	BonjourResolver.cpp
endif

libastrodiscovery_la_SOURCES = 						\
	InstrumentComponent.cpp						\
	InstrumentComponentTable.cpp					\
	ServerName.cpp							\
	ServiceDiscovery.cpp						\
	ServiceInstrument.cpp						\
	ServiceKey.cpp							\
	ServiceObject.cpp						\
	ServicePublisher.cpp						\
	ServiceResolver.cpp						\
	ServiceSubset.cpp						\
	URL.cpp								\
	$(avahi_files) $(bonjour_files)

libastrodiscovery_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" -DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"


