#
# Makefile.am -- build the persistence layer
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . test

noinst_LTLIBRARIES = libastropersistence.la

libastropersistence_la_SOURCES = 					\
	Backend.cpp							\
	Configuration.cpp						\
	GlobalTable.cpp							\
	NoSuchEntry.cpp							\
	Persistence.cpp							\
	Testtable.cpp	

libastropersistence_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

