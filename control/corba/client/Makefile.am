#
# Makefile.am -- build the test clients
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . trackingmonitor

if ENABLE_CORBA

bin_PROGRAMS = astroinfo getimage
noinst_PROGRAMS = astroclient

getimage_SOURCES = getimage.cpp
getimage_DEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/corba/lib/libcorbastro.la
getimage_LDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/corba/lib -lcorbastro @SBIGLIBRARY@

astroinfo_SOURCES = astroinfo.cpp
astroinfo_DEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/corba/lib/libcorbastro.la
astroinfo_LDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/corba/lib -lcorbastro @SBIGLIBRARY@

astroclient_SOURCES = astroclient.cpp
astroclient_DEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/corba/lib/libcorbastro.la
astroclient_LDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/corba/lib -lcorbastro @SBIGLIBRARY@

test:	astroinfo
	./astroinfo -d -ORBInitRef NameService=corbaname::localhost

image:	getimage
	./getimage -d -ORBInitRef NameService=corbaname::localhost \
		-e 1  -m sx test.fits

endif
