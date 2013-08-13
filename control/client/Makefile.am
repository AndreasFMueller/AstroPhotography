#
# Makefile.am -- build the test client
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
bin_PROGRAMS = astroclient

ORBLDFLAGS = -lomniCodeSets4 -lomniConnectionMgmt4 -lomniDynamic4 \
	-lomniORB4 -lomnithread

astroclient_SOURCES = astroclient.cpp
astroclient_DEPENDENCIES = ../lib/libastro.la
astroclient_LDADD = -L../lib -lastro -L../idl -lastrocom @SBIGLIBRARY@ \
	$(ORBLDFLAGS)

test:	astroclient
	./astroclient -d -ORBInitRef NameService=corbaname::localhost



