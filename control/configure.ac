#
# Process this file with autoconf to produce a configure script.

AC_PREREQ(2.57)
AC_REVISION([1.0])
AC_INIT
LT_INIT
AC_CONFIG_SRCDIR([include/AstroImage.h])
AC_CANONICAL_TARGET([])
AM_INIT_AUTOMAKE(astro, 1.0)
AC_CONFIG_HEADERS(include/config.h)

# some configuration directives

# Checks for programs
AC_PROG_CXX
AC_PROG_CC
AC_PROG_LIBTOOL
AC_CHECK_TOOL(FLEX, flex)
AC_CHECK_TOOL(BISON, bison)

# Checks for header files
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h stdio.h unistd.h math.h errno.h string.h stdarg.h libintl.h locale.h syslog.h sys/types.h sys/stat.h sys/time.h dirent.h fcntl.h sys/file.h dlfcn.h libusb-1.0/libusb.h fftw3.h Accelerate/Accelerate.h termios.h])

# Check for libraries
AC_CHECK_LIB([m], [sqrt])
AC_CHECK_LIB([cfitsio], [ftffrw_])
AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([usb-1.0], [libusb_bulk_transfer])
AC_CHECK_LIB([lapack], [dgels_], [LIBS="-llapack $LIBS"],
	AC_MSG_ERROR([required library LAPACK not found]))
AC_CHECK_LIB([fftw3], [fftw_malloc])
AC_CHECK_LIB([pthread], [pthread_create])
AC_CHECK_LIB([glpk], [glp_create_prob])
AC_CHECK_LIB([sqlite3], [sqlite3_initialize])

# use pkg-config to find out about the flags to use for omniorb
if type pkg-config >/dev/null
then
	ORBLIBS="`pkg-config omniORB4 --libs`"
else
	ORBLIBS="-lomniCodeSets4 -lomniConnectionMgmt4 -lomniDynamic4 -lomniORB4 -lomnithread"
fi
LIBS="${ORBLIBS} ${LIBS}"

######################################################################
# decide whether or not to build the test programs
AC_ARG_ENABLE(unittests,
[AS_HELP_STRING([--enable-unittests], [also build the unit tests if CPPUNIT is present])],
[
],[
])
AM_CONDITIONAL(ENABLE_UNITTESTS, test x$enable_unittests = xyes)

######################################################################
# decide whether to build the UVC driver, by default we only build
# it on Mac OS X
AC_ARG_ENABLE(uvc,
[AS_HELP_STRING([--enable-uvc], [build the UVC driver])],
[
],[
	case `uname` in
	Darwin)	enable_uvc=yes
		;;
	*)	enable_uvc=no
		;;
	esac
])
AM_CONDITIONAL(ENABLE_UVC, test x$enable_uvc = xyes)

######################################################################
# check for SBIG library
AC_ARG_ENABLE(sbig,
[AS_HELP_STRING([--enable-sbig], [build the proprietary SBIG driver])],
[
	case `uname` in
	Darwin)	SBIGTARGET=ENV_MACOSX
		SBIGINCLUDE="-framework SBIGUDrv"
		;;
	Linux)	SBIGTARGET=ENV_LINUX
		SBIGINCLUDE=-I/usr/local/include
		;;
	esac
	AC_SUBST(SBIGTARGET)
	OLDCFLAGS="${CFLAGS}"
	CFLAGS="${CFLAGS} -DTARGET=${SBIGTARGET} ${SBIGINCLUDE}"
	case `uname` in
	Linux)	OLDLIBS="${LIBS}"
		AC_CHECK_HEADERS(lpardrv.h, ,
			[AC_MSG_ERROR([SBIG header not found])])
		AC_CHECK_LIB([sbigudrv], [SBIGUnivDrvCommand], enable_sbig=yes,
			[AC_MSG_ERROR([SBIG library not found])])
		SBIGLIBRARY=-lsbigudrv
		LIBS="${OLDLIBS}"
		;;
	Darwin)	
		AC_CHECK_HEADERS(SBIGUDrv/lpardrv.h, , [AC_MSG_ERROR([SBIG header not found])])
		echo "Mac OS X: header was found, so library will work"
		SBIGLIBRARY="-framework SBIGUDrv"
		CFLAGS="${CPPFLAGS} ${SBIGLIBRARY}"
		;;
	esac
	AC_SUBST(SBIGLIBRARY)
	AC_SUBST(SBIGINCLUDE)
	CFLAGS="${OLDCFLAGS}"
],[
enable_sbig=no
]
)
AM_CONDITIONAL(ENABLE_SBIG, test x$enable_sbig = xyes)

