#
# Makefile.am -- build the mount tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

bin_PROGRAMS = astromount 

astromount_SOURCES = astromount.cpp
astromount_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astromount_LDADD = -L$(top_builddir)/lib -lastro

test:	astromount
	./astromount --debug GET mount:celestron/0 2>&1 | tee test.log
