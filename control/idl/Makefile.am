#
# Makefile.am -- Makefile for the idl directory
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

EXTRA_DIST = exceptions.idl types.idl image.idl camera.idl module.idl guider.idl

all-am:	../lib/typesSK.cc ../include/types.hh \
	../lib/exceptionsSK.cc ../include/exceptions.hh \
	../lib/imageSK.cc ../include/image.hh \
	../lib/cameraSK.cc ../include/camera.hh \
	../lib/moduleSK.cc ../include/module.hh \
	../lib/guiderSK.cc ../include/guider.hh

../lib/exceptionsSK.cc ../include/exceptions.hh:	exceptions.idl
	omniidl -bcxx exceptions.idl
	mv exceptions.hh ../include
	mv exceptionsSK.cc ../lib

../lib/typesSK.cc ../include/types.hh:	types.idl
	omniidl -bcxx types.idl
	mv types.hh ../include
	mv typesSK.cc ../lib

../lib/imageSK.cc ../include/image.hh:	image.idl
	omniidl -bcxx image.idl
	mv image.hh ../include
	mv imageSK.cc ../lib

../lib/cameraSK.cc ../include/camera.hh:	camera.idl
	omniidl -bcxx camera.idl
	mv camera.hh ../include
	mv cameraSK.cc ../lib

../lib/moduleSK.cc ../include/module.hh:	module.idl
	omniidl -bcxx module.idl
	mv module.hh ../include
	mv moduleSK.cc ../lib

../lib/guiderSK.cc ../include/guider.hh:	guider.idl
	omniidl -bcxx guider.idl
	mv guider.hh ../include
	mv guiderSK.cc ../lib

