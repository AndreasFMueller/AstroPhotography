#
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_REVISION([1.0])
AC_INIT([snowgui], [1.0], [andreas.mueller@hsr.ch])
AC_CONFIG_SRCDIR([snowgui/common.h])
AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE()
AC_CONFIG_HEADERS([common/config.h])

# Checks for programs
AC_PROG_CXX
AC_PROG_CC
AC_OPENMP

# Checks for header files
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h stdio.h unistd.h math.h errno.h string.h stdarg.h libintl.h locale.h syslog.h sys/types.h sys/stat.h sys/time.h dirent.h fcntl.h sys/file.h dlfcn.h libusb-1.0/libusb.h fftw3.h Accelerate/Accelerate.h termios.h sys/mman.h execinfo.h sys/select.h poll.h signal.h sys/param.h getopt.h uuid/uuid.h netdb.h assert.h])

# We want to use backtrace API for stack dumps, this requires the use of
# the -rdynamic option during compile
LDFLAGS="${LDFLAGS} -rdynamic"

# Check for libraries that we cannot do without
AC_CHECK_LIB([m], [sqrt])
AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([usb-1.0], [libusb_bulk_transfer])
AC_CHECK_LIB([lapack], [dgels_], [LIBS="-llapack $LIBS"],
	AC_MSG_ERROR([required library LAPACK not found]))
AC_CHECK_LIB([fftw3], [fftw_malloc])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([glpk], [glp_create_prob])

## Multiarch support in debian #######################################
if test -r /etc/debian_version
then
	multiarch=`dpkg-architecture -qDEB_HOST_MULTIARCH`
fi

## LIBUSB ############################################################
if pkg-config --exists libusb-1.0
then
	LIBS="${LIBS} `pkg-config --libs libusb-1.0`"
	CFLAGS="${CFLAGS} `pkg-config --cflags libusb-1.0`"
	CXXFLAGS="${CXXFLAGS} `pkg-config --cflags libusb-1.0`"
else
	AC_CHECK_HEADERS([libusb-1.0/libusb.h])
	AC_CHECK_LIB([usb-1.0], [libusb_bulk_transfer])
fi

## CFITS #############################################################
# if cfits is installed with pkginfo, we use that
if pkg-config --exists cfitsio
then
	LIBS="${LIBS} `pkg-config --libs cfitsio`"
	CXXFLAGS="${CXXFLAGS} `pkg-config --cflags cfitsio`"
	CFLAGS="${CFLAGS} `pkg-config --cflags cfitsio`"
else
AC_CHECK_LIB([cfitsio], [ftffrw_])
fi

## LIBJPEG ###########################################################
# if libjpeg is installed with pkginfo, we use that
if pkg-config --exists libjpeg
then
	LIBS="${LIBS} `pkg-config --libs libjpeg`"
	CXXFLAGS="${CXXFLAGS} `pkg-config --cflags libjpeg`"
else
AC_CHECK_LIB([jpeg], [jpeg_abort])
fi

## LIBPNG ############################################################
# if libjpeg is installed with pkginfo, we use that
if pkg-config --exists libpng
then
	LIBS="${LIBS} `pkg-config --libs libpng`"
	CXXFLAGS="${CXXFLAGS} `pkg-config --cflags libpng`"
else
AC_CHECK_LIB([png], [png_free])
fi

## LIBSQLITE3 ########################################################
# if libjpeg is installed with pkginfo, we use that
if pkg-config --exists sqlite3
then
	LIBS="${LIBS} `pkg-config --libs sqlite3`"
	CXXFLAGS="${CXXFLAGS} `pkg-config --cflags sqlite3`"
else
AC_CHECK_LIB([sqlite3], [sqlite3_initialize])
fi

## UUID ##############################################################
# if uuid is configured with pkg-config
if pkg-config --exists uuid
then
	LIBS="${LIBS} `pkg-config --libs uuid`"
	CXXFLAGS="${CXXFLAGS} `pkg-config --cflags uuid`"
else
AC_CHECK_LIB([uuid], [uuid_parse])
fi

## DNS-DS ############################################################
servicediscovery=
case `uname` in
Linux)	# find out whether we have AVAHI support
	if pkg-config --exists avahi-client
	then
		LIBS="${LIBS} `pkg-config --libs avahi-client`"
		CXXFLAGS="${CXXFLAGS} `pkg-config --cflags avahi-client`"
	fi
	;;
Darwin)	;;
esac

## ICE ###############################################################
# detect ICE support (www.zeroc.com)
#
# unfortunately, ICE does not have a well developed installation concept
# the default installation goes into /opt/Ice-3.5.1, but on Mac OS X it
# is /Library/Developer/Ice-3.5.1. Furthermore, the binaries on Mac OS X
# are in a subdirectory ICEDIR/bin/c++ instead of ICEDIR/bin.
#

