#
# Makefile.am -- Build the service discovery portion of the library
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . test

noinst_HEADERS =							\
	AvahiDiscovery.h						\
	BonjourDiscovery.h						\
	InstrumentBackendImpl.h						\
	InstrumentComponentTable.h					\
	InstrumentImpl.h						\
	InstrumentPropertyTable.h

noinst_LTLIBRARIES = libastrodiscovery.la

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
	Instrument.cpp							\
	InstrumentBackend.cpp						\
	InstrumentBackendImpl.cpp					\
	InstrumentComponent.cpp						\
	InstrumentComponentKey.cpp					\
	InstrumentComponentTable.cpp					\
	InstrumentImpl.cpp						\
	InstrumentList.cpp						\
	InstrumentProperty.cpp						\
	InstrumentPropertyTable.cpp					\
	PostData.cpp							\
	ServerName.cpp							\
	ServiceDiscovery.cpp						\
	ServiceKey.cpp							\
	ServiceLocation.cpp						\
	ServiceObject.cpp						\
	ServicePublisher.cpp						\
	ServiceResolver.cpp						\
	ServiceSubset.cpp						\
	URL.cpp								\
	$(avahi_files) $(bonjour_files)

libastrodiscovery_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" -DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"


