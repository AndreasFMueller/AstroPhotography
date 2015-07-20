#
# Makefile.am -- build the USB/UVC parts of the library
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = 

lib_LTLIBRARIES = libastrousb.la

if ENABLE_UVC
uvc_files = 								\
	UVCCamera.cpp							\
	UVCDescriptors.cpp						\
	UVCFactory.cpp							\
	UVCFormat.cpp							\
	UVCFrameBased.cpp						\
	UVCFrameFactory.cpp						\
	UVCMJPEG.cpp							\
	UVCTransfer.cpp							\
	UVCUncompressed.cpp						\
	UVCVideoControl.cpp						\
	UVCVideoStreaming.cpp
else
uvc_files =
endif

libastrousb_la_SOURCES = 						\
	USBConfiguration.cpp						\
	USBContext.cpp							\
	USBContextHolder.cpp						\
	USBDescriptors.cpp						\
	USBDevice.cpp							\
	USBDeviceDescriptor.cpp						\
	USBEndpoint.cpp							\
	USBFrame.cpp							\
	USBInterface.cpp						\
	USBIsoTransfer.cpp						\
	USBRawDescriptors.cpp						\
	USBRequests.cpp							\
	USBTransfer.cpp							\
	$(uvc_files)

libastrousb_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" -DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"


