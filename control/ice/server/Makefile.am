#
# Makefile.am -- build ICE server for the Astro project
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

sbin_PROGRAMS = snowstar

noinst_HEADERS = DevicesI.h \
	CoolerI.h CcdI.h CameraI.h \
	FilterWheelI.h GuiderPortI.h FocuserI.h \
	AdaptiveOpticsI.h MountI.h \
	DeviceServantLocator.h NameConverter.h \
	ImagesI.h ImageI.h ImageLocator.h \
	TaskQueueI.h TaskI.h TaskLocator.h \
	GuiderFactoryI.h GuiderI.h \
	GuiderLocator.h \
	ModulesI.h DriverModuleI.h DriverModuleLocator.h \
	DeviceLocatorI.h DeviceLocatorLocator.h \
	FocusingFactoryI.h FocusingI.h FocusingLocator.h \
	CallbackHandler.h \
	RepositoriesI.h RepositoryI.h RepositoryLocator.h \
	InstrumentI.h InstrumentLocator.h InstrumentsI.h \
	DeviceI.h \
	CalibrationSource.h \
	EventHandlerI.h ConfigurationI.h \
	RepositoryUser.h \
	Server.h
	
snowstar_SOURCES = snowstar.cpp \
	DevicesI.cpp \
	CoolerI.cpp CcdI.cpp CcdSink.cpp CameraI.cpp \
	FilterWheelI.cpp GuiderPortI.cpp FocuserI.cpp \
	AdaptiveOpticsI.cpp MountI.cpp \
	DeviceServantLocator.cpp NameConverter.cpp \
	ImagesI.cpp ImageI.cpp ImageLocator.cpp \
	TaskQueueI.cpp TaskI.cpp TaskLocator.cpp \
	GuiderFactoryI.cpp GuiderI.cpp \
	GuiderLocator.cpp \
	ModulesI.cpp DriverModuleI.cpp DriverModuleLocator.cpp \
	DeviceLocatorI.cpp DeviceLocatorLocator.cpp \
	FocusingFactoryI.cpp FocusingI.cpp FocusingLocator.cpp \
	RepositoriesI.cpp RepositoryI.cpp RepositoryLocator.cpp \
	InstrumentI.cpp InstrumentLocator.cpp InstrumentsI.cpp \
	DeviceI.cpp \
	CalibrationSource.cpp \
	EventHandlerI.cpp ConfigurationI.cpp \
	RepositoryUser.cpp \
	Server.cpp

snowstar_DEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/ice/lib/libiceastro.la
snowstar_LDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/ice/lib -liceastro @ICELIBS@
snowstar_CPPFLAGS = -I$(top_builddir)/ice/include -I$(top_builddir)/ice/slice \
	@ICEINCLUDE@ -DPIDDIR=\"@PIDDIR@\"

test:	snowstar
	if [ -r servername ]; \
	then \
		servername=`cat servername`; \
	else \
		servername=server; \
	fi; \
	./snowstar --debug --base=`pwd`/images --database=`pwd`/testdb.db \
		--config=`pwd`/testdb.db --name=$${servername} --foreground \
		2>&1 | tee test.log

testsun:	snowstar
	if [ -r servername ]; \
	then \
		servername=`cat servername`; \
	else \
		servername=server; \
	fi; \
	STARCONTENT=SUN ./snowstar --debug --base=`pwd`/images \
		--database=`pwd`/testdb.db --config=testdb.db \
		--name=$${servername} --foreground 2>&1 | tee test.log

testplanet:	snowstar
	if [ -r servername ]; \
	then \
		servername=`cat servername`; \
	else \
		servername=server; \
	fi; \
	STARCONTENT=PLANET ./snowstar --debug --base=`pwd`/images \
		--database=`pwd`/testdb.db --config=testdb.db \
		--name=$${servername} --foreground 2>&1 | tee test.log

valgrind:
	if [ -r servername ]; \
	then \
		servername=`cat servername`; \
	else \
		servername=server; \
	fi; \
	valgrind --redzone-size=1024 --trace-children=yes --log-file=valgrind.log \
		./snowstar --debug --base=`pwd`/images \
		--database=`pwd`/testdb.db \
		--config=`pwd`/testdb.db --name=$${servername} --foreground \
		2>&1 | tee test.log
