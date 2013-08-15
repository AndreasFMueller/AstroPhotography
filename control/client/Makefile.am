#
# Makefile.am -- build the test client
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
bin_PROGRAMS = astroinfo
noinst_PROGRAMS = astroclient

ORBLDFLAGS = -lomniCodeSets4 -lomniConnectionMgmt4 -lomniDynamic4 \
	-lomniORB4 -lomnithread

astroinfo_SOURCES = astroinfo.cpp
astroinfo_DEPENDENCIES = ../lib/libastro.la
astroinfo_LDADD = -L../lib -lastro -L../idl -lastrocom @SBIGLIBRARY@ \
	$(ORBLDFLAGS)

astroclient_SOURCES = astroclient.cpp
astroclient_DEPENDENCIES = ../lib/libastro.la
astroclient_LDADD = -L../lib -lastro -L../idl -lastrocom @SBIGLIBRARY@ \
	$(ORBLDFLAGS)

test:	astroinfo
	./astroinfo -d -ORBInitRef NameService=corbaname::localhost



