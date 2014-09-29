#
# Makefile.am -- build ICE server for the Astro project
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_ICE

bin_PROGRAMS = snowstar

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
	FocusingFactoryI.h FocusingI.h
	
snowstar_SOURCES = snowstar.cpp \
	DevicesI.cpp \
	CoolerI.cpp CcdI.cpp CameraI.cpp \
	FilterWheelI.cpp GuiderPortI.cpp FocuserI.cpp \
	AdaptiveOpticsI.cpp MountI.cpp \
	DeviceServantLocator.cpp NameConverter.cpp \
	ImagesI.cpp ImageI.cpp ImageLocator.cpp \
	TaskQueueI.cpp TaskI.cpp TaskLocator.cpp \
	GuiderFactoryI.cpp GuiderI.cpp \
	GuiderLocator.cpp \
	ModulesI.cpp DriverModuleI.cpp DriverModuleLocator.cpp \
	DeviceLocatorI.cpp DeviceLocatorLocator.cpp \
	FocusingFactoryI.cpp FocusingI.cpp

snowstar_DEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/ice/lib/libiceastro.la
snowstar_LDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/ice/lib -liceastro
snowstar_LDFLAGS = -L@ICEDIR@/lib/c++11 -Wl,-rpath,@ICEDIR@/lib/c++11 -lIce -lIceUtil
snowstar_CPPFLAGS = -I$(top_builddir)/ice/include -I@ICEDIR@/include

test:	snowstar
	./snowstar -d -b images -q testdb.db

endif
