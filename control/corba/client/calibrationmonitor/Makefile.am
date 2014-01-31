#
# Makefile.am -- build the calibration monitor client
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
if ENABLE_CORBA

bin_PROGRAMS = calibrationmonitor

calibrationmonitor_SOURCES = calibrationmonitor.cpp
calibrationmonitor_DEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/corba/lib/libcorbastro.la
calibrationmonitor_LDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/corba/lib -lcorbastro @SBIGLIBRARY@

test:	calibrationmonitor
	./calibrationmonitor -d -ORBInitRef NameService=corbaname::localhost \
		-C camera:simulator/camera -c 0 \
		-g guiderport:simulator/guiderport

endif
