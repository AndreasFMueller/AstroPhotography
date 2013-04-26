#
# Makefile.am -- build the sbig driver, based on the SBIG library
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_SBIG

pkglib_LTLIBRARIES = sbig.la 
noinst_LTLIBRARIES = libsbig.la

noinst_HEADERS = utils.h SbigLocator.h SbigCamera.h

libsbig_la_SOURCES = SbigLocator.cpp utils.cpp SbigCamera.cpp SbigCcd.cpp
libsbig_la_DEPENDENCIES = ../../lib/libastro.la
libsbig_la_LIBADD = ../../lib/libastro.la

sbig_la_SOURCES = SbigLocator.cpp utils.cpp SbigCamera.cpp SbigCcd.cpp
sbig_la_LDFLAGS = -module
sbig_la_DEPENDENCIES = ../../lib/libastro.la
sbig_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = sbigtests
sbigtests_SOURCES = sbigtests.cpp sbigtest.cpp
sbigtests_LDADD = -lcppunit ../../lib/libastro.la -L. -lsbig -lsbigudrv
sbigtests_DEPENDENCIES = ../../lib/libastro.la libsbig.la

endif
