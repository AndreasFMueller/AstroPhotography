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
	Binning.cpp							\
	Camera.cpp							\
	Ccd.cpp								\
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
	Module.cpp							\
	ModuleDescriptor.cpp						\
	Mount.cpp							\
	Properties.cpp							\
	Repository.cpp							\
	Serial.cpp							\
	Shutter.cpp

libastrodevice_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"
