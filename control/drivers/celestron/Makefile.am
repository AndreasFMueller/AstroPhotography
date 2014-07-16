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

