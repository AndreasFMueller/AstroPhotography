#
# Makefile.am -- subdirectory of solarsystem related classes
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_LTLIBRARIES = libastrosolarsystem.la

libastrosolarsystem_la_SOURCES = 					\
	Moon.cpp							\
	SolarsystemBody.cpp						\
	SolarsystemFactory.cpp						\
	Sun.cpp

libastrosolarsystem_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

