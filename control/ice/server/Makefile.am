#
# Makefile.am -- build ICE server for the Astro project
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

sbin_PROGRAMS = snowstar

noinst_HEADERS =							\
	AdaptiveOpticsI.h						\
	CalibrationSource.h						\
	CallbackHandler.h						\
	CameraI.h							\
	CcdI.h								\
	ConfigurationI.h						\
	CoolerI.h							\
	DaemonI.h							\
	DeviceI.h							\
	DeviceLocatorI.h						\
	DeviceLocatorLocator.h						\
	DeviceServantLocator.h						\
	DevicesI.h							\
	DriverModuleI.h							\
	DriverModuleLocator.h						\
	EventHandlerI.h							\
	EvictorBase.h							\
	FilterWheelI.h							\
	FocuserI.h							\
	FocusingFactoryI.h						\
	FocusingI.h							\
	FocusingLocator.h						\
	GatewayI.h							\
	GuidePortI.h							\
	GuiderFactoryI.h						\
	GuiderI.h							\
	GuiderI.h							\
	GuiderITemplates.h						\
	GuiderLocator.h							\
	ImageI.h							\
	ImageLocator.h							\
	ImageRepo.h							\
	ImagesI.h							\
	InstrumentI.h							\
	InstrumentLocator.h						\
	InstrumentsI.h							\
	ModulesI.h							\
	MountI.h							\
	NameConverter.h							\
	RepositoriesI.h							\
	RepositoryI.h							\
	RepositoryLocator.h						\
	RepositoryUser.h						\
	Restart.h							\
	Server.h							\
	TaskI.h								\
	TaskLocator.h							\
	TaskQueueI.h
	
snowstar_SOURCES = snowstar.cpp 					\
	AdaptiveOpticsI.cpp						\
	CalibrationSource.cpp						\
	CameraI.cpp							\
	CcdI.cpp							\
	CcdSink.cpp							\
	ConfigurationI.cpp						\
	CoolerI.cpp							\
	DaemonI.cpp							\
	DeviceI.cpp							\
	DeviceLocatorI.cpp						\
	DeviceLocatorLocator.cpp					\
	DeviceServantLocator.cpp					\
	DevicesI.cpp							\
	DriverModuleI.cpp						\
	DriverModuleLocator.cpp						\
	EventHandlerI.cpp						\
	EvictorBase.cpp							\
	FilterWheelI.cpp						\
	FocuserI.cpp							\
	FocusingFactoryI.cpp						\
	FocusingI.cpp							\
	FocusingCallback.cpp						\
	FocusingLocator.cpp						\
	GatewayI.cpp							\
	GuidePortI.cpp							\
	GuiderFactoryI.cpp						\
	GuiderIBacklash.cpp						\
	GuiderICalibrationImage.cpp					\
	GuiderICalibration.cpp						\
	GuiderIGuiding.cpp						\
	GuiderIImage.cpp						\
	GuiderI.cpp							\
	GuiderLocator.cpp						\
	ImageI.cpp							\
	ImageLocator.cpp						\
	ImageRepo.cpp							\
	ImagesI.cpp							\
	InstrumentI.cpp							\
	InstrumentLocator.cpp						\
	InstrumentsI.cpp						\
	ModulesI.cpp							\
	MountI.cpp							\
	NameConverter.cpp						\
	RepositoriesI.cpp						\
	RepositoryI.cpp							\
	RepositoryLocator.cpp						\
	RepositoryUser.cpp						\
	Restart.cpp							\
	Server.cpp							\
	TaskI.cpp							\
	TaskLocator.cpp							\
	TaskQueueI.cpp

snowstar_DEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/ice/lib/libiceastro.la
snowstar_LDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/ice/lib -liceastro @ICELIBS@
snowstar_CPPFLAGS = -I$(top_builddir)/ice/include -I$(top_builddir)/ice/slice \
	@ICEINCLUDE@ -DPIDDIR=\"@PIDDIR@\" -DMOUNT_COMMAND=\"@MOUNT@\" \
	-DUMOUNT_COMMAND=\"@UMOUNT@\"

COMMONFLAGS = --debug --foreground --base=`pwd`/images \
	--database=`pwd`/testdb.db --config=`pwd`/testdb.db \
	--name=$${servername} 
EFENCE = LD_PRELOAD=libefence.so.0.0 EF_PROTECT_FREE=1 EF_FILL=255 EF_ALLOW_MALLOC_0=1

test:	snowstar
	if [ -r servername ];						\
	then								\
		servername=`cat servername`;				\
	else								\
		servername=server;					\
	fi;								\
	./snowstar $(COMMONFLAGS) 2>&1 | tee test.log

#		--lines=100 --files=10 --logfile=test.log

testsun:	snowstar
	if [ -r servername ];						\
	then								\
		servername=`cat servername`;				\
	else								\
		servername=server;					\
	fi;								\
	STARCONTENT=SUN ./snowstar $(COMMONFLAGS) 2>&1 | tee test.log

testplanet:	snowstar
	if [ -r servername ];						\
	then								\
		servername=`cat servername`;				\
	else								\
		servername=server;					\
	fi;								\
	STARCONTENT=PLANET ./snowstar $(COMMONFLAGS) 2>&1 | tee test.log

valgrind:
	if [ -r servername ];						\
	then								\
		servername=`cat servername`;				\
	else								\
		servername=server;					\
	fi;								\
	valgrind --redzone-size=1024 --trace-children=yes 		\
		--log-file=valgrind.log 				\
		./snowstar $(COMMONFLAGS) 2>&1 | tee test.log
