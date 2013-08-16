#
# Makefile.am -- build the test client
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
bin_PROGRAMS = astroinfo getimage
noinst_PROGRAMS = astroclient

ORBLDFLAGS = -lomniCodeSets4 -lomniConnectionMgmt4 -lomniDynamic4 \
	-lomniORB4 -lomnithread

getimage_SOURCES = getimage.cpp
getimage_DEPENDENCIES = ../lib/libastro.la
getimage_LDADD = -L../lib -lastro -L../idl -lastrocom @SBIGLIBRARY@ \
	$(ORBLDFLAGS)

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

image:	getimage
	./getimage -d -ORBInitRef NameService=corbaname::localhost \
		-e 0.01  -m sx

