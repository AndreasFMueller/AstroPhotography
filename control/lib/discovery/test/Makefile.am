#
# Makefile.am -- Test service publishing and discovery
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
if ENABLE_UNITTESTS
test_programs = tests singletest
else
test_programs =
endif

noinst_PROGRAMS = $(test_programs) sdiscover spublish

discover_ldadd = 							\
	-L$(top_builddir)/lib/discovery -lastrodiscovery		\
	-L$(top_builddir)/lib/persistence -lastropersistence		\
	-L$(top_builddir)/lib/utils -lastroutils 
discover_dependencies =  						\
	$(top_builddir)/lib/utils/libastroutils.la 			\
	$(top_builddir)/lib/persistence/libastropersistence.la 		\
	$(top_builddir)/lib/discovery/libastrodiscovery.la

if ENABLE_UNITTESTS

# single test
singletest_SOURCES = singletest.cpp \
	URLTest.cpp
singletest_LDADD = -lcppunit $(discover_ldadd)
singletest_DEPENDENCIES = $(discover_dependencies)

.PHONY:	single
single:	singletest
	./singletest -d 2>&1 | tee single.log

tests_SOURCES = tests.cpp 						\
	ServiceKeyTest.cpp						\
	ServiceSubsetTest.cpp						\
	URLTest.cpp

tests_DEPENDENCIES = $(discovery_dependencies)
tests_LDADD = -lcppunit $(discover_ldadd)

test:	tests
	./tests -d 2>&1 | tee test.log

endif

sdiscover_SOURCES = sdiscover.cpp
sdiscover_DEPENDENCIES = $(discover_dependencies)
sdiscover_LDADD = $(discover_ldadd)

spublish_SOURCES = spublish.cpp
spublish_DEPENDENCIES = $(discover_dependencies)
spublish_LDADD = $(discover_ldadd)

