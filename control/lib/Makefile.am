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
	Point.cpp Pixel.cpp MosaicType.cpp Coordinates.cpp \
	ImagePoint.cpp ImageSize.cpp ImageRectangle.cpp ImageBase.cpp \
	ImageIteratorBase.cpp ImageLine.cpp Image.cpp ImageProperties.cpp \
	Metavalue.cpp Sun.cpp Viewer.cpp Background.cpp Histogram.cpp \
	Subgrid.cpp Transform.cpp Adapters.cpp DisplayConverter.cpp \
	PhaseCorrelator.cpp TransformAnalyzer.cpp Stacker.cpp \
	Guiding.cpp GuiderProcess.cpp Simulator.cpp \
	Exposure.cpp Binning.cpp ExposureTimer.cpp Loop.cpp \
	ImageProgramCallback.cpp \
	ModuleDescriptor.cpp Module.cpp DeviceLocator.cpp Repository.cpp \
	Camera.cpp Ccd.cpp Cooler.cpp FilterWheel.cpp GuiderPort.cpp \
	Imager.cpp FITS.cpp FITSout.cpp FITSin.cpp FITSdirectory.cpp \
	Demosaic.cpp Filters.cpp Masks.cpp Clamper.cpp Interpolation.cpp \
	Corrector.cpp DarkCorrector.cpp FlatCorrector.cpp \
	CalibrationFactory.cpp Focuser.cpp Focusing.cpp \
	USBContextHolder.cpp USBContext.cpp \
	USBDevice.cpp USBDeviceDescriptor.cpp \
	USBDescriptors.cpp USBEndpoint.cpp USBConfiguration.cpp \
	USBFrame.cpp USBInterface.cpp USBRawDescriptors.cpp \
	USBRequests.cpp USBTransfer.cpp USBIsoTransfer.cpp \
	Conversions.cpp NameService.cpp OrbSingleton.cpp deviceSK.cc \
	$(uvc_files) \
	$(unicap_flies)

libastro_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\"
