#
# Makefile.am -- build the mock1 driver class
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
pkglib_LTLIBRARIES = mock1.la

noinst_HEADERS = Mock1Camera.h Mock1Ccd.h

mock1_la_SOURCES = mock1.cpp Mock1CameraLocator.cpp Mock1Camera.cpp \
	Mock1Ccd.cpp
mock1_la_LDFLAGS = -module
mock1_la_DEPENDENCIES = ../../lib/libastro.la
mock1_la_LIBADD = ../../lib/libastro.la
