#
# Makefile.am -- Makefile for the ICE SLICE source code
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
SLICES = camera.ice exceptions.ice focusing.ice guider.ice image.ice \
	device.ice tasks.ice types.ice repository.ice instruments.ice

EXTRA_DIST = $(SLICES)

SLICEHEADERS = camera.h exceptions.h focusing.h guider.h image.h device.h \
	tasks.h types.h repository.h instruments.h
SLICESOURCE = camera.cpp exceptions.cpp focusing.cpp guider.cpp image.cpp \
	device.cpp tasks.cpp types.cpp  repository.cpp instruments.cpp

if ENABLE_ICE

all-am:	copytimestamp

camera.h camera.cpp:	camera.ice
	@SLICE2CPP@ -I. -I@SLICEDIR@ camera.ice

exceptions.h exceptions.cpp:	exceptions.ice
	@SLICE2CPP@ -I. -I@SLICEDIR@ exceptions.ice

focusing.h focusing.cpp:	focusing.ice
	@SLICE2CPP@ -I. -I@SLICEDIR@ focusing.ice

guider.h guider.cpp:	guider.ice
	@SLICE2CPP@ -I. -I@SLICEDIR@ guider.ice

image.h image.cpp:	image.ice
	@SLICE2CPP@ -I. -I@SLICEDIR@ image.ice

device.h device.cpp:	device.ice
	@SLICE2CPP@ -I. -I@SLICEDIR@ device.ice

tasks.h tasks.cpp:	tasks.ice
	@SLICE2CPP@ -I. -I@SLICEDIR@ tasks.ice

types.h types.cpp:	types.ice
	@SLICE2CPP@ -I. -I@SLICEDIR@ types.ice

repository.h repository.cpp:	repository.ice
	@SLICE2CPP@ -I. -I@SLICEDIR@ repository.ice

instruments.h instruments.cpp:	instruments.ice
	@SLICE2CPP@ -I. -I@SLICEDIR@ instruments.ice


#
# We should automatically add the slice directory of the Ice installation
# to the slic2cpp command
#
copytimestamp:	$(SLICEHEADERS) $(SLICESOURCE)
	for h in camera exceptions focusing guider image device tasks types \
		repository instruments; \
	do \
		cp $${h}.h ../include; \
		cp $${h}.cpp ../lib; \
	done
	touch copytimestamp	

endif
