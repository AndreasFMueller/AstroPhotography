#
# Makefile.am -- build catalog tests
# 
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

catalogs_ldadd = -lcppunit						\
	-L$(top_builddir)/lib/catalogs -lastrocatalogs			\
	-L$(top_builddir)/lib/image -lastroimage			\
	-L$(top_builddir)/lib/utils -lastroutils
catalogs_dependencies =							\
	$(top_builddir)/lib/utils/libastroutils.la			\
	$(top_builddir)/lib/image/libastroimage.la			\
	$(top_builddir)/lib/catalogs/libastrocatalogs.la

if ENABLE_UNITTESTS

# stuff related to testing
noinst_PROGRAMS = tests singletest 

## general tests
tests_SOURCES = tests.cpp						\
	BSCTest.cpp							\
	ChartTest.cpp							\
	HipparcosTest.cpp 						\
	ImageNormalizerTest.cpp						\
	NGCICTest.cpp							\
	ProjectionTest.cpp						\
	SkyRectangleTest.cpp						\
	SkyWindowTest.cpp 						\
	StereographicChartTest.cpp 					\
	StereographicProjectionTest.cpp 				\
	Tycho2Test.cpp							\
	Ucac4Test.cpp

tests_LDADD = $(catalogs_ldadd)
tests_DEPENDENCIES = $(catalogs_dependencies)

test:	tests
	./tests -d 2>&1 | tee test.log

# single test
singletest_SOURCES = singletest.cpp 
singletest_LDADD = $(catalogs_ldadd)
singletest_DEPENDENCIES = $(catalogs_dependencies)

endif
