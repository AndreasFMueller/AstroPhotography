#
# Makefile.am -- tests for guiding
# 
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
noinst_HEADERS = 

guiding_ldadd = -lcppunit 						\
	-L$(top_builddir)/lib/guiding -lastroguiding 			\
	-L$(top_builddir)/lib/device -lastrodevice 			\
	-L$(top_builddir)/lib/image -lastroimage -lastrofits 		\
	-L$(top_builddir)/lib/persistence -lastropersistence		\
	-L$(top_builddir)/lib/utils -lastroutils
guiding_dependencies = 							\
	$(top_builddir)/lib/utils/libastroutils.la  			\
	$(top_builddir)/lib/image/libastroimage.la  			\
	$(top_builddir)/lib/image/libastrofits.la			\
	$(top_builddir)/lib/persistence/libastropersistence.la		\
	$(top_builddir)/lib/device/libastrodevice.la  			\
	$(top_builddir)/lib/guiding/libastroguiding.la 

if ENABLE_UNITTESTS

noinst_PROGRAMS = tests singletest guidetest

# single test
singletest_SOURCES = singletest.cpp
singletest_LDADD = $(guiding_ldadd)
singletest_DEPENDENCIES = $(guiding_dependencies)

single:	singletest
	./singletest -d 2>&1 | tee single.log

## general tests
tests_SOURCES = tests.cpp						\
	GuiderFactoryTest.cpp						\
	StarDetectorTest.cpp
tests_LDADD = $(guiding_ldadd)
tests_DEPENDENCIES = $(guiding_dependencies)

test:	tests
	./tests -d 2>&1 | tee test.log

# guide test
guidetest_SOURCES = guidetest.cpp
guidetest_LDADD = $(guiding_ldadd)
guidetest_DEPENDENCIES = $(guiding_dependencies)

testguide:	guidetest
	rm -f gt/*
	./guidetest -d -s "(380,152)" -r "64x64@(348,120)" -t 200 -T 3600 -i gt -C 2>&1 | tee testguide.log

endif
