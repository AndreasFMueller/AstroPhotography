#
# Makefile.am -- build the persistence tests
# 
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
persistence_ldadd = -lcppunit						\
	-L$(top_builddir)/lib/persistence -lastropersistence		\
	-L$(top_builddir)/lib/utils -lastroutils
persistence_dependencies =  $(top_builddir)/lib/utils/libastroutils.la	\
	$(top_builddir)/lib/persistence/libastropersistence.la

if ENABLE_UNITTESTS

# stuff related to testing
noinst_PROGRAMS = tests 

## general tests
tests_SOURCES = tests.cpp 						\
	TableTest.cpp							\
	UpdateSpecTest.cpp						\
	DatabaseTest.cpp 
tests_LDADD = $(persistence_ldadd)
tests_DEPENDENCIES = $(persistence_dependencies)

test:	tests
	./tests -d 2>&1 | tee test.log

# single test
singletest_SOURCES = singletest.cpp 					\
	TableTest.cpp
singletest_LDADD = (
singletest_DEPENDENCIES = $(persistence_dependencies)

single:	singletest
	./singletest -d 2>&1 | tee single.log

endif
