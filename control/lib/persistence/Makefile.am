#
# Makefile.am -- build the persistence layer
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . test

noinst_HEADERS =							\
	Testtable.h							\
	ConfigurationTable.h						\
	ConfigurationBackend.h

noinst_LTLIBRARIES = libastropersistence.la

libastropersistence_la_SOURCES = 					\
	Backend.cpp							\
	Configuration.cpp						\
	ConfigurationBackend.cpp					\
	ConfigurationEntry.cpp						\
	ConfigurationKey.cpp						\
	ConfigurationRegister.cpp					\
	ConfigurationRegistry.cpp					\
	ConfigurationTable.cpp						\
	NoSuchEntry.cpp							\
	Persistence.cpp							\
	TableBase.cpp							\
	Testtable.cpp	

libastropersistence_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

