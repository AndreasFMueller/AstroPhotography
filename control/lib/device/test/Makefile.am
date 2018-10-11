#
# Makefile -- build the test subdirectory
# 
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
device_ldadd = -lcppunit						\
	-L$(top_builddir)/lib/device -lastrodevice			\
	-L$(top_builddir)/lib/discovery -lastrodiscovery		\
	-L$(top_builddir)/lib/persistence -lastropersistence		\
	-L$(top_builddir)/lib/image -lastroimage			\
	-L$(top_builddir)/lib/utils -lastroutils
device_dependencies =							\
	$(top_builddir)/lib/utils/libastroutils.la			\
	$(top_builddir)/lib/image/libastroimage.la			\
	$(top_builddir)/lib/discovery/libastrodiscovery.la		\
	$(top_builddir)/lib/persistence/libastropersistence.la		\
	$(top_builddir)/lib/device/libastrodevice.la

if ENABLE_UNITTESTS

# stuff related to testing
noinst_PROGRAMS = tests singletest aotest

## general tests
tests_SOURCES = tests.cpp						\
	BinningTest.cpp							\
	DeviceNameTest.cpp						\
	ModuleDescriptorTest.cpp					\
	ModuleTest.cpp							\
	NiceTest.cpp							\
	ParameterDescriptionTest.cpp					\
	RepositoryTest.cpp
tests_LDADD = $(device_ldadd)
tests_DEPENDENCIES = $(device_dependencies)

test:	tests
	./tests -d 2>&1 | tee test.log

# single test
singletest_SOURCES = singletest.cpp 					\
	RepositoryTest.cpp
singletest_LDADD = $(device_ldadd)
singletest_DEPENDENCIES = $(device_dependencies)

single:	singletest
	./singletest -d 2>&1 | tee single.log

# test program for AO units
aotest_SOURCES = aotest.cpp
aotest_LDADD = $(device_ldadd)
aotest_DEPENDENCIES = $(device_dependencies)

endif
