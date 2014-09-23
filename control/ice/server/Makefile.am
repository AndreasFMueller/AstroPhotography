#
# Makefile.am -- build ICE server for the Astro project
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_ICE

bin_PROGRAMS = snowstar

noinst_HEADERS = DevicesI.h \
	CoolerI.h CcdI.h CcdIconversions.h CameraI.h \
	FilterWheelI.h GuiderPortI.h FocuserI.h \
	DeviceServantLocator.h NameConverter.h \
	ImagesI.h ImageI.h ImageLocator.h \
	TypesI.h \
	TaskQueueI.h TaskI.h TaskLocator.h \
	GuiderIconversions.h GuiderFactoryI.h GuiderI.h \
	GuiderLocator.h \
	ModulesI.h DriverModuleI.h DriverModuleLocator.h \
	DeviceLocatorI.h DeviceLocatorLocator.h
	
snowstar_SOURCES = snowstar.cpp \
	DevicesI.cpp DevicesIconversions.cpp \
	CoolerI.cpp CcdI.cpp CcdIconversions.cpp CameraI.cpp \
	FilterWheelI.cpp GuiderPortI.cpp FocuserI.cpp \
	DeviceServantLocator.cpp NameConverter.cpp \
	ImagesI.cpp ImageI.cpp ImageLocator.cpp \
	TypesI.cpp \
	TaskQueueI.cpp TaskQueueIconversions.cpp TaskI.cpp TaskLocator.cpp \
	GuiderIconversions.cpp GuiderFactoryI.cpp GuiderI.cpp \
	GuiderLocator.cpp \
	ModulesI.cpp DriverModuleI.cpp DriverModuleLocator.cpp \
	DeviceLocatorI.cpp DeviceLocatorLocator.cpp

snowstar_DEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/ice/lib/libiceastro.la
snowstar_LDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/ice/lib -liceastro
snowstar_LDFLAGS = -L@ICEDIR@/lib/c++11 -Wl,-rpath,@ICEDIR@/lib/c++11 -lIce -lIceUtil
snowstar_CPPFLAGS = -I$(top_builddir)/ice/include -I@ICEDIR@/include

test:	snowstar
	./snowstar -d -b images -q testdb.db

endif
