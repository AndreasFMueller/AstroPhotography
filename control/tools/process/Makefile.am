#
# Makefile.am -- tool to execute a process from an XML file
#
# (c) 2017 Prof Dr Andreas Müller, Hochschule Rapperswil
#
bin_PROGRAMS = astroprocess

astroprocess_SOURCES = astroprocess.cpp
astroprocess_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astroprocess_LDADD = -L$(top_builddir)/lib -lastro

test:	astroprocess process.xml
	./astroprocess --debug process.xml 2>&1 | tee test.log
