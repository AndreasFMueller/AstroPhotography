#
# Makefile.am -- build directory for the ICE stuff
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = common conversion client server .

noinst_HEADERS = IceVersion.h

lib_LTLIBRARIES = libiceastro.la 

libiceastro_la_SOURCES = IceVersion.cpp
libiceastro_la_LIBADD = 					\
	common/libcommon.la					\
	conversion/libconversion.la				\
	server/libserver.la					\
	client/libclient.la

