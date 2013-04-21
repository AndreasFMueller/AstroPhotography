#
# build the basic library
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . test

lib_LTLIBRARIES = libastro.la

UNICAP_FILES =
if ENABLE_UNICAP
UNICAP_FILES = UnicapDevice.cpp
endif

libastro_la_SOURCES = debug.cpp Format.cpp \
	Pixel.cpp \
	ImagePoint.cpp ImageSize.cpp ImageRectangle.cpp ImageBase.cpp \
	ImageIteratorBase.cpp ImageLine.cpp Image.cpp \
	Binning.cpp \
	Descriptor.cpp Module.cpp Repository.cpp \
	Ccd.cpp \
	Fits.cpp \
	USBContext.cpp USBDevice.cpp USBDeviceDescriptor.cpp \
	USBDescriptors.cpp USBEndpoint.cpp USBConfiguration.cpp \
	USBInterface.cpp USBRawDescriptors.cpp \
	USBRequests.cpp USBTransfer.cpp USBIsoTransfer.cpp \
	UVCDescriptors.cpp UVCVideoControl.cpp UVCVideoStreaming.cpp \
	UVCFactory.cpp \
	UVCFormat.cpp UVCUncompressed.cpp UVCMJPEG.cpp UVCFrameBased.cpp \
	UVCFrame.cpp UVCCamera.cpp UVCTransfer.cpp UVCFrameFactory.cpp \
	MicroTouch.cpp \
	$(UNICAP_FILES)

libastro_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\"