######################################################################
# check for qsi library
AC_ARG_ENABLE(qsi,
[AS_HELP_STRING([--enable-qsi], [build the QSI driver])],
[
	# don't enable the QSI driver unless you find the header
	# and the library
	enable_qsi=no

	# check whether the qsiapi.h header is present
	AC_LANG_PUSH([C++])
	AC_CHECK_HEADERS([qsiapi.h])
	AC_CHECK_HEADER([qsiapi.h],
                     [AC_DEFINE([HAVE_QSIAPI_H], [1],
                        [Define to 1 if you have <qsiapi.h>.])],
                     [AC_MSG_ERROR([QSI header not available, install or disable QSI driver])])
	AC_LANG_POP([C++])

	# check whether the qsiapi library is present
	AC_CHECK_LIB([qsiapi], [_ZN10CCCDCameraC1Ev])

	# if we fail, disable qsi
	enable_qsi=yes
],[
enable_qsi=no
]
)
AM_CONDITIONAL(ENABLE_QSI, test x$enable_qsi = xyes)

######################################################################
# enable the qhy driver
AC_ARG_ENABLE(qhy,
[AS_HELP_STRING([--enable-qhy], [build the QHY driver])],
[
	enable_qhy=yes
],[
	enable_qhy=no
])
AM_CONDITIONAL(ENABLE_QHY, test x$enable_qhy = xyes)

######################################################################
# check for the Unicap library
AC_ARG_ENABLE(unicap,
[AS_HELP_STRING([--enable-unicap], [build the unicap driver])],
[
	if test x${enable_unicap} = xyes
	then
		AC_CHECK_HEADERS(unicap.h, ,
			[AC_MSG_ERROR([unicap header not found])])
		AC_CHECK_LIB([unicap], [unicap_open], LIBS="${LIBS} -lunicap",
			[AC_MSG_ERROR([unicap library not found])])
	fi
],[
]
)
AM_CONDITIONAL(ENABLE_UNICAP, test x${enable_unicap} = xyes)

######################################################################
# turn on the microtouch stuff
AC_ARG_ENABLE(microtouch,
[AS_HELP_STRING([--enable-microtouch], [build the microtouch driver])], ,
	enable_microtouch=yes)
AM_CONDITIONAL(ENABLE_MICROTOUCH, test x${enable_microtouch} = xyes)

######################################################################

# Checks for typedefs
AC_C_CONST

# Checks for library functions
AC_CHECK_FUNCS([memset strdup strerror trunc])

# definitions for the extension for shared libraries
case `uname` in
	Darwin*)
		SHLIB_EXT=dylib
		;;
	*)
		SHLIB_EXT=so
		;;
esac
AC_SUBST(SHLIB_EXT)

AC_CONFIG_FILES([Makefile include/Makefile lib/Makefile lib/test/Makefile
	idl/Makefile server/Makefile client/Makefile tools/Makefile
	guider/Makefile
	drivers/Makefile drivers/mock1/Makefile drivers/mock2/Makefile
	drivers/simulator/Makefile drivers/net/Makefile
	drivers/sx/Makefile drivers/sbig/Makefile
	drivers/uvc/Makefile drivers/unicap/Makefile 
	drivers/microtouch/Makefile drivers/qsi/Makefile drivers/qhy/Makefile
	man/Makefile doc/Makefile
	test/Makefile])
AC_OUTPUT

