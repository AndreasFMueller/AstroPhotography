#
# Makefile.am -- tests for task
# 
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
noinst_HEADERS = 

task_ldadd = -lcppunit 							\
	-L$(top_builddir)/lib/task -lastrotask				\
	-L$(top_builddir)/lib/device -lastrodevice			\
	-L$(top_builddir)/lib/image -lastroimage 			\
	-L$(top_builddir)/lib/discovery -lastrodiscovery		\
	-L$(top_builddir)/lib/persistence -lastropersistence		\
	-L$(top_builddir)/lib/utils -lastroutils
task_dependencies = 							\
	$(top_builddir)/lib/utils/libastroutils.la  			\
	$(top_builddir)/lib/task/libastrotask.la  			\
	$(top_builddir)/lib/device/libastrodevice.la  			\
	$(top_builddir)/lib/persistence/libastropersistence.la  	\
	$(top_builddir)/lib/discovery/libastrodiscovery.la  		\
	$(top_builddir)/lib/image/libastroimage.la  

if ENABLE_UNITTESTS

noinst_PROGRAMS = tests singletest tasktest

# single test
singletest_SOURCES = singletest.cpp 
singletest_LDADD = $(task_ldadd)
singletest_DEPENDENCIES = $(task_dependencies)

single:	singletest
	./singletest -d 2>&1 | tee single.log

# task test
tasktest_SOURCES = tasktest.cpp 
tasktest_LDADD = $(task_ldadd)
tasktest_DEPENDENCIES = $(task_dependencies)

## general tests
tests_SOURCES = tests.cpp 
tests_LDADD = $(task_ldadd)
tests_DEPENDENCIES = $(task_dependencies)

test:	tests
	./tests -d 2>&1 | tee test.log

endif
