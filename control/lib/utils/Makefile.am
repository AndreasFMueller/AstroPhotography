#
# Makefile.am -- subdirectory of image processing related classes
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

noinst_LTLIBRARIES = libastroutils.la

libastroutils_la_SOURCES = 						\
	AsynchronousAction.cpp						\
	AttributeValuePairs.cpp						\
	Barrier.cpp							\
	BarycentricCoordinates.cpp					\
	BlockStopWatch.cpp						\
	CallbackSet.cpp							\
	Concatenator.cpp						\
	Coordinates.cpp							\
	debug.cpp							\
	demangle.cpp							\
	Exceptions.cpp							\
	Format.cpp							\
	Path.cpp							\
	Pidfile.cpp							\
	Point.cpp							\
	Rectangle.cpp							\
	Rotation.cpp							\
	stacktrace.cpp							\
	Size.cpp							\
	Sun.cpp								\
	Timer.cpp							\
	timeformat.cpp							\
	ThreadBase.cpp							\
	Trim.cpp							\
	Typename.cpp							\
	utils.cpp							\
	UUID.cpp							\
	Vector.cpp

libastroutils_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"
