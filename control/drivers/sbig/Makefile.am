#
# Makefile.am -- build the sbig driver, based on the SBIG library
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_SBIG

noinst_HEADERS = utils.h SbigLocator.h SbigCamera.h
pkglib_LTLIBRARIES = sbig.la 

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libsbig.la

libsbig_la_SOURCES = SbigLocator.cpp utils.cpp SbigCamera.cpp SbigCcd.cpp
libsbig_la_DEPENDENCIES = ../../lib/libastro.la
libsbig_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = sbigtests
sbigtests_SOURCES = sbigtests.cpp sbigtest.cpp
sbigtests_LDADD = -lcppunit ../../lib/libastro.la -L. -lsbig -lsbigudrv
sbigtests_DEPENDENCIES = ../../lib/libastro.la libsbig.la

endif

sbig_la_SOURCES = SbigLocator.cpp utils.cpp SbigCamera.cpp SbigCcd.cpp
sbig_la_LDFLAGS = -module
sbig_la_DEPENDENCIES = ../../lib/libastro.la
sbig_la_LIBADD = ../../lib/libastro.la

endif
