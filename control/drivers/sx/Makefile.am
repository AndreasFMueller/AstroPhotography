#
# Makefile.am -- build the starlight express driver
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
pkglib_LTLIBRARIES = sx.la

noinst_HEADERS = sx.h

sx_la_SOURCES = sx.cpp
sx_la_LDFLAGS = -module
sx_la_DEPENDENCIES = ../../lib/libastro.la
sx_la_LIBADD = ../../lib/libastro.la
