#
# Makefile.am -- build the celestron driver
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
pkglib_LTLIBRARIES = celestron.la
noinst_HEADERS = CelestronLocator.h CelestronMount.h

SRCFILES = CelestronLocator.cpp CelestronMount.cpp

celestron_la_SOURCES = $(SRCFILES)
celestron_la_LDFLAGS = -module
celestron_la_DEPENDENCIES = ../../lib/libastro.la
celestron_la_LIBADD = ../../lib/libastro.la

deviceconfdir = $(sysconfdir)/device.d
deviceconf_DATA = celestron.properties-dist

celestron.properties-dist:	device.properties
	cp device.properties celestron.properties-dist

install-data-hook:
	if [ ! -f $(deviceconfdir)/celestron.properties ]; 		\
	then								\
		cp $(deviceconfdir)/celestron.properties-dist		\
			$(deviceconfdir)/celestron.properties;		\
	fi

