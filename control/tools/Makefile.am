#
# Makefile.am -- build the tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#
#SUBDIRS = . test

if ENABLE_UNICAP
unicap_flies = UnicapDevice.cpp
else
unicap_flies =
endif

if ENABLE_MICROTOUCH
microtouch_files = MicroTouch.cpp
else
microtouch_files =
endif

bin_PROGRAMS = getimages imageloop imageinfo makedark makeflat calibrate \
	focus transform guider dark flat clamp logscale stack

imageloop_SOURCES = imageloop.cpp
imageloop_DEPENDENCIES = ../lib/libastro.la
imageloop_LDADD = -L../lib -lastro

getimages_SOURCES = getimages.cpp
getimages_DEPENDENCIES = ../lib/libastro.la
getimages_LDADD = -L../lib -lastro @SBIGLIBRARY@

imageinfo_SOURCES = imageinfo.cpp
imageinfo_DEPENDENCIES = ../lib/libastro.la
imageinfo_LDADD = -L../lib -lastro

makedark_SOURCES = makedark.cpp
makedark_DEPENDENCIES = ../lib/libastro.la
makedark_LDADD = -L../lib -lastro

dark_SOURCES = dark.cpp
dark_DEPENDENCIES = ../lib/libastro.la
dark_LDADD = -L../lib -lastro

flat_SOURCES = flat.cpp
flat_DEPENDENCIES = ../lib/libastro.la
flat_LDADD = -L../lib -lastro

makeflat_SOURCES = makeflat.cpp
makeflat_DEPENDENCIES = ../lib/libastro.la
makeflat_LDADD = -L../lib -lastro

calibrate_SOURCES = calibrate.cpp
calibrate_DEPENDENCIES = ../lib/libastro.la
calibrate_LDADD = -L../lib -lastro

transform_SOURCES = transform.cpp
transform_DEPENDENCIES = ../lib/libastro.la
transform_LDADD = -L../lib -lastro

focus_SOURCES = focus.cpp
focus_DEPENDENCIES = ../lib/libastro.la
focus_LDADD = -L../lib -lastro @SBIGLIBRARY@

guider_SOURCES = guider.cpp
guider_DEPENDENCIES = ../lib/libastro.la
guider_LDADD = -L../lib -lastro @SBIGLIBRARY@

stack_SOURCES = stack.cpp
stack_DEPENDENCIES = ../lib/libastro.la
stack_LDADD = -L../lib -lastro

clamp_SOURCES = clamp.cpp
clamp_DEPENDENCIES = ../lib/libastro.la
clamp_LDADD = -L../lib -lastro

logscale_SOURCES = logscale.cpp
logscale_DEPENDENCIES = ../lib/libastro.la
logscale_LDADD = -L../lib -lastro

looptest:	imageloop
	./imageloop -d -p 15 -t -m sx -n 3 -e 0.01 -E 10000 -o cat -a 
