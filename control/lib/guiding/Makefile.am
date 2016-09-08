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
	CalibrationRedirector.h						\
	ControlDeviceCallback.h						\
	GPCalibrationProcess.h						\
	GuidePortAction.h						\
	TrackingPersistence.h						\
	TrackingProcess.h

libastroguiding_la_SOURCES = 						\
	AOCalibrationProcess.cpp					\
	AdaptiveOpticsCalibration.cpp					\
	BasicCalibration.cpp						\
	BasicProcess.cpp						\
	BasicSummary.cpp						\
	CalibrationPersistence.cpp					\
	CalibrationPoint.cpp						\
	CalibrationProcess.cpp						\
	CalibrationRedirector.cpp					\
	CalibrationStore.cpp						\
	ControlDevice.cpp						\
	ControlDeviceBase.cpp						\
	ControlDeviceCallback.cpp					\
	ControlDeviceName.cpp						\
	ControlDeviceType.cpp						\
	GPCalibrationProcess.cpp					\
	Guide.cpp							\
	Guider.cpp							\
	GuiderBase.cpp							\
	GuiderCalibration.cpp						\
	GuiderDescriptor.cpp						\
	GuiderFactory.cpp						\
	GuiderName.cpp							\
	GuidePortAction.cpp						\
	GuidePortProcess.cpp						\
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
