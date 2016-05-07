#
# Makefile.am -- build the configuration tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

bin_PROGRAMS = astroconfig astroinstrument

astroconfig_SOURCES = astroconfig.cpp
astroconfig_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astroconfig_LDADD = -L$(top_builddir)/lib -lastro

astroinstrument_SOURCES = astroinstrument.cpp
astroinstrument_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astroinstrument_LDADD = -L$(top_builddir)/lib -lastro

