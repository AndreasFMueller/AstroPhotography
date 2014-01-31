#
# Makefile.am -- build the net camera
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
if ENABLE_CORBA

pkglib_LTLIBRARIES = net.la
noinst_HEADERS = NetLocator.h NetCamera.h NetCcd.h \
	NetGuiderPort.h NetFilterWheel.h NetCooler.h \
	NetFocuser.h NetUtils.h

SRCFILES = NetLocator.cpp NetCamera.cpp NetCcd.cpp \
	NetGuiderPort.cpp NetFilterWheel.cpp NetCooler.cpp \
	NetFocuser.cpp NetUtils.cpp

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libnet.la

libnet_la_SOURCES = $(SRCFILES)
libnet_la_DEPENDENCIES = $(top_builddir)/lib/libastro.la
libnet_la_LIBADD = $(top_builddir)/lib/libastro.la

noinst_PROGRAMS = nettests
nettests_SOURCES = nettests.cpp 
nettests_LDADD = -lcppunit $(top_builddir)/lib/libastro.la -L. -lnet
nettests_DEPENDENCIES = $(top_builddir)/lib/libastro.la libnet.la

test:	nettests
	./nettests -d

endif

net_la_SOURCES = $(SRCFILES)
net_la_LDFLAGS = -module
net_la_DEPENDENCIES = $(top_builddir)/lib/libastro.la
net_la_LIBADD = $(top_builddir)/lib/libastro.la

endif
