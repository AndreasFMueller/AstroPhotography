#
# Makefile -- build the test subdirectory
# 
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
noinst_HEADERS = 

if ENABLE_UNITTESTS

# stuff related to testing
noinst_PROGRAMS = tests processtests singletest calimggen caltests phasecorr

## general tests
tests_SOURCES = tests.cpp

# single test
singletest_SOURCES = singletest.cpp 
singletest_LDADD = -lcppunit -L.. -lastro 
singletest_DEPENDENCIES = ../libastro.la 

single:	singletest
	./singletest -d 2>&1 | tee single.log

# tests related to the processing engine. These tests include stuff
processtests_SOURCES = processtests.cpp 
processtests_LDADD = -lcppunit -L.. -lastro 
processtests_DEPENDENCIES = ../libastro.la 

# test for the image processing stuff (takes time)
tests_LDADD = -lcppunit -L.. -lastro 
tests_DEPENDENCIES = ../libastro.la 

calimggen_SOURCES = calimggen.cpp
calimggen_LDADD = -L.. -lastro
calimggen_DEPENDENCIES = ../libastro.la

caltests_SOURCES = caltests.cpp
caltests_LDADD = -lcppunit -L.. -lastro
caltests_DEPENDENCIES = ../libastro.la

#simtest_SOURCES = simtest.cpp
#simtest_LDADD = -L.. -lastro
#simtest_DEPENDENCIES = ../libastro.la

# test program for phase correlation of images
phasecorr_SOURCES = phasecorr.cpp
phasecorr_LDADD = -L.. -lastro
phasecorr_DEPENDENCIES = ../libastro.la

# test2_SOURCES = test2.cpp
# test2_LDADD = -L.. -lastro
# test2_DEPENDENCIES = ../libastro.la

EXTRA_DIST = test_module.la test_module.so

#all-am:	tests usbtests 
#	./usbtests

testguide:	guidetest
	rm -f gt/*
	./guidetest -d -s "(380,152)" -r "64x64@(348,120)" -t 200 -T 3600 -i gt -C 2>&1 | tee testguide.log

endif
