#
# Makefile.am -- build the atik driver, based on the CloudMakers library
#
# (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_ATIK

noinst_HEADERS = AtikLocator.h
pkglib_LTLIBRARIES = atik.la 

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libatik.la

libatik_la_SOURCES = AtikLocator.cpp
libatik_la_DEPENDENCIES = ../../lib/libastro.la
libatik_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = atiktests 
atiktests_SOURCES = atiktests.cpp atiktest.cpp
atiktests_LDADD = -lcppunit ../../lib/libastro.la -L. -latik @ATIKLIBPATH@ @ATIKLIBS@
atiktests_DEPENDENCIES = ../../lib/libastro.la libatik.la

testcase_SOURCES = testcase.cpp
testcase_LDADD = @ATIKLIBRARY@

endif

atik_la_SOURCES = AtikLocator.cpp
atik_la_LDFLAGS = -module
atik_la_DEPENDENCIES = ../../lib/libastro.la
atik_la_LIBADD = ../../lib/libastro.la @ATIKLIBPATH@ @ATIKLIBS@

endif
