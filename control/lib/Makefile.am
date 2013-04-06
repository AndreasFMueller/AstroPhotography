#
# build the basic library
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . test

lib_LTLIBRARIES = libastro.la

libastro_la_SOURCES = debug.cpp Format.cpp \
	Pixel.cpp \
	ImagePoint.cpp ImageSize.cpp ImageRectangle.cpp ImageBase.cpp \
	ImageIteratorBase.cpp ImageLine.cpp Image.cpp \
	Binning.cpp \
	Descriptor.cpp Module.cpp Repository.cpp \
	Ccd.cpp \
	Fits.cpp \
	USBContext.cpp USBDevice.cpp USBDeviceDescriptor.cpp \
	USBDeviceHandle.cpp USBDescriptors.cpp USBRawDescriptors.cpp \
	USBRequests.cpp USBTransfer.cpp \
	UVCDescriptors.cpp UVCVideoControl.cpp UVCVideoStreaming.cpp \
	UVCFactory.cpp \
	UVCFormat.cpp UVCUncompressed.cpp UVCMJPEG.cpp UVCFrameBased.cpp \
	UVCFrame.cpp UVCCamera.cpp

libastro_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\"
