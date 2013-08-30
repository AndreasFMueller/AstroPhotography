#
# Makefile.am -- Makefile for the idl directory
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

EXTRA_DIST = device.idl

all-am:	../lib/deviceSK.cc ../include/device.hh

../lib/deviceSK.cc ../include/device.hh:	device.idl
	omniidl -bcxx device.idl
	mv device.hh ../include
	mv deviceSK.cc ../lib