# we start with empty variables, and try to fill them step by step
ICEDIR=
ICELIBS=
ICEINCLUDE=
AC_ARG_WITH([ice],
[AS_HELP_STRING([--with-ice=dir], [build the ICE components from path])],
[
	# given the installation directory, we can derive the path
	ICEDIR=${with_ice}
	PATH=${PATH}:${ICEDIR}/bin:${ICEDIR}/c++/bin

	# get the library directories
	if test -d ${ICEDIR}/lib64
	then
		if test -d ${ICEDIR}/lib64/c++11
		then
			ICELIBS="-L${ICEDIR}/lib64/c++11 -Wl,-rpath,${ICEDIR}/lib64/c++11"
		else
			ICELIBS="-L${ICEDIR}/lib64 -Wl,-rpath,${ICEDIR}/lib64"
		fi
	elif test -d ${ICEDIR}/lib/${multiarch}
	then
		if test -d ${ICEDIR}/lib/${multiarch}/c++11
		then
			ICELIBS="-L${ICEDIR}/lib/${multiarch}/c++11 -Wl,-rpath,${ICEDIR}/lib/${multiarch}/c++11"
		else
			ICELIBS="-L${ICEDIR}/lib/${multiarch} -Wl,-rpath,${ICEDIR}/lib/${multiarch}"
		fi
	elif test -d ${ICEDIR}/lib
	then
		if test -d ${ICEDIR}/lib/c++11
		then
			ICELIBS="-L${ICEDIR}/lib/c++11 -Wl,-rpath,${ICEDIR}/lib/c++11"
		else
			ICELIBS="-L${ICEDIR}/lib -Wl,-rpath,${ICEDIR}/lib"
		fi
	fi

	# get the include directory, if we cannot find it below the ICE
	# directory, we will assume that it is already on the include path
	if test -d ${ICEDIR}/include
	then
		ICEINCLUDE=-I${ICEDIR}/include
	fi
],[
])

OLDLIBS="${LIBS}"
OLDCXXFLAGS="${CXXFLAGS} ${ICEINCLUDE}"
OLDCFLAGS="${CFLAGS} ${ICEINCLUDE}"

AC_CHECK_LIB([IceUtil], [childAtFork],[
	ICELIBS="${ICELIBS} -lIce -lIceUtil"
],[
	ICELIBS="${ICELIBS} -lIce"
])

AC_SUBST(ICEDIR)
AC_SUBST(ICELIBS)
AC_SUBST(ICEINCLUDE)

LIBS="${OLDLIBS}"
CXXFLAGS="${OLDCXXFLAGS}"
CFLAGS="${OLDCFLAGS}"

if test -d /usr/local/astro
then
	ASTROPATH=/usr/local/astro
else
	ASTROPATH=`pwd`/../control/root
fi
AC_SUBST(ASTROPATH)
if test -d ${ASTROPATH}/lib64
then
	ASTROLIBDIR=${ASTROPATH}/lib64
else 
	ASTROLIBDIR=${ASTROPATH}/lib
fi
AC_SUBST(ASTROLIBDIR)

EXTRA_CXXFLAGS="${EXTRA_CXXFLAGS} ${CXXFLAGS} ${OPENMP_CFLAGS}"
EXTRA_LDFLAGS="${EXTRA_LDFLAGS} ${LDFLAGS} ${OPENMP_CFLAGS}"
EXTRA_LIBS="${EXTRA_LIBS} ${LIBS}"

AC_SUBST(EXTRA_CXXFLAGS)
AC_SUBST(EXTRA_LDFLAGS)
AC_SUBST(EXTRA_LIBS)

MACSDK=macosx10.15
AC_SUBST(MACSDK)

AC_CONFIG_FILES([Makefile snowgui/snowgui.pro
	snowgui/astrogui/astrogui.pro
	snowgui/icegui/icegui.pro
	snowgui/image/image.pro
	snowgui/preview/preview.pro
	snowgui/test/test.pro
	snowgui/focusing/focusing.pro
	snowgui/guiding/guiding.pro
	snowgui/pointing/pointing.pro
	snowgui/instruments/instruments.pro
	snowgui/images/images.pro
	snowgui/repository/repository.pro
	snowgui/expose/expose.pro
	snowgui/task/task.pro
	snowgui/browser/browser.pro
	snowgui/main/main.pro
	snowgui/astroviewer/astroviewer.pro
	snowgui/astrosky/astrosky.pro
	snowgui/astrobrowser/astrobrowser.pro
	snowgui/snowlife/snowlife.pro
	snowgui/build])
AC_OUTPUT

