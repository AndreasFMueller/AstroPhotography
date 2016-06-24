#
# Makefile.am -- build the asi driver, based on the ASI library
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_ASI

noinst_HEADERS = utils.h AsiLocator.h AsiCamera.h
pkglib_LTLIBRARIES = asi.la 

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libasi.la

libasi_la_SOURCES = AsiLocator.cpp utils.cpp AsiCamera.cpp AsiCcd.cpp 
libasi_la_DEPENDENCIES = ../../lib/libastro.la
libasi_la_LIBADD = ../../lib/libastro.la
libasi_la_CPPFLAGS = -DTARGET=@ASITARGET@

noinst_PROGRAMS = asitests  testcase
asitests_SOURCES = asitests.cpp asitest.cpp
asitests_LDADD = -lcppunit ../../lib/libastro.la -L. -lasi @ASILIBRARY@
asitests_DEPENDENCIES = ../../lib/libastro.la libasi.la
asitests_CPPFLAGS = -DTARGET=@ASITARGET@

testcase_SOURCES = testcase.cpp
testcase_CPPFLAGS = -DTARGET=@ASITARGET@
testcase_LDADD = @ASILIBRARY@

endif

asi_la_SOURCES = AsiLocator.cpp utils.cpp AsiCamera.cpp AsiCcd.cpp
asi_la_LDFLAGS = -module
asi_la_DEPENDENCIES = ../../lib/libastro.la
asi_la_LIBADD = ../../lib/libastro.la @ASILIBRARY@
asi_la_CPPFLAGS = -DTARGET=@ASITARGET@

endif
