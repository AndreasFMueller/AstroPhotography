#
# Makefile.am -- build the tracking monitor client
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
bin_PROGRAMS = trackingmonitor

trackingmonitor_SOURCES = trackingmonitor.cpp
trackingmonitor_DEPENDENCIES = ../../lib/libastro.la ../../lib/libcorbastro.la
trackingmonitor_LDADD = -L../../lib -lastro -lcorbastro @SBIGLIBRARY@

test:	trackingmonitor
	./trackingmonitor -d -ORBInitRef NameService=corbaname::localhost \
		-C camera:simulator/camera -c 0 \
		-g guiderport:simulator/guiderport

