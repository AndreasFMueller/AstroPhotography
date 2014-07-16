#
# Makefile.am -- build the celestron driver
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
pkglib_LTLIBRARIES = celestron.la
noinst_HEADERS = CelestronLocator.h 
	SimFocuser.h Stars.h Blurr.h

SRCFILES = CelestronLocator.cpp

celestron_la_SOURCES = $(SRCFILES)
celestron_la_LDFLAGS = -module
celestron_la_DEPENDENCIES = ../../lib/libastro.la
celestron_la_LIBADD = ../../lib/libastro.la

