#
# Makefile.am -- build the tracking monitor client
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
if ENABLE_CORBA

bin_PROGRAMS = trackingmonitor

trackingmonitor_SOURCES = trackingmonitor.cpp
trackingmonitor_DEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/corba/lib/libcorbastro.la
trackingmonitor_LDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/corba/lib -lcorbastro @SBIGLIBRARY@

test:	trackingmonitor
	./trackingmonitor -d -ORBInitRef NameService=corbaname::localhost \
		-C camera:simulator/camera -c 0 \
		-g guiderport:simulator/guiderport

endif
