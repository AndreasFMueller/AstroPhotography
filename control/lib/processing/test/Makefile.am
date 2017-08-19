#
# Makefile.am -- tests for image processing functions
# 
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
noinst_HEADERS = 

noinst_DATA = process.xml

test_ldadd = -lcppunit 							\
	-L$(top_builddir)/lib/processing -lastroprocessing		\
	-L$(top_builddir)/lib/image -lastroimage			\
	-L$(top_builddir)/lib/utils -lastroutils
test_dependencies = 							\
	$(top_builddir)/lib/utils/libastroutils.la  			\
	$(top_builddir)/lib/image/libastroimage.la  			\
	$(top_builddir)/lib/processing/libastroprocessing.la  

if ENABLE_UNITTESTS

noinst_PROGRAMS = tests singletest

# single test
singletest_SOURCES = singletest.cpp					\
	ProcessorParserTest.cpp
singletest_LDADD = $(test_ldadd)
singletest_DEPENDENCIES = $(test_dependencies)

single:	singletest
	./singletest -d 2>&1 | tee single.log

## general tests
tests_SOURCES = tests.cpp 
tests_LDADD = $(test_ldadd)
tests_DEPENDENCIES = $(test_dependencies)

test:	tests
	./tests -d 2>&1 | tee test.log

endif
