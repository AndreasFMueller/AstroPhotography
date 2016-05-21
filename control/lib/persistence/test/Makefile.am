#
# Makefile.am -- build the persistence tests
# 
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
persistence_dependencies =  						\
	$(top_builddir)/lib/persistence/libastropersistence.la		\
	$(top_builddir)/lib/utils/libastroutils.la

persistence_ldadd = -lcppunit						\
	-L$(top_builddir)/lib/persistence -lastropersistence		\
	-L$(top_builddir)/lib/utils -lastroutils


if ENABLE_UNITTESTS

# stuff related to testing
noinst_PROGRAMS = tests singletest

## general tests
tests_SOURCES = tests.cpp 						\
	ConfigurationTest.cpp						\
	TableTest.cpp							\
	DatabaseTest.cpp 						\
	UpdateSpecTest.cpp
tests_LDADD = $(persistence_ldadd)
tests_DEPENDENCIES = $(persistence_dependencies)

test:	tests
	./tests -d 2>&1 | tee test.log

# single test
singletest_SOURCES = singletest.cpp 					\
	TableTest.cpp
singletest_LDADD = $(persistence_ldadd)
singletest_DEPENDENCIES = $(persistence_dependencies)

single:	singletest
	./singletest -d 2>&1 | tee single.log

endif
