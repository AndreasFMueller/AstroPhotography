#
# Makefile.am -- build the calibration monitor client
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
bin_PROGRAMS = calibrationmonitor

calibrationmonitor_SOURCES = calibrationmonitor.cpp
calibrationmonitor_DEPENDENCIES = ../../lib/libastro.la ../../lib/libcorbastro.la
calibrationmonitor_LDADD = -L../../lib -lastro -lcorbastro @SBIGLIBRARY@

test:	calibrationmonitor
	./calibrationmonitor -d -ORBInitRef NameService=corbaname::localhost \
		-C camera:simulator/camera -c 0 \
		-g guiderport:simulator/guiderport

