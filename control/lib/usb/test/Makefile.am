#
# Makefile -- build the test subdirectory
# 
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
noinst_HEADERS = 
usb_ldadd = -lcppunit -L$(top_builddir)/lib/usb -lastrousb \
	-L$(top_builddir)/lib/utils -lastroutils
usb_dependencies = ../libastrousb.la $(top_builddir)/lib/utils/libastroutils.la

if ENABLE_UNITTESTS

# files needed for the UVC driver tests
if ENABLE_UVC
uvc_tests = uvctests.cpp UVCDescriptorTest.cpp UVCCameraTest.cpp
uvc_cmds = uvctests
else
uvc_tests =
uvc_cmds =
endif

# stuff related to testing
noinst_PROGRAMS = singletest usbtests $(uvc_cmds) $(unicap_cmds)

# single test
singletest_SOURCES = singletest.cpp
singletest_LDADD = $(usb_ldadd)
singletest_DEPENDENCIES = $(usb_dependencies)

single:	singletest
	./singletest -d 2>&1 | tee single.log

# tests for USB library
usbtests_SOURCES = usbtests.cpp 					\
	USBContextTest.cpp 						\
	USBDescriptorTest.cpp
usbtests_LDADD = $(usb_ldadd)
usbtests_DEPENDENCIES = $(usb_dependencies)

usbteset:	usbtests
	./usbtests -d 

# tests for UVC cameras
if ENABLE_UVC
uvctests_SOURCES = $(uvc_tests)
uvctests_LDADD = $(usb_ldadd)
uvctests_DEPENDENCIES = $(usb_dependencies)

uvctest:	uvctests
	./uvctests -d
endif

endif
