#
# Makefile.am -- build directory for the ICE stuff
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

slice_sources = camera.cpp exceptions.cpp focusing.cpp guider.cpp image.cpp \
	device.cpp tasks.cpp types.cpp repository.cpp

BUILT_SOURCES = $(slice_sources)

additional_sources = RemoteInstrument.cpp CommunicatorSingleton.cpp \
	TypesConversions.cpp DeviceConversions.cpp CcdConversions.cpp \
	FilterwheelConversions.cpp GuiderConversions.cpp TaskConversions.cpp \
	MountConversions.cpp FocusingConversions.cpp RepositoryConversions.cpp \
	CommonClientTasks.cpp ImageCallbackI.cpp \
	ImageConversions.cpp

if ENABLE_ICE

lib_LTLIBRARIES = libiceastro.la

libiceastro_la_SOURCES = \
	$(slice_sources) $(additional_sources)
libiceastro_la_CPPFLAGS = -I$(top_builddir)/ice/include @ICEINCLUDE@

endif
