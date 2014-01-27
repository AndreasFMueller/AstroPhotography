#
# Makefile.am -- Makefile for the include files
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
CORBAHEADERS = types.hh exceptions.hh image.hh camera.hh module.hh \
	guider.hh tasks.hh

include_HEADERS = AstroDebug.h AstroFormat.h AstroImage.h AstroCamera.h \
	AstroLoader.h AstroDisplay.h AstroImager.h \
	AstroIO.h AstroPixel.h AstroUSB.h AstroUVC.h AstroUnicap.h \
	AstroUtils.h AstroMosaic.h AstroDemosaic.h AstroAdapter.h \
	AstroFilter.h AstroFilterfunc.h AstroCalibration.h \
	AstroOperations.h AstroTransform.h \
	AstroMask.h AstroGuiding.h AstroTypes.h AstroCallback.h \
	AstroCoordinates.h AstroDevice.h AstroLocator.h AstroExceptions.h \
	AstroFocus.h AstroViewer.h AstroTonemapping.h \
	AstroPersistence.h AstroTask.h AstroInterpolation.h \
	$(CORBAHEADERS)

noinst_HEADERS = PixelValue.h Simulator.h Sun.h \
	GuidingProcess.h CalibrationProcess.h GuiderProcess.h \
	NameService.h OrbSingleton.h Conversions.h ViewerPipeline.h \
	CorbaExceptionReporter.h DeviceLocatorAdapter.h Testtable.h \
	ImageDirectory.h TaskTable.h TrackingWork.h DrivingWork.h \
	TrackingPersistence.h CalibrationPersistence.h \
	TrackingStore.h CalibrationStore.h ImagePersistence.h

BUILT_SOURCES = $(CORBAHEADERS)
