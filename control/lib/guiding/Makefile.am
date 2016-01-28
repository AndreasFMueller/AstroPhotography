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
	CalibrationPoint.cpp						\
	CalibrationProcess.cpp						\
	CalibrationStore.cpp						\
	ControlDevice.cpp						\
	Guide.cpp							\
	Guider.cpp							\
	GuiderBase.cpp							\
	GuiderCalibration.cpp						\
	GuiderDescriptor.cpp						\
	GuiderFactory.cpp						\
	GuiderPortProcess.cpp						\
	GuiderStateMachine.cpp						\
	NullTracker.cpp							\
	RefreshingTracker.cpp						\
	SaveImageCallback.cpp						\
	StarDetectorBase.cpp						\
	StarTracker.cpp							\
	PhaseTracker.cpp						\
	Tracker.cpp							\
	TrackingPersistence.cpp						\
	TrackingPoint.cpp						\
	TrackingProcess.cpp						\
	TrackingStore.cpp						\
	TrackingSummary.cpp						

libastroguiding_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" 
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"
