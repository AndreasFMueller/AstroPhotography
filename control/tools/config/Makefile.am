#
# Makefile.am -- build the configuration tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

bin_PROGRAMS = astroconfig devicemapper astroinstrument

astroconfig_SOURCES = astroconfig.cpp
astroconfig_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astroconfig_LDADD = -L$(top_builddir)/lib -lastro

devicemapper_SOURCES = devicemapper.cpp
devicemapper_DEPENDENCIES = $(top_builddir)/lib/libastro.la
devicemapper_LDADD = -L$(top_builddir)/lib -lastro

astroinstrument_SOURCES = astroinstrument.cpp
astroinstrument_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astroinstrument_LDADD = -L$(top_builddir)/lib -lastro

