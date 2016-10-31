#
# build the basic library
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . test

noinst_LTLIBRARIES = libastrotask.la

libastrotask_la_SOURCES = 						\
	CancellableWork.cpp						\
	ExposureTimer.cpp						\
	ExposureWork.cpp						\
	Loop.cpp							\
	TaskExecutor.cpp						\
	TaskInfo.cpp							\
	TaskParameters.cpp						\
	TaskQueue.cpp							\
	TaskQueueEntry.cpp						\
	TaskTable.cpp

libastrotask_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" 		\
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

