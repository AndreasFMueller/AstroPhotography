#
# global tests, some of which can even only be execute after the whole thing
# has been installed locally
#
if ENABLE_UNITTESTS

noinst_HEADERS =

noinst_PROGRAMS = tests postinstall_tests loadtest

tests_SOURCES = tests.cpp  Mock1Test.cpp
postinstall_tests_SOURCES = postinstall_tests.cpp RepositoryTest.cpp
loadtest_SOURCES = loadtest.cpp

LDADD = -lcppunit -L../lib -lcorbastro -lastro
DEPENDENCIES = ../lib/libastro.a ../lib/libcorbastro.a

all-am: tests
	./tests -d

test:	postinstall_tests
	./postinstall_tests

endif
