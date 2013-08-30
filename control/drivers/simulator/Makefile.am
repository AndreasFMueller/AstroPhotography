#
# Makefile.am -- build the simulator camera
#
# (c) 2013 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
pkglib_LTLIBRARIES = simulator.la
noinst_HEADERS = SimUtil.cpp SimLocator.h SimCamera.h SimCcd.h \
	SimGuiderPort.h SimFilterWheel.h SimCooler.h \
	SimFocuser.h Stars.h Blurr.h

SRCFILES = SimUtil.cpp SimLocator.cpp SimCamera.cpp SimCcd.cpp \
	SimGuiderPort.cpp SimFilterWheel.cpp SimCooler.cpp \
	SimFocuser.cpp \
	Stars.cpp Starfield.cpp StarCamera.cpp Blurr.cpp

if ENABLE_UNITTESTS

noinst_LTLIBRARIES = libsimulator.la

libsimulator_la_SOURCES = $(SRCFILES)
libsimulator_la_DEPENDENCIES = ../../lib/libastro.la
libsimulator_la_LIBADD = ../../lib/libastro.la

noinst_PROGRAMS = simulatortests
simulatortests_SOURCES = simulatortests.cpp \
	SimLocatorTest.cpp SimUtilTest.cpp SimCoolerTest.cpp \
	SimCameraTest.cpp SimCcdTest.cpp SimGuiderPortTest.cpp \
	StarsTest.cpp
simulatortests_LDADD = -lcppunit ../../lib/libastro.la -L. -lsimulator
simulatortests_DEPENDENCIES = ../../lib/libastro.la libsimulator.la

test:	simulatortests
	./simulatortests -d

endif

simulator_la_SOURCES = $(SRCFILES)
simulator_la_LDFLAGS = -module
simulator_la_DEPENDENCIES = ../../lib/libastro.la
simulator_la_LIBADD = ../../lib/libastro.la

