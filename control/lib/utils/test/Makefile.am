#
# Makefile -- build the test subdirectory
# 
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
noinst_HEADERS = 

test_ldadd = -lcppunit -L.. -lastroutils
test_dependencies = ../libastroutils.la

if ENABLE_UNITTESTS

noinst_PROGRAMS = tests singletest

# single test
singletest_SOURCES = singletest.cpp 					\
	SyncQueueTest.cpp
singletest_LDADD = $(test_ldadd)
singletest_DEPENDENCIES = $(test_dependencies)

single:	singletest
	./singletest -d 2>&1 | tee single.log

## general tests
tests_SOURCES = tests.cpp 						\
	AngleTest.cpp							\
	ConcatenatorTest.cpp 						\
	JulianDateTest.cpp						\
	MedianTest.cpp							\
	PathTest.cpp 							\
	SplitterTest.cpp						\
	StacktraceTest.cpp 						\
	SyncQueueTest.cpp						\
	TrimTest.cpp

tests_LDADD = $(test_ldadd)
tests_DEPENDENCIES = $(test_dependencies)

test:	tests
	./tests -d 

endif
