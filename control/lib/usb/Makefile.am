#
# Makefile.am -- build the USB/UVC parts of the library
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = 

noinst_LTLIBRARIES = libastrousb.la

EXTRA_DIST = hid-macos.c hid-libusb.c

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
	DeviceNameUSB.cpp						\
	hidapi.c							\
	USBConfiguration.cpp						\
	USBContext.cpp							\
	USBContextHolder.cpp						\
	USBDebug.cpp							\
	USBDescriptors.cpp						\
	USBDevice.cpp							\
	USBDeviceDescriptor.cpp						\
	USBEndpoint.cpp							\
	USBError.cpp							\
	USBFrame.cpp							\
	USBInterface.cpp						\
	USBIsoTransfer.cpp						\
	USBRawDescriptors.cpp						\
	USBRequests.cpp							\
	USBTransfer.cpp							\
	$(uvc_files)


noinst_HEADERS = USBDebug.h

libastrousb_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" -DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"


