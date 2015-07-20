#
# build the basic library
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SUBDIRS = utils persistence discovery image device catalogs task guiding usb . test

lib_LTLIBRARIES = libastro.la

if ENABLE_UNICAP
unicap_flies = UnicapDevice.cpp
else
unicap_flies =
endif

libastro_la_SOURCES = \
	ThreadBase.cpp \
	Simulator.cpp \
	ImageProgramCallback.cpp \
	Imager.cpp ImageRepoTables.cpp ImageReposTable.cpp \
	ImageSpec.cpp ImageEnvelope.cpp ImageRepo.cpp \
	RepoReplicator.cpp \
	Configuration.cpp GlobalTable.cpp ProjectTable.cpp Project.cpp \
	DeviceMapTable.cpp DeviceMapper.cpp Instrument.cpp \
	InstrumentTables.cpp ServerTable.cpp \
	Clamper.cpp Interpolation.cpp \
	Corrector.cpp DarkCorrector.cpp FlatCorrector.cpp \
	CalibrationFactory.cpp \
	PreviewAdapter.cpp ImageProcessing.cpp \
	ProcessingStep.cpp ProcessingController.cpp ProcessingThread.cpp \
	ImageStep.cpp RawImageFileStep.cpp WriteImageStep.cpp \
	ImageCalibrationStep.cpp CalibrationImageStep.cpp ImageBufferStep.cpp \
	CalibrationProcessorStep.cpp InterpolationStep.cpp \
	$(unicap_flies)

libastro_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" 
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

libastro_la_LIBADD =							\
	utils/libastroutils.la 						\
	persistence/libastropersistence.la 				\
	discovery/libastrodiscovery.la 					\
	image/libastroimage.la image/libastrofits.la 			\
	device/libastrodevice.la 					\
	catalogs/libastrocatalogs.la 					\
	task/libastrotask.la 						\
	guiding/libastroguiding.la 					\
	usb/libastrousb.la 
	

