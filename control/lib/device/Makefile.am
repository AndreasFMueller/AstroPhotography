#
# build the basic library
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . test

noinst_HEADERS = ImageStreamThread.h

noinst_LTLIBRARIES = libastrodevice.la

libastrodevice_la_SOURCES =						\
	AdaptiveOptics.cpp						\
	BasicGuideport.cpp						\
	BinningSet.cpp							\
	Camera.cpp							\
	Ccd.cpp								\
	CcdInfo.cpp							\
	CcdState.cpp							\
	Cooler.cpp							\
	Device.cpp							\
	DeviceAccessor.cpp						\
	DeviceDenicer.cpp						\
	DeviceLocator.cpp						\
	DeviceName.cpp							\
	DeviceNicer.cpp							\
	Devices.cpp							\
	Exposure.cpp							\
	FilterWheel.cpp							\
	Focuser.cpp							\
	GuidePort.cpp							\
	Imager.cpp							\
	ImageQueue.cpp							\
	ImageQueueEntry.cpp						\
	ImageStream.cpp							\
	ImageStreamThread.cpp						\
	Module.cpp							\
	ModuleDescriptor.cpp						\
	Mount.cpp							\
	ParameterDescription.cpp					\
	Properties.cpp							\
	Repository.cpp							\
	Serial.cpp							\
	Shutter.cpp							\
	ThreadCcd.cpp

libastrodevice_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"
