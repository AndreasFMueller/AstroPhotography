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

# Checks for header files
AC_HEADER_STDC
AC_CHECK_HEADERS([stdlib.h stdio.h unistd.h math.h errno.h string.h stdarg.h libintl.h locale.h syslog.h sys/types.h sys/stat.h dirent.h fcntl.h dlfcn.h libusb-1.0/libusb.h])

# Check for libraries
AC_CHECK_LIB([m], [sqrt])
AC_CHECK_LIB([cfitsio], [ftffrw_])
AC_CHECK_LIB([dl], [dlopen])
AC_CHECK_LIB([usb-1.0], [libusb_bulk_transfer])

# check for SBIG library
AC_ARG_ENABLE(sbig,
[AS_HELP_STRING([--enable-sbig], [build the proprietary SBIG driver])],
[
AC_CHECK_HEADERS(sbigudrv.h, , [AC_MSG_ERROR([SBIG header not found])])
AC_CHECK_LIB([sbigudrv], [SBIGUnivDrvCommand], enable_sbig, [AC_MSG_ERROR([SBIG library not found])])
enable_sbig=yes
],[
enable_sbig=no
]
)
AM_CONDITIONAL(ENABLE_SBIG, test x$enable_sbig = xyes)

# check for the Unicap library
AC_ARG_ENABLE(unicap,
[AS_HELP_STRING([--enable-unicap], [build the unicap driver])],
[
AC_CHECK_HEADERS(unicap.h, , [AC_MSG_ERROR([unicap header not found])])
AC_CHECK_LIB([unicap], [unicap_open], LIBS="${LIBS} -lunicap", [AC_MSG_ERROR([unicap library not found])])
enable_unicap=yes
],[
enable_unicap=no
]
)
AM_CONDITIONAL(ENABLE_UNICAP, test x$enable_unicap = xyes)

# Checks for typedefs
AC_C_CONST

# Checks for library functions
AC_CHECK_FUNCS([memset strdup strerror trunc])
AC_FUNC_MALLOC
AC_FUNC_REALLOC

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
	drivers/Makefile drivers/mock1/Makefile drivers/mock2/Makefile
	doc/Makefile
	test/Makefile])
AC_OUTPUT

