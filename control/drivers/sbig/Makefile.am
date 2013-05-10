#
# Makefile.am -- build the sbig driver, based on the SBIG library
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_SBIG

noinst_HEADERS = utils.h SbigLocator.h SbigCamera.h SbigFilterWheel.h
pkglib_LTLIBRARIES = sbig.la 

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libsbig.la

libsbig_la_SOURCES = SbigLocator.cpp utils.cpp SbigCamera.cpp SbigCcd.cpp \
	SbigFilterWheel.cpp
libsbig_la_DEPENDENCIES = ../../lib/libastro.la
libsbig_la_LIBADD = ../../lib/libastro.la
libsbig_la_CPPFLAGS = -DTARGET=@SBIGTARGET@

noinst_PROGRAMS = sbigtests 
sbigtests_SOURCES = sbigtests.cpp sbigtest.cpp
sbigtests_LDADD = -lcppunit ../../lib/libastro.la -L. -lsbig @SBIGLIBRARY@
sbigtests_DEPENDENCIES = ../../lib/libastro.la libsbig.la
sbigtests_CPPFLAGS = -DTARGET=@SBIGTARGET@

endif

sbig_la_SOURCES = SbigLocator.cpp utils.cpp SbigCamera.cpp SbigCcd.cpp \
	SbigFilterWheel.cpp
sbig_la_LDFLAGS = -module
sbig_la_DEPENDENCIES = ../../lib/libastro.la
sbig_la_LIBADD = ../../lib/libastro.la
sbig_la_CPPFLAGS = -DTARGET=@SBIGTARGET@

endif
