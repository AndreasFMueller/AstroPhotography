#
# Makefile.am -- tests for guiding
# 
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
noinst_HEADERS = 

guiding_ldadd = -lcppunit 						\
	-L$(top_builddir)/lib/guiding -lastroguiding 			\
	-L$(top_builddir)/lib/device -lastrodevice 			\
	-L$(top_builddir)/lib/image -lastroimage 			\
	-L$(top_builddir)/lib/event -lastroevent			\
	-L$(top_builddir)/lib/discovery -lastrodiscovery 		\
	-L$(top_builddir)/lib/persistence -lastropersistence		\
	-L$(top_builddir)/lib/utils -lastroutils
guiding_dependencies = 							\
	$(top_builddir)/lib/utils/libastroutils.la  			\
	$(top_builddir)/lib/discovery/libastrodiscovery.la		\
	$(top_builddir)/lib/event/libastroevent.la  			\
	$(top_builddir)/lib/image/libastroimage.la  			\
	$(top_builddir)/lib/persistence/libastropersistence.la		\
	$(top_builddir)/lib/device/libastrodevice.la  			\
	$(top_builddir)/lib/guiding/libastroguiding.la 

if ENABLE_UNITTESTS

noinst_PROGRAMS = tests singletest

# single test
singletest_SOURCES = singletest.cpp					\
	BacklashAnalysisTest.cpp
singletest_LDADD = $(guiding_ldadd)
singletest_DEPENDENCIES = $(guiding_dependencies)

single:	singletest
	./singletest -d 2>&1 | tee single.log

## general tests
tests_SOURCES = tests.cpp						\
	BacklashAnalysisTest.cpp					\
	GuiderFactoryTest.cpp						\
	StarDetectorTest.cpp
tests_LDADD = $(guiding_ldadd)
tests_DEPENDENCIES = $(guiding_dependencies)

test:	tests
	./tests -d 2>&1 | tee test.log

endif
