
# Makefile.am -- build the tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

bin_PROGRAMS = astroscan devicemapper getimages imageloop focus focustest \
	dark flat focusing getstream filterwheel

astroscan_SOURCES = astroscan.cpp
astroscan_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astroscan_LDADD = -L$(top_builddir)/lib -lastro 

imageloop_SOURCES = imageloop.cpp
imageloop_DEPENDENCIES = $(top_builddir)/lib/libastro.la
imageloop_LDADD = -L$(top_builddir)/lib -lastro 

getimages_SOURCES = getimages.cpp
getimages_DEPENDENCIES = $(top_builddir)/lib/libastro.la 
getimages_LDADD = -L$(top_builddir)/lib -lastro

getstream_SOURCES = getstream.cpp
getstream_DEPENDENCIES = $(top_builddir)/lib/libastro.la 
getstream_LDADD = -L$(top_builddir)/lib -lastro

dark_SOURCES = dark.cpp
dark_DEPENDENCIES = $(top_builddir)/lib/libastro.la
dark_LDADD = -L$(top_builddir)/lib -lastro 

flat_SOURCES = flat.cpp
flat_DEPENDENCIES = $(top_builddir)/lib/libastro.la
flat_LDADD = -L$(top_builddir)/lib -lastro 

focus_SOURCES = focus.cpp
focus_DEPENDENCIES = $(top_builddir)/lib/libastro.la
focus_LDADD = -L$(top_builddir)/lib -lastro

focustest_SOURCES = focustest.cpp
focustest_DEPENDENCIES = $(top_builddir)/lib/libastro.la
focustest_LDADD = -L$(top_builddir)/lib -lastro

focusing_SOURCES = focusing.cpp
focusing_DEPENDENCIES = $(top_builddir)/lib/libastro.la
focusing_LDADD = -L$(top_builddir)/lib -lastro 

devicemapper_SOURCES = devicemapper.cpp
devicemapper_DEPENDENCIES = $(top_builddir)/lib/libastro.la
devicemapper_LDADD = -L$(top_builddir)/lib -lastro

filterwheel_SOURCES = filterwheel.cpp
filterwheel_DEPENDENCIES = $(top_builddir)/lib/libastro.la
filterwheel_LDADD = -L$(top_builddir)/lib -lastro 

# full night with the Starlight Express Oculus all sky camera.
looptest:	imageloop
	./imageloop --debug --period=60 --timestamp --exposure=0.01 	\
		--median=20000 --outdir=cat --align --night 		\
		--foreground --longitude=8.494 --latitude=46.318 	\
		--width=1040 --x-offset=176 				\
		--image-callback=./convertimage.sh 			\
		--loop-callback=./makemovie.sh

asiloop:	imageloop
	./imageloop --debug --period=60 --timestamp --exposure=0.001	\
		--mean=100 --outdir=asi --align --number=0 --foreground	\
		--image-callback=./asiconvert.sh			\
		ccd:asi/0/rgb24 2>&1 | tee test.log

focusingtest:	focusing
	./focusing -d -w 64 -h 64 -x 350 -y 255 -e 2 2>&1 | tee focusing.log

test:	filterwheel
	./filterwheel --debug filterwheel:sx/080 

