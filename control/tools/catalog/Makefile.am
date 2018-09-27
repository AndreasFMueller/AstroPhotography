#
# Makefile.am -- build the tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

bin_PROGRAMS = starcatalog buildcatalog

starcatalog_SOURCES = starcatalog.cpp
starcatalog_DEPENDENCIES = $(top_builddir)/lib/libastro.la
starcatalog_LDADD = -L$(top_builddir)/lib -lastro 
starcatalog_CXXFLAGS = -DDATAROOTDIR=\"${datarootdir}\"

buildcatalog_SOURCES = buildcatalog.cpp
buildcatalog_DEPENDENCIES = $(top_builddir)/lib/libastro.la
buildcatalog_LDADD = -L$(top_builddir)/lib -lastro 
buildcatalog_CXXFLAGS = -DDATAROOTDIR=\"${datarootdir}\"

catalogtest:	buildcatalog
	./buildcatalog -d \
		-h /usr/local/starcatalogs/hipparcos/hip_main.dat \
		-t /usr/local/starcatalogs/tycho2/tyc2.dat \
		-u /usr/local/starcatalogs/u4 \
		/usr/local/starcatalogs/database/stars.db
	
