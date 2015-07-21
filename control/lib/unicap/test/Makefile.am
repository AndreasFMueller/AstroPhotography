#
# Makefile -- build the test subdirectory
# 
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
noinst_HEADERS = 
usb_ldadd = -lcppunit 							\
	-L$(top_builddir)/lib/unicap -lastrounicap 			\
	-L$(top_builddir)/lib/utils -lastroutils
usb_dependencies = 							\
	../libastrounicap.la						\
	$(top_builddir)/lib/utils/libastroutils.la

if ENABLE_UNITTESTS

# stuff related to testing
noinst_PROGRAMS = singletest tests

# single test
singletest_SOURCES = singletest.cpp
singletest_LDADD = $(usb_ldadd)
singletest_DEPENDENCIES = $(usb_dependencies)

single:	singletest
	./singletest -d 2>&1 | tee single.log

# tests for UVC cameras
if ENABLE_UNICAP
unicaptests_SOURCES = $(unicap_tests)
unicaptests_LDADD = $(unicap_ldadd)
unicaptests_DEPENDENCIES = $(unicap_dependencies)

unicaptest:	unicaptests
	./unicaptests -d
endif

endif
