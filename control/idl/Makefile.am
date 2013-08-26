#
# Makefile.am -- Makefile for the idl directory
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

EXTRA_DIST = device.idl NameService.h 

BUILT_SOURCES = deviceSK.cc ../include/device.hh

deviceSK.cc ../include/device.hh:	device.idl
	omniidl -bcxx device.idl
	mv device.hh ../include

lib_LTLIBRARIES = libastrocom.la

libastrocom_la_SOURCES = deviceSK.cc NameService.cpp Conversions.cpp \
	OrbSingleton.cpp

