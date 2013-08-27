#
# Makefile.am -- build the server
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
bin_PROGRAMS = astrod

EXTRA_DIST = Modules_impl.h DriverModule_impl.h DeviceLocator_impl.h \
	Camera_impl.h Ccd_impl.h GuiderPort_impl.h FilterWheel_impl.h \
	Cooler_impl.h Image_impl.h

noinst_LTLIBRARIES = libserver.la
libserver_la_SOURCES = Modules_impl.cpp DriverModule_impl.cpp \
	DeviceLocator_impl.cpp Camera_impl.cpp Ccd_impl.cpp \
	GuiderPort_impl.cpp FilterWheel_impl.cpp Cooler_impl.cpp \
	Image_impl.cpp

astrod_SOURCES = astrod.cpp
astrod_DEPENDENCIES = ../lib/libastro.la libserver.la
astrod_LDADD = libserver.la -L../lib -lastro @SBIGLIBRARY@

test:	astrod
	./astrod -d -ORBInitRef NameService=corbaname::localhost




