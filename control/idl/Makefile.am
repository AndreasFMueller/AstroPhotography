#
# Makefile.am -- Makefile for the idl directory
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

EXTRA_DIST = device.idl NameService.h

BUILT_SOURCES = deviceSK.cc device.hh

deviceSK.cc device.h:	device.idl
	omniidl -bcxx device.idl

lib_LTLIBRARIES = libastrocom.la

libastrocom_la_SOURCES = deviceSK.cc NameService.cpp

