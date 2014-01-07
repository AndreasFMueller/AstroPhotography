#
# Makefile.am -- build the test client
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . trackingmonitor

bin_PROGRAMS = astroinfo getimage
noinst_PROGRAMS = astroclient

getimage_SOURCES = getimage.cpp
getimage_DEPENDENCIES = ../lib/libastro.la ../lib/libcorbastro.la
getimage_LDADD = -L../lib -lastro -lcorbastro @SBIGLIBRARY@

astroinfo_SOURCES = astroinfo.cpp
astroinfo_DEPENDENCIES = ../lib/libastro.la ../lib/libcorbastro.la
astroinfo_LDADD = -L../lib -lastro -lcorbastro @SBIGLIBRARY@

astroclient_SOURCES = astroclient.cpp
astroclient_DEPENDENCIES = ../lib/libastro.la ../lib/libcorbastro.la
astroclient_LDADD = -L../lib -lastro -lcorbastro @SBIGLIBRARY@

test:	astroinfo
	./astroinfo -d -ORBInitRef NameService=corbaname::localhost

image:	getimage
	./getimage -d -ORBInitRef NameService=corbaname::localhost \
		-e 1  -m sx test.fits

