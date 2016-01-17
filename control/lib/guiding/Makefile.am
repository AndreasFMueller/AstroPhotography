#
# Makefile.am -- build the guiding stuff
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_LTLIBRARIES = libastroguiding.la

libastroguiding_la_SOURCES = 						\
	BasicCalibration.cpp						\
	BasicCalibrator.cpp						\
	BasicSummary.cpp						\
	CalibrationPersistence.cpp					\
	CalibrationProcess.cpp						\
	CalibrationStore.cpp						\
	DrivingWork.cpp							\
	Guide.cpp							\
	Guider.cpp							\
	GuiderCalibration.cpp						\
	GuiderFactory.cpp						\
	GuiderProcess.cpp						\
	GuiderStateMachine.cpp						\
	GuidingProcess.cpp						\
	SaveImageCallback.cpp						\
	StarDetectorBase.cpp						\
	Tracker.cpp							\
	TrackingPersistence.cpp						\
	TrackingPoint.cpp						\
	TrackingStore.cpp						\
	TrackingSummary.cpp						\
	TrackingWork.cpp

libastroguiding_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" 
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"
