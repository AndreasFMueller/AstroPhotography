#
# Makefile.am -- build the uvc driver, based on the Uvc code in this project
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
if ENABLE_UVC

noinst_HEADERS =  UvcLocator.h UvcUtils.h UvcCamera.h UvcCcd.h
pkglib_LTLIBRARIES = uvc.la 

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libuvc.la

libuvc_la_SOURCES = UvcLocator.cpp UvcUtils.cpp UvcCamera.cpp UvcCcd.cpp
libuvc_la_DEPENDENCIES = ../../lib/libastro.la
libuvc_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = uvctests
uvctests_SOURCES = uvctests.cpp uvctest.cpp
uvctests_LDADD = -lcppunit ../../lib/libastro.la -L. -luvc 
uvctests_DEPENDENCIES = ../../lib/libastro.la libuvc.la

endif

uvc_la_SOURCES = UvcLocator.cpp UvcUtils.cpp UvcCamera.cpp UvcCcd.cpp
uvc_la_LDFLAGS = -module
uvc_la_DEPENDENCIES = ../../lib/libastro.la
uvc_la_LIBADD = ../../lib/libastro.la

endif
