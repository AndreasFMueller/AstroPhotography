#
# Makefile.am -- build the atik driver, based on the CloudMakers library
#
# (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_ATIK

noinst_HEADERS = AtikLocator.h AtikUtils.h AtikCamera.h AtikCcd.h	\
	AtikCooler.h AtikFilterwheel.h AtikGuideport.h
pkglib_LTLIBRARIES = atik.la 

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libatik.la

libatik_la_SOURCES = AtikLocator.cpp AtikUtils.cpp AtikCamera.cpp	\
	AtikCooler.cpp AtikCcd.cpp AtikFilterwheel.cpp AtikGuideport.cpp
libatik_la_DEPENDENCIES = ../../lib/libastro.la
libatik_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = atiktests 
atiktests_SOURCES = atiktests.cpp atiktest.cpp
atiktests_LDADD = -lcppunit ../../lib/libastro.la -L. -latik @ATIKLIBPATH@ @ATIKLIBS@
atiktests_DEPENDENCIES = ../../lib/libastro.la libatik.la

endif

atik_la_SOURCES = AtikLocator.cpp AtikUtils.cpp AtikCamera.cpp		\
	AtikCooler.cpp AtikCcd.cpp AtikFilterwheel.cpp AtikGuideport.cpp
atik_la_LDFLAGS = -module
atik_la_DEPENDENCIES = ../../lib/libastro.la
atik_la_LIBADD = ../../lib/libastro.la @ATIKLIBPATH@ @ATIKLIBS@

endif
