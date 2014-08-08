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

libastro_la_SOURCES = debug.cpp utils.cpp \
	Format.cpp Timer.cpp Concatenator.cpp Exceptions.cpp \
	URL.cpp DeviceName.cpp Device.cpp Properties.cpp \
	Point.cpp Pixel.cpp MosaicType.cpp Coordinates.cpp Vector.cpp \
	ImagePoint.cpp ImageSize.cpp ImageRectangle.cpp ImageBase.cpp \
	ImageIteratorBase.cpp ImageLine.cpp Image.cpp ImageProperties.cpp \
	ImageDirectory.cpp ImageDatabaseDirectory.cpp \
	Metavalue.cpp Sun.cpp ViewerPipeline.cpp Viewer.cpp \
	Functions.cpp Background.cpp \
	Histogram.cpp \
	Subgrid.cpp Adapters.cpp DisplayConverter.cpp \
	Transform.cpp Projection.cpp Residual.cpp ProjectionCorrector.cpp \
	PhaseCorrelator.cpp Analyzer.cpp TransformAnalyzer.cpp Stacker.cpp \
	Tracker.cpp Guider.cpp ThreadBase.cpp \
	GuidingProcess.cpp GuiderStateMachine.cpp \
	GuiderProcess.cpp CalibrationProcess.cpp \
	TrackingWork.cpp DrivingWork.cpp \
	GuiderCalibrator.cpp GuiderCalibration.cpp \
	GuiderFactory.cpp \
	Simulator.cpp \
	Exposure.cpp Binning.cpp ExposureTimer.cpp Loop.cpp \
	ImageProgramCallback.cpp SaveImageCallback.cpp \
	ModuleDescriptor.cpp Module.cpp DeviceLocator.cpp Repository.cpp \
	DeviceAccessor.cpp Devices.cpp \
	Camera.cpp Ccd.cpp Cooler.cpp FilterWheel.cpp GuiderPort.cpp \
	AdaptiveOptics.cpp \
	Mount.cpp \
	Serial.cpp \
	Imager.cpp FITS.cpp FITSout.cpp FITSin.cpp FITSdirectory.cpp \
	Demosaic.cpp Filters.cpp Masks.cpp Clamper.cpp Interpolation.cpp \
	LevelMaskExtractor.cpp ConnectedComponent.cpp MinRadius.cpp \
	Corrector.cpp DarkCorrector.cpp FlatCorrector.cpp \
	CalibrationFactory.cpp Focuser.cpp Focusing.cpp \
	FocusWork.cpp VCurveFocusWork.cpp FocusCompute.cpp \
	MeasureFocusWork.cpp FocusFilterfunc.cpp \
	USBContextHolder.cpp USBContext.cpp \
	USBDevice.cpp USBDeviceDescriptor.cpp \
	USBDescriptors.cpp USBEndpoint.cpp USBConfiguration.cpp \
	USBFrame.cpp USBInterface.cpp USBRawDescriptors.cpp \
	USBRequests.cpp USBTransfer.cpp USBIsoTransfer.cpp \
	Persistence.cpp Backend.cpp Testtable.cpp \
	TaskParameters.cpp TaskInfo.cpp TaskQueueEntry.cpp \
	TaskExecutor.cpp TaskQueue.cpp TaskTable.cpp \
	TrackingPersistence.cpp CalibrationPersistence.cpp \
	CalibrationStore.cpp TrackingStore.cpp \
	ImagePersistence.cpp \
	SkyWindow.cpp \
	Catalog.cpp MappedFile.cpp Ucac4.cpp Tycho2.cpp BSC.cpp Hipparcos.cpp \
	NGCIC.cpp \
	CatalogBackend.cpp FileBackend.cpp DatabaseBackend.cpp \
	ImageGeometry.cpp SkyRectangle.cpp Chart.cpp \
	$(uvc_files) \
	$(unicap_flies)

libastro_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" -DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"
