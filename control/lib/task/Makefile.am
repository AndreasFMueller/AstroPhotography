#
# build the basic library
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_LTLIBRARIES = libastrotask.la

libastrotask_la_SOURCES = 						\
	CancellableWork.cpp						\
	DitherWork.cpp							\
	ExposureTimer.cpp						\
	ExposureWork.cpp						\
	Loop.cpp							\
	SleepWork.cpp							\
	TaskExecutor.cpp						\
	TaskInfo.cpp							\
	TaskParameters.cpp						\
	TaskQueue.cpp							\
	TaskQueueEntry.cpp						\
	TaskTable.cpp							\
	tasktype.cpp

libastrotask_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" 		\
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

