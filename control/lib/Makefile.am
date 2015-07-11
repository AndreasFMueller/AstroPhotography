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

if ENABLE_AVAHI
avahi_files = AvahiBase.cpp AvahiServiceSubset.cpp AvahiThread.cpp \
	AvahiResolver.cpp AvahiDiscovery.cpp AvahiPublisher.cpp
endif
if ENABLE_BONJOUR
bonjour_files = BonjourResolver.cpp BonjourDiscovery.cpp BonjourPublisher.cpp
endif

libastro_la_SOURCES = debug.cpp utils.cpp stacktrace.cpp Typename.cpp Trim.cpp \
	Format.cpp Timer.cpp Concatenator.cpp Exceptions.cpp \
	AttributeValuePairs.cpp UUID.cpp Path.cpp demangle.cpp ServerName.cpp \
	URL.cpp DeviceName.cpp Device.cpp Properties.cpp timeformat.cpp \
	Point.cpp Size.cpp Rectangle.cpp \
	Pixel.cpp MosaicType.cpp Coordinates.cpp Vector.cpp \
	ImagePoint.cpp ImageSize.cpp ImageRectangle.cpp ImageBase.cpp \
	ImageIteratorBase.cpp ImageLine.cpp Image.cpp ImageProperties.cpp \
	ImageDirectory.cpp ImageDatabaseDirectory.cpp \
	FITSdate.cpp Metavalue.cpp ImageMetadata.cpp \
	Sun.cpp ViewerPipeline.cpp Viewer.cpp \
	Functions.cpp Background.cpp \
	Histogram.cpp Radon.cpp \
	Subgrid.cpp Adapters.cpp DisplayConverter.cpp \
	Transform.cpp Projection.cpp Residual.cpp ProjectionCorrector.cpp \
	PhaseCorrelator.cpp Analyzer.cpp TransformAnalyzer.cpp Stacker.cpp \
	Tracker.cpp Guide.cpp Guider.cpp ThreadBase.cpp \
	GuidingProcess.cpp GuiderStateMachine.cpp \
	GuiderProcess.cpp CalibrationProcess.cpp \
	TrackingWork.cpp DrivingWork.cpp \
	GuiderCalibrator.cpp GuiderCalibration.cpp \
	GuiderFactory.cpp \
	Simulator.cpp \
	Exposure.cpp Binning.cpp ExposureTimer.cpp Loop.cpp \
	ImageProgramCallback.cpp SaveImageCallback.cpp \
	ModuleDescriptor.cpp Module.cpp DeviceLocator.cpp Repository.cpp \
	DeviceAccessor.cpp Devices.cpp Shutter.cpp \
	Camera.cpp Ccd.cpp Cooler.cpp FilterWheel.cpp GuiderPort.cpp \
	AdaptiveOptics.cpp \
	Mount.cpp \
	Serial.cpp \
	BasicGuiderport.cpp \
	Imager.cpp FITS.cpp FITSout.cpp FITSin.cpp FITSdirectory.cpp \
	FITSKeywords.cpp ImageRepoTables.cpp ImageReposTable.cpp \
	ImageSpec.cpp ImageEnvelope.cpp ImageRepo.cpp \
	RepoReplicator.cpp \
	Configuration.cpp GlobalTable.cpp ProjectTable.cpp Project.cpp \
	DeviceMapTable.cpp DeviceMapper.cpp Instrument.cpp \
	InstrumentTables.cpp ServerTable.cpp \
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
	CancellableWork.cpp ExposureWork.cpp \
	TrackingPersistence.cpp CalibrationPersistence.cpp \
	CalibrationStore.cpp TrackingStore.cpp \
	ImagePersistence.cpp \
	SkyWindow.cpp \
	Catalog.cpp MappedFile.cpp Ucac4.cpp Tycho2.cpp BSC.cpp Hipparcos.cpp \
	NGCIC.cpp \
	CatalogBackend.cpp FileBackend.cpp DatabaseBackend.cpp \
	ImageGeometry.cpp SkyRectangle.cpp Chart.cpp ImageNormalizer.cpp \
	StereographicProjection.cpp StereographicChart.cpp \
	PreviewAdapter.cpp ImageProcessing.cpp \
	ProcessingStep.cpp ProcessingController.cpp ProcessingThread.cpp \
	ImageStep.cpp RawImageFileStep.cpp WriteImageStep.cpp \
	ImageCalibrationStep.cpp CalibrationImageStep.cpp ImageBufferStep.cpp \
	CalibrationProcessorStep.cpp InterpolationStep.cpp \
	ServiceKey.cpp ServiceSubset.cpp ServiceObject.cpp \
	ServiceResolver.cpp ServiceDiscovery.cpp ServicePublisher.cpp \
	$(avahi_files) $(bonjour_files) \
	InstrumentComponent.cpp ServiceInstrument.cpp \
	$(uvc_files) \
	$(unicap_flies)

libastro_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" -DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"
