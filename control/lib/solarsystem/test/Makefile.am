#
# Makefile -- build the test subdirectory
# 
# (c) 2020 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
noinst_HEADERS = 

test_ldadd = -lcppunit							\
	-L$(top_builddir)/lib/utils -lastroutils			\
	-L$(top_builddir)/lib/solarsystem -lastrosolarsystem
test_dependencies = 							\
	$(top_builddir)/lib/utils/libastroutils.la			\
	$(top_builddir)/lib/solarsystem/libastrosolarsystem.la

if ENABLE_UNITTESTS

noinst_PROGRAMS = tests singletest

# single test
singletest_SOURCES = singletest.cpp 					\
	MoonTest.cpp
singletest_LDADD = $(test_ldadd)
singletest_DEPENDENCIES = $(test_dependencies)

single:	singletest
	./singletest -d 2>&1 | tee single.log

## general tests
tests_SOURCES = tests.cpp 						\
	SunTest.cpp

tests_LDADD = $(test_ldadd)
tests_DEPENDENCIES = $(test_dependencies)

test:	tests
	./tests -d 

endif
