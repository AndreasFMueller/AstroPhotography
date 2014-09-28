#
# Makefile.am -- Makefile for the ICE SLICE source code
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SLICES = camera.ice exceptions.ice focusing.ice guider.ice image.ice \
	device.ice tasks.ice types.ice

EXTRA_DIST = $(SLICES)

SLICEHEADERS = ../include/camera.h ../include/exceptions.h \
	../include/focusing.h ../include/guider.h ../include/image.h \
	../include/device.h ../include/tasks.h ../include/types.h
SLICESOURCE = ../lib/camera.cpp ../lib/exceptions.cpp ../lib/focusing.cpp \
	../lib/guider.cpp ../lib/image.cpp ../lib/device.cpp ../lib/tasks.cpp \
	../lib/types.cpp 

if ENABLE_ICE

all-am:	$(SLICEHEADERS) $(SLICESOURCE)

camera.h camera.cpp:

$(SLICEHEADERS) $(SLICESOURCE): $(SLICES)
	@SLICE2CPP@ -I. $(SLICES)
	for h in camera exceptions focusing guider image device tasks types; \
	do \
		mv $${h}.h ../include; \
		mv $${h}.cpp ../lib; \
	done

endif
