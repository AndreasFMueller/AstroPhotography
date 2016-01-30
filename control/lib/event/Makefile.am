#
# Makefile.am -- subdirectory for event handling functions
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
#SUBDIRS = . test

noinst_LTLIBRARIES = libastroevent.la

libastroevent_la_SOURCES = 						\
	EventHandler.cpp						\
	EventPersistence.cpp

libastroevent_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

