#
# Makefile.am -- build the config layer
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . test

noinst_HEADERS = 							\
	DeviceMapTable.h						\
	ImageRepoTables.h						\
	ImageReposTable.h						\
	InstrumentTables.h						\
	ProjectTable.h

noinst_LTLIBRARIES = libastroconfig.la

libastroconfig_la_SOURCES =						\
	DeviceMapTable.cpp						\
	DeviceMapper.cpp						\
	DeviceMapperConfiguration.cpp					\
	ImageEnvelope.cpp						\
	ImageRepo.cpp							\
	ImageRepoConfiguration.cpp					\
	ImageReposTable.cpp						\
	ImageRepoTables.cpp						\
	ImageSpec.cpp							\
	Instrument.cpp							\
	InstrumentConfiguration.cpp					\
	InstrumentTables.cpp						\
	Project.cpp							\
	ProjectConfiguration.cpp					\
	ProjectTable.cpp						\
	RepoReplicator.cpp

libastroconfig_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

