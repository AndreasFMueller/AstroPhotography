#
# Makefile.am -- tests for event handler
# 
# (c) 2016 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
noinst_HEADERS = 

event_ldadd = -lcppunit 						\
	-L$(top_builddir)/lib/device -lastrodevice			\
	-L$(top_builddir)/lib/discovery -lastrodiscovery		\
	-L$(top_builddir)/lib/persistence -lastropersistence		\
	-L$(top_builddir)/lib/event -lastroevent			\
	-L$(top_builddir)/lib/utils -lastroutils
event_dependencies = 							\
	$(top_builddir)/lib/utils/libastroutils.la  			\
	$(top_builddir)/lib/device/libastrodevice.la  			\
	$(top_builddir)/lib/persistence/libastropersistence.la  	\
	$(top_builddir)/lib/event/libastroevent.la			\
	$(top_builddir)/lib/discovery/libastrodiscovery.la 

if ENABLE_UNITTESTS

noinst_PROGRAMS = singletest

# single test
singletest_SOURCES = singletest.cpp 					\
	EventHandlerTest.cpp
singletest_LDADD = $(event_ldadd)
singletest_DEPENDENCIES = $(event_dependencies)

single:	singletest
	./singletest -d -c testdb.db 2>&1 | tee single.log

endif
