#
# Makefile.am -- build the unicap driver, based on the unicap stuff in the
#                astro library, if that was built
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_UNICAP

noinst_HEADERS = UnicapLocator.h

pkglib_LTLIBRARIES = unicap.la 

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libunicap.la

libunicap_la_SOURCES = 
libunicap_la_DEPENDENCIES = ../../lib/libastro.la
libunicap_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = unicaptests
unicaptests_SOURCES = unicaptests.cpp unicaptest.cpp
unicaptests_LDADD = -lcppunit ../../lib/libastro.la -L. -lunicap -lunicapudrv
unicaptests_DEPENDENCIES = ../../lib/libastro.la libunicap.la

endif

unicap_la_SOURCES = 
unicap_la_LDFLAGS = -module
unicap_la_DEPENDENCIES = ../../lib/libastro.la
unicap_la_LIBADD = ../../lib/libastro.la

endif
