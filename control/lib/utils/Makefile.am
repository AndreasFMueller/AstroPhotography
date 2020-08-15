#
# Makefile.am -- subdirectory of image processing related classes
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

noinst_LTLIBRARIES = libastroutils.la

libastroutils_la_SOURCES = 						\
	Angle.cpp							\
	AngularSize.cpp							\
	AsynchronousAction.cpp						\
	AttributeValuePairs.cpp						\
	AzmAltConverter.cpp						\
	Barrier.cpp							\
	BarycentricCoordinates.cpp					\
	BlockStopWatch.cpp						\
	CallbackSet.cpp							\
	Concatenator.cpp						\
	debug.cpp							\
	demangle.cpp							\
	Ecliptic.cpp							\
	Exceptions.cpp							\
	Format.cpp							\
	Gateway.cpp							\
	GridCalculator.cpp						\
	Horizon.cpp							\
	ImageCoordinates.cpp						\
	JulianDate.cpp							\
	LongLat.cpp							\
	Memory.cpp							\
	Path.cpp							\
	Pidfile.cpp							\
	Point.cpp							\
	Precession.cpp							\
	RaDec.cpp							\
	Rectangle.cpp							\
	Rotation.cpp							\
	Rotation3D.cpp							\
	stacktrace.cpp							\
	Size.cpp							\
	SphericalCoordinates.cpp					\
	Sun.cpp								\
	TaskUpdate.cpp							\
	Temperature.cpp							\
	Time.cpp							\
	Timer.cpp							\
	timeformat.cpp							\
	ThreadBase.cpp							\
	Trim.cpp							\
	TwoAngles.cpp							\
	Typename.cpp							\
	utils.cpp							\
	UUID.cpp							\
	UnitVector.cpp							\
	Vector.cpp							\
	Version.cpp

libastroutils_la_CPPFLAGS = -DPKGLIBDIR=\"$(pkglibdir)\" \
	-DDEVICEPROPERTIES=\"@DEVICEPROPERTIES@\"
