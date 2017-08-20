#
# Makefile.am -- build the image processing tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = config mount catalog devices image guiding process

bin_PROGRAMS = astroproject 

astroproject_SOURCES = astroproject.cpp
astroproject_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astroproject_LDADD = -L$(top_builddir)/lib -lastro

.PHONY:	orion
orion:
	rm -f orion.fits
	./stack --debug --output=orion.fits orion/orion*.fits 2>&1 | tee stack.log
#	./stack --debug --output=orion.fits orion/orion1.fits orion/orion2.fits 2>&1 | tee stack.log
