#
# build the basic library
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . test

noinst_LTLIBRARIES = libastrodevice.la

libastrodevice_la_SOURCES =						\
	AdaptiveOptics.cpp						\
	BasicGuiderport.cpp						\
	BinningSet.cpp							\
	Camera.cpp							\
	Ccd.cpp								\
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
	GuiderPort.cpp							\
	Imager.cpp							\
	ImageQueue.cpp							\
	ImageQueueEntry.cpp						\
	Module.cpp							\
	ModuleDescriptor.cpp						\
	Mount.cpp							\
	ParameterDescription.cpp					\
	Properties.cpp							\
	Repository.cpp							\
	Serial.cpp							\
	Shutter.cpp

libastrodevice_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"
