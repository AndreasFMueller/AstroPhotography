#
# Makefile.am -- subdirectory of unicap stuff
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_HEADERS =

noinst_LTLIBRARIES = libastrounicap.la

if ENABLE_UNICAP
unicap_files =
	UnicapDevice.cpp
else
unicap_files =
endif

libastrounicap_la_SOURCES = UnicapDummy.cpp $(unicap_files)

libastrounicap_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

