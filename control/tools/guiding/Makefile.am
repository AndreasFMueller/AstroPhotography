#
# Makefile.am -- build the tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

bin_PROGRAMS = guider guideport 

guider_SOURCES = guider.cpp
guider_DEPENDENCIES = $(top_builddir)/lib/libastro.la
guider_LDADD = -L$(top_builddir)/lib -lastro

guideport_SOURCES = guideport.cpp
guideport_DEPENDENCIES = $(top_builddir)/lib/libastro.la
guideport_LDADD = -L$(top_builddir)/lib -lastro

