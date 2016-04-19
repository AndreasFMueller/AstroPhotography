#
# Makefile.am -- othello
#
# (c) 2015 Prof Dr Andreas Mueller, Hochschule Rapperswil
#
if ENABLE_OTHELLO

pkglib_LTLIBRARIES = othello.la

noinst_HEADERS = OthelloUtil.h OthelloLocator.h OthelloGuiderPort.h \
	OthelloFocuser.h

noinst_LTLIBRARIES = libothello.la

libothello_la_SOURCES = OthelloUtil.cpp OthelloLocator.cpp 		\
	OthelloGuiderPort.cpp OthelloFocuser.cpp
libothello_la_DEPENDENCIES = ../../lib/libastro.la
libothello_la_LIBADD = ../../lib/libastro.la

othello_la_SOURCES = OthelloUtil.cpp OthelloLocator.cpp OthelloGuiderPort.cpp \
	OthelloFocuser.cpp
othello_la_LDFLAGS = -module
othello_la_DEPENDENCIES = ../../lib/libastro.la
othello_la_LIBADD = ../../lib/libastro.la

endif
