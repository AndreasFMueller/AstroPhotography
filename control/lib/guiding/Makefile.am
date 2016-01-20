#
# Makefile.am -- build the guiding stuff
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_LTLIBRARIES = libastroguiding.la

libastroguiding_la_SOURCES = 						\
	AdaptiveOpticsCalibration.cpp					\
	AOCalibrationProcess.cpp					\
	BasicCalibration.cpp						\
	BasicCalibrator.cpp						\
	BasicProcess.cpp						\
	BasicSummary.cpp						\
	CalibrationPersistence.cpp					\
	CalibrationProcess.cpp						\
	CalibrationStore.cpp						\
	ControlDevice.cpp						\
	DrivingWork.cpp							\
	Guide.cpp							\
	Guider.cpp							\
	GuiderCalibration.cpp						\
	GuiderDescriptor.cpp						\
	GuiderFactory.cpp						\
	GuiderPortProcess.cpp						\
	GuiderProcess.cpp						\
	GuiderStateMachine.cpp						\
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
