#
# configure script to create the pro files
#
# (c) 2014 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
AC_PREREQ(2.57)
AC_REVISION([1.0])
AC_INIT([astrogui], [1.0])
AC_CONFIG_SRCDIR([ExposureWorker.h])
AC_CANONICAL_TARGET([])

AC_PROG_CXX
AC_PROG_CC
AC_PROG_INSTALL

######################################################################
# find the omniorb parameters using pkgconfig
for pkg in omniORB4 omniCOS4
do
	CPPFLAGS="${CPPFLAGS} `pkg-config --cflags-only-I ${pkg}`"
	CXXFLAGS="${CXXFLAGS} `pkg-config --cflags-only-other ${pkg}`"
	CFLAGS="${CFLAGS} `pkg-config --cflags-only-other ${pkg}`"
	LIBS="${LIBS} `pkg-config --libs ${pkg}`"
done

AC_CONFIG_FILES(astrogui.pro tasks/Tasks/Tasks.pro guiding/Guiding/Guiding.pro)
AC_OUTPUT
