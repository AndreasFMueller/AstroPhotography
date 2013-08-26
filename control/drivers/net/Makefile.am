#
# Makefile.am -- build the net camera
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
pkglib_LTLIBRARIES = net.la
noinst_HEADERS = NetLocator.h NetCamera.h NetCcd.h \
	NetGuiderPort.h NetFilterWheel.h NetCooler.h

SRCFILES = NetLocator.cpp NetCamera.cpp NetCcd.cpp \
	NetGuiderPort.cpp NetFilterWheel.cpp NetCooler.cpp

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libnet.la

libnet_la_SOURCES = $(SRCFILES)
libnet_la_DEPENDENCIES = ../../lib/libastro.la
libnet_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = nettests
nettests_SOURCES = nettests.cpp \
	NetLocatorTest.cpp NetCoolerTest.cpp \
	NetCameraTest.cpp NetCcdTest.cpp NetGuiderPortTest.cpp
nettests_LDADD = -lcppunit ../../lib/libastro.la -L. -lnet
nettests_DEPENDENCIES = ../../lib/libastro.la libnet.la

test:	nettests
	./nettests -d

endif

net_la_SOURCES = $(SRCFILES)
net_la_LDFLAGS = -module
net_la_DEPENDENCIES = ../../lib/libastro.la
net_la_LIBADD = ../../lib/libastro.la

