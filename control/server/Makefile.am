#
# Makefile.am -- build the server
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
bin_PROGRAMS = astrod

EXTRA_DIST = Modules.h

ORBLDFLAGS = -lomniCodeSets4 -lomniConnectionMgmt4 -lomniDynamic4 -lomniORB4 -lomnithread

astrod_SOURCES = Modules.cpp DriverModule.cpp astrod.cpp
astrod_DEPENDENCIES = ../lib/libastro.la
astrod_LDADD = -L../lib -lastro -L../idl -lastrocom @SBIGLIBRARY@  $(ORBLDFLAGS)

test:	astrod
	./astrod -d -ORBInitRef NameService=corbaname::localhost




