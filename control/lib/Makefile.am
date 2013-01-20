#
# build the basic library
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . test

lib_LTLIBRARIES = libastro.la

libastro_la_SOURCES = debug.cpp Format.cpp \
	ImagePoint.cpp ImageSize.cpp ImageRectangle.cpp ImageBase.cpp \
	ImageIteratorBase.cpp ImageLine.cpp Image.cpp RGBPixel.cpp \
	Binning.cpp \
	Descriptor.cpp Module.cpp Repository.cpp \
	Ccd.cpp
libastro_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\"
