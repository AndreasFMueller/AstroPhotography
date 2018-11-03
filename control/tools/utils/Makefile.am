#
# Makefile.am -- build the utilities
#
# (c) 2018 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

bin_PROGRAMS = astrotime

noinst_HEADERS = 

noinst_PROGRAMS = 

astrotime_SOURCES = astrotime.cpp
astrotime_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astrotime_LDADD = -L$(top_builddir)/lib -lastro

test:	astrotime
	./astrotime --debug 9.7531 46.9246
