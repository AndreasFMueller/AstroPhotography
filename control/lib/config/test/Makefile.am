#
# Makefile.am -- tests for images
# 
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
noinst_HEADERS = 

test_ldadd = -lcppunit 							\
	-L$(top_builddir)/lib/config -lastroconfig			\
	-L$(top_builddir)/lib/device -lastrodevice			\
	-L$(top_builddir)/lib/image -lastroimage			\
	-L$(top_builddir)/lib/discovery -lastrodiscovery		\
	-L$(top_builddir)/lib/persistence -lastropersistence		\
	-L$(top_builddir)/lib/utils -lastroutils
test_dependencies = 							\
	$(top_builddir)/lib/utils/libastroutils.la  			\
	$(top_builddir)/lib/persistence/libastropersistence.la 		\
	$(top_builddir)/lib/discovery/libastrodiscovery.la 		\
	$(top_builddir)/lib/image/libastroimage.la  			\
	$(top_builddir)/lib/device/libastrodevice.la  			\
	$(top_builddir)/lib/config/libastroconfig.la  

if ENABLE_UNITTESTS

noinst_PROGRAMS = tests singletest

# single test
singletest_SOURCES = singletest.cpp \
	DeviceMapperTest.cpp
singletest_LDADD = $(test_ldadd)
singletest_DEPENDENCIES = $(test_dependencies)

single:	singletest
	./singletest -d 2>&1 | tee single.log

## general tests
tests_SOURCES = tests.cpp 						\
	DeviceMapTableTest.cpp						\
	DeviceMapperTest.cpp						\
	ImageRepoTableTest.cpp						\
	ImageRepoTablesTest.cpp						\
	ImageRepoTest.cpp						\
	ProjectTableTest.cpp

#	InstrumentTest.cpp

tests_LDADD = $(test_ldadd)
tests_DEPENDENCIES = $(test_dependencies)

test:	tests
	./tests -d 2>&1 | tee test.log

endif
