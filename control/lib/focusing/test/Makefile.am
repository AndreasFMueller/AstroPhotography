#
# Makefile.am -- tests for focusing
# 
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
noinst_HEADERS = 

focusing_ldadd = -lcppunit 						\
	-L$(top_builddir)/lib/focusing -lastrofocusing 			\
	-L$(top_builddir)/lib/device -lastrodevice 			\
	-L$(top_builddir)/lib/image -lastroimage -lastrofits 		\
	-L$(top_builddir)/lib/persistence -lastropersistence		\
	-L$(top_builddir)/lib/utils -lastroutils
focusing_dependencies = 						\
	$(top_builddir)/lib/utils/libastroutils.la  			\
	$(top_builddir)/lib/image/libastroimage.la  			\
	$(top_builddir)/lib/image/libastrofits.la			\
	$(top_builddir)/lib/persistence/libastropersistence.la		\
	$(top_builddir)/lib/device/libastrodevice.la  			\
	$(top_builddir)/lib/focusing/libastrofocusing.la 

if ENABLE_UNITTESTS

noinst_PROGRAMS = tests singletest

# single test
singletest_SOURCES = singletest.cpp					\
	FocusComputeTest.cpp
singletest_LDADD = $(focusing_ldadd)
singletest_DEPENDENCIES = $(focusing_dependencies)

single:	singletest
	./singletest -d 2>&1 | tee single.log

## general tests
tests_SOURCES = tests.cpp						\
	FocusComputeTest.cpp
tests_LDADD = $(focusing_ldadd)
tests_DEPENDENCIES = $(focusing_dependencies)

test:	tests
	./tests -d 2>&1 | tee test.log

endif
