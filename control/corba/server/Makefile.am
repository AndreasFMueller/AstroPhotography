#
# Makefile.am -- build the server
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
if ENABLE_CORBA

bin_PROGRAMS = astrod

noinst_HEADERS = Modules_impl.h \
	DriverModuleActivator_impl.h DriverModule_impl.h DeviceLocator_impl.h \
	Image_impl.h Images_impl.h ImageActivator_impl.h \
	Camera_impl.h Ccd_impl.h GuiderPort_impl.h FilterWheel_impl.h \
	Cooler_impl.h Focuser_impl.h \
	GuiderFactory_impl.h Guider_impl.h \
	PoaNameMap.h POABuilder.h \
	ImageObjectDirectory.h \
	TaskQueue_impl.h TaskActivator_impl.h Task_impl.h \
	MonitorChannel.h \
	GuiderImageCallback.h TrackingInfoCallback.h \
	CalibrationPointCallback.h \
	TaskQueueCallback.h ServerDatabase.h

noinst_LTLIBRARIES = libserver.la
libserver_la_SOURCES = Modules_impl.cpp \
	DriverModuleActivator_impl.cpp DriverModule_impl.cpp \
	Image_impl.cpp Images_impl.cpp \
	ImageActivator_impl.cpp \
	DeviceLocator_impl.cpp Camera_impl.cpp Ccd_impl.cpp \
	GuiderPort_impl.cpp FilterWheel_impl.cpp Cooler_impl.cpp \
	Focuser_impl.cpp \
	GuiderFactory_impl.cpp Guider_impl.cpp \
	Guider_images.cpp Guider_calibration.cpp Guider_tracking.cpp \
	PoaNameMap.cpp POABuilder.cpp \
	ImageObjectDirectory.cpp \
	TaskQueue_impl.cpp TaskActivator_impl.cpp Task_impl.cpp \
	GuiderImageCallback.cpp TrackingInfoCallback.cpp \
	CalibrationPointCallback.cpp \
	TaskQueueCallback.cpp ServerDatabase.cpp

astrod_SOURCES = astrod.cpp
astrod_DEPENDENCIES = $(top_builddir)/lib/libastro.la \
	$(top_builddir)/corba/lib/libcorbastro.la libserver.la
astrod_LDADD = libserver.la -L$(top_builddir)/lib -lastro \
	-L$(top_builddir)/corba/lib -lcorbastro @SBIGLIBRARY@

test:	astrod
	./astrod -d -ORBInitRef NameService=corbaname::localhost -F -b `pwd`/images 2>&1 | tee astrod.log

endif
