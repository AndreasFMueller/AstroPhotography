#
# Makefile.am -- build ICE server for the Astro project
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

if ENABLE_ICE

bin_PROGRAMS = snowstar

noinst_HEADERS = ModulesI.h DriverModuleI.h DeviceLocatorI.h CoolerI.h \
	CcdI.h CameraI.h FilterWheelI.h GuiderPortI.h FocuserI.h

snowstar_SOURCES = snowstar.cpp \
	ModulesI.cpp DriverModuleI.cpp DeviceLocatorI.cpp CoolerI.cpp \
	CcdI.cpp CameraI.cpp  FilterWheelI.cpp GuiderPortI.cpp FocuserI.cpp
snowstar_DEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/ice/lib/libiceastro.la
snowstar_LDADD = -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/ice/lib -liceastro
snowstar_LDFLAGS = -L@ICEDIR@/lib/c++11 -Wl,-rpath,@ICEDIR@/lib/c++11 -lIce -lIceUtil
snowstar_CPPFLAGS = -I$(top_builddir)/ice/include -I@ICEDIR@/include

test:	snowstar
	./snowstar -d

endif
