#
# Makefile.am -- build the tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

bin_PROGRAMS = guider guiderport 

guider_SOURCES = guider.cpp
guider_DEPENDENCIES = $(top_builddir)/lib/libastro.la
guider_LDADD = -L$(top_builddir)/lib -lastro @SBIGLIBRARY@ 

guiderport_SOURCES = guiderport.cpp
guiderport_DEPENDENCIES = $(top_builddir)/lib/libastro.la
guiderport_LDADD = -L$(top_builddir)/lib -lastro

