#
# Makefile.am -- build the guiding stuff
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_LTLIBRARIES = libastroguiding.la

noinst_HEADERS = 							\
	AOCalibrationProcess.h						\
	BasicProcess.h							\
	CalibrationPersistence.h					\
	CalibrationProcess.h						\
	CalibrationStore.h						\
	ControlDeviceCallback.h						\
	GuiderCalibrationRedirector.h					\
	GuiderPortAction.h						\
	TrackingPersistence.h						\
	TrackingProcess.h						\
	TrackingStore.h

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
	ControlDeviceCallback.cpp					\
	ControlDeviceBase.cpp						\
	Guide.cpp							\
	Guider.cpp							\
	GuiderBase.cpp							\
	GuiderCalibration.cpp						\
	GuiderCalibrationRedirector.cpp					\
	GuiderDescriptor.cpp						\
	GuiderFactory.cpp						\
	GuiderPortAction.cpp						\
	GuiderPortProcess.cpp						\
	GuiderStateMachine.cpp						\
	LargeTracker.cpp						\
	NullTracker.cpp							\
	RefreshingTracker.cpp						\
	SaveImageCallback.cpp						\
	StarDetectorBase.cpp						\
	StarTracker.cpp							\
	Tracker.cpp							\
	TrackingPersistence.cpp						\
	TrackingPoint.cpp						\
	TrackingProcess.cpp						\
	TrackingStore.cpp						\
	TrackingSummary.cpp						

libastroguiding_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" 
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"
