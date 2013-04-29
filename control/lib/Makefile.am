#
# build the basic library
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = . test

lib_LTLIBRARIES = libastro.la

if ENABLE_UNICAP
unicap_flies = UnicapDevice.cpp
else
unicap_flies =
endif

if ENABLE_MICROTOUCH
microtouch_files = MicroTouch.cpp
else
microtouch_files =
endif

if ENABLE_UVC
uvc_files = \
	UVCDescriptors.cpp UVCVideoControl.cpp UVCVideoStreaming.cpp \
	UVCFactory.cpp \
	UVCFormat.cpp UVCUncompressed.cpp UVCMJPEG.cpp UVCFrameBased.cpp \
	UVCCamera.cpp UVCTransfer.cpp UVCFrameFactory.cpp 
else
uvc_files =
endif

libastro_la_SOURCES = debug.cpp Format.cpp Timer.cpp \
	Pixel.cpp \
	ImagePoint.cpp ImageSize.cpp ImageRectangle.cpp ImageBase.cpp \
	ImageIteratorBase.cpp ImageLine.cpp Image.cpp \
	Exposure.cpp Binning.cpp \
	Descriptor.cpp Module.cpp Repository.cpp \
	Ccd.cpp \
	Fits.cpp \
	USBContext.cpp USBDevice.cpp USBDeviceDescriptor.cpp \
	USBDescriptors.cpp USBEndpoint.cpp USBConfiguration.cpp \
	USBFrame.cpp USBInterface.cpp USBRawDescriptors.cpp \
	USBRequests.cpp USBTransfer.cpp USBIsoTransfer.cpp \
	$(uvc_files) \
	$(unicap_flies) $(microtouch_files)

libastro_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\"
