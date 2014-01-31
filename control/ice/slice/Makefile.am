#
# Makefile.am -- Makefile for the ICE SLICE source code
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SLICES = camera.ice exceptions.ice guider.ice image.ice module.ice \
	tasks.ice types.ice

EXTRA_DIST = $(SLICES)

SLICEHEADERS = ../include/camera.h ../include/exceptions.h \
	../include/guider.h ../include/image.h ../include/module.h \
	../include/tasks.h ../include/types.h
SLICESOURCE = ../lib/camera.cpp ../lib/exceptions.cpp ../lib/guider.cpp \
	../lib/image.cpp ../lib/module.cpp ../lib/tasks.cpp ../lib/types.cpp

if ENABLE_ICE

all-am:	$(SLICEHEADERS) $(SLICESOURCE)

camera.h camera.cpp:

$(SLICEHEADERS) $(SLICESOURCE): $(SLICES)
	@SLICE2CPP@ -I. $(SLICES)
	for h in camera exceptions guider image module tasks types; \
	do \
		mv $${h}.h ../include; \
		mv $${h}.cpp ../lib; \
	done

endif
