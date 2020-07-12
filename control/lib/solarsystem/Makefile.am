#
# Makefile.am -- subdirectory of solarsystem related classes
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
SUBDIRS = . test

noinst_LTLIBRARIES = libastrosolarsystem.la

libastrosolarsystem_la_SOURCES = 					\
	EclipticalCoordinates.cpp					\
	JulianCenturies.cpp						\
	Moon.cpp							\
	PerturbationTerm.cpp						\
	PerturbationSeries.cpp						\
	PerturbedPlanetoid.cpp						\
	PerturbedPlanets.cpp						\
	Planetoid.cpp							\
	Planets.cpp							\
	SinCos.cpp							\
	SolarsystemBody.cpp						\
	SolarsystemFactory.cpp						\
	Sun.cpp

libastrosolarsystem_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"

