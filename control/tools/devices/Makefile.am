#
# Makefile.am -- build the tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

bin_PROGRAMS = astroscan devicemapper getimages imageloop focus focustest \
	dark flat focusing

astroscan_SOURCES = astroscan.cpp
astroscan_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astroscan_LDADD = -L$(top_builddir)/lib -lastro 

imageloop_SOURCES = imageloop.cpp
imageloop_DEPENDENCIES = $(top_builddir)/lib/libastro.la
imageloop_LDADD = -L$(top_builddir)/lib -lastro 

getimages_SOURCES = getimages.cpp
getimages_DEPENDENCIES = $(top_builddir)/lib/libastro.la 
getimages_LDADD = -L$(top_builddir)/lib -lastro @SBIGLIBRARY@ 

dark_SOURCES = dark.cpp
dark_DEPENDENCIES = $(top_builddir)/lib/libastro.la
dark_LDADD = -L$(top_builddir)/lib -lastro 

flat_SOURCES = flat.cpp
flat_DEPENDENCIES = $(top_builddir)/lib/libastro.la
flat_LDADD = -L$(top_builddir)/lib -lastro 

focus_SOURCES = focus.cpp
focus_DEPENDENCIES = $(top_builddir)/lib/libastro.la
focus_LDADD = -L$(top_builddir)/lib -lastro @SBIGLIBRARY@ 

focustest_SOURCES = focustest.cpp
focustest_DEPENDENCIES = $(top_builddir)/lib/libastro.la
focustest_LDADD = -L$(top_builddir)/lib -lastro @SBIGLIBRARY@ 

focusing_SOURCES = focusing.cpp
focusing_DEPENDENCIES = $(top_builddir)/lib/libastro.la
focusing_LDADD = -L$(top_builddir)/lib -lastro 

devicemapper_SOURCES = devicemapper.cpp
devicemapper_DEPENDENCIES = $(top_builddir)/lib/libastro.la
devicemapper_LDADD = -L$(top_builddir)/lib -lastro

# full night with the Starlight Express Oculus all sky camera.
looptest:	imageloop
	./imageloop -d -p 60 -t -m sx -n 0 -e 0.01 -M 20000 -o cat -a \
		-L 8.494 -l 46.318 -N -F -w 1040 -x 176 \
		-P ./convertimage.sh -Q ./makemovie.sh

focusingtest:	focusing
	./focusing -d -w 64 -h 64 -x 350 -y 255 -e 2 2>&1 | tee focusing.log

