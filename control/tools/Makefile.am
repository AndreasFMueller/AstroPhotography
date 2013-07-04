#
# Makefile.am -- build the tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
#SUBDIRS = . test

if ENABLE_UNICAP
unicap_flies = UnicapDevice.cpp
else
unicap_flies =
endif

if ENABLE_MICROTOUCH
microtouch_files = MicroTouch.cpp
else
microtouch_files =
endif

bin_PROGRAMS = getimages imageinfo makedark makeflat

getimages_SOURCES = getimages.cpp
getimages_DEPENDENCIES = ../lib/libastro.la
getimages_LDADD = -L../lib -lastro

imageinfo_SOURCES = imageinfo.cpp
imageinfo_DEPENDENCIES = ../lib/libastro.la
imageinfo_LDADD = -L../lib -lastro

makedark_SOURCES = makedark.cpp
makedark_DEPENDENCIES = ../lib/libastro.la
makedark_LDADD = -L../lib -lastro

makeflat_SOURCES = makeflat.cpp
makeflat_DEPENDENCIES = ../lib/libastro.la
makeflat_LDADD = -L../lib -lastro
