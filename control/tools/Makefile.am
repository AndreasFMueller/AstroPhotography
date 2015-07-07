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
	focus focustest transform guider dark flat clamp logscale stack \
	focusing starcatalog buildcatalog fitsheader astroconfig imagerepo \
	devicemapper astroinstrument astromount astroproject rescale \
	gammacorrect

noinst_PROGRAMS = guiderport sdiscover spublish

imageloop_SOURCES = imageloop.cpp
imageloop_DEPENDENCIES = ../lib/libastro.la
imageloop_LDADD = -L../lib -lastro 

getimages_SOURCES = getimages.cpp
if ENABLE_CORBA
getimages_DEPENDENCIES = ../lib/libastro.la ../corba/lib/libcorbastro.la
getimages_LDADD = -L../lib -lastro -L../corba/lib -lcorbastro @SBIGLIBRARY@ 
else
getimages_DEPENDENCIES = ../lib/libastro.la 
getimages_LDADD = -L../lib -lastro @SBIGLIBRARY@ 
endif

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

focustest_SOURCES = focustest.cpp
if ENABLE_CORBA
focustest_DEPENDENCIES = ../lib/libastro.la ../corba/lib/libcorbastro.la
focustest_LDADD = -L../lib -lastro -L../corba/lib -lcorbastro @SBIGLIBRARY@ 
else
focustest_DEPENDENCIES = ../lib/libastro.la
focustest_LDADD = -L../lib -lastro @SBIGLIBRARY@ 
endif

guider_SOURCES = guider.cpp
guider_DEPENDENCIES = ../lib/libastro.la
guider_LDADD = -L../lib -lastro @SBIGLIBRARY@ 

stack_SOURCES = stack.cpp
stack_DEPENDENCIES = ../lib/libastro.la
stack_LDADD = -L../lib -lastro 

clamp_SOURCES = clamp.cpp
clamp_DEPENDENCIES = ../lib/libastro.la
clamp_LDADD = -L../lib -lastro 

rescale_SOURCES = rescale.cpp
rescale_DEPENDENCIES = ../lib/libastro.la
rescale_LDADD = -L../lib -lastro 

logscale_SOURCES = logscale.cpp
logscale_DEPENDENCIES = ../lib/libastro.la
logscale_LDADD = -L../lib -lastro 

gammacorrect_SOURCES = gammacorrect.cpp
gammacorrect_DEPENDENCIES = ../lib/libastro.la
gammacorrect_LDADD = -L../lib -lastro 

focusing_SOURCES = focusing.cpp
focusing_DEPENDENCIES = ../lib/libastro.la
focusing_LDADD = -L../lib -lastro 

starcatalog_SOURCES = starcatalog.cpp
starcatalog_DEPENDENCIES = ../lib/libastro.la
starcatalog_LDADD = -L../lib -lastro 

buildcatalog_SOURCES = buildcatalog.cpp
buildcatalog_DEPENDENCIES = ../lib/libastro.la
buildcatalog_LDADD = -L../lib -lastro 

fitsheader_SOURCES = fitsheader.cpp
fitsheader_DEPENDENCIES = ../lib/libastro.la
fitsheader_LDADD = -L../lib -lastro 

astroconfig_SOURCES = astroconfig.cpp
astroconfig_DEPENDENCIES = ../lib/libastro.la
astroconfig_LDADD = -L../lib -lastro

astroproject_SOURCES = astroproject.cpp
astroproject_DEPENDENCIES = ../lib/libastro.la
astroproject_LDADD = -L../lib -lastro

imagerepo_SOURCES = imagerepo.cpp
imagerepo_DEPENDENCIES = ../lib/libastro.la
imagerepo_LDADD = -L../lib -lastro

devicemapper_SOURCES = devicemapper.cpp
devicemapper_DEPENDENCIES = ../lib/libastro.la
devicemapper_LDADD = -L../lib -lastro

astroinstrument_SOURCES = astroinstrument.cpp
astroinstrument_DEPENDENCIES = ../lib/libastro.la
astroinstrument_LDADD = -L../lib -lastro

astromount_SOURCES = astromount.cpp
astromount_DEPENDENCIES = ../lib/libastro.la
astromount_LDADD = -L../lib -lastro

guiderport_SOURCES = guiderport.cpp
guiderport_DEPENDENCIES = ../lib/libastro.la
guiderport_LDADD = -L../lib -lastro

sdiscover_SOURCES = sdiscover.cpp
sdiscover_DEPENDENCIES = ../lib/libastro.la
sdiscover_LDADD = -L../lib -lastro

spublish_SOURCES = spublish.cpp
spublish_DEPENDENCIES = ../lib/libastro.la
spublish_LDADD = -L../lib -lastro

# The looptest shows how to call imageloop to create a video for a
# full night with the Starlight Express Oculus all sky camera.
looptest:	imageloop
	./imageloop -d -p 60 -t -m sx -n 0 -e 0.01 -M 20000 -o cat -a \
		-L 8.494 -l 46.318 -N -F -w 1040 -x 176 \
		-P ./convertimage.sh -Q ./makemovie.sh

test:	getimages
	./getimages -d -ORBInitRef NameService=corbaname::localhost \
		-m net -C 10 -e 1 -o net -p net \
		-F net:simulator/sim-focuser -f 25000

testfocus:	focustest
	./focustest -d -ORBInitRef NameService=corbaname::localhost \
		-m simulator -C 0 -c 0 -E 1 -F simulator -f sim-focuser \
		-b 16000 -e 48000 -s 100 | tee focus.csv

testfocus2:	focustest
	./focustest -d -ORBInitRef NameService=corbaname::localhost \
		-m simulator -C 0 -c 0 -E 1 -F simulator -f sim-focuser \
		-b 26000 -e 40000 -s 20 | tee focus2.csv

testfocus3:	focustest
	./focustest -d -ORBInitRef NameService=corbaname::localhost \
		-m simulator -C 0 -c 0 -E 4 -F simulator -f sim-focuser \
		-p 32768 -b 28000 -e 38001 -s 4 | tee focus3.csv

focusingtest:	focusing
	./focusing -d -w 64 -h 64 -x 350 -y 255 -e 2 2>&1 | tee focusing.log

catalogtest:	buildcatalog
	./buildcatalog -d \
		-h /usr/local/starcatalogs/hipparcos/hip_main.dat \
		-t /usr/local/starcatalogs/tycho2/tyc2.dat \
		-u /usr/local/starcatalogs/u4 \
		/usr/local/starcatalogs/database/stars.db
	

fitsheadertest:	fitsheader
	cp andromeda-chart.fits headertest.fits
	./fitsheader -d add headertest.fits \
		DECCENTR 41.269167 "declination of image center in degrees" \
		DECHIGHT 6.717103 "declination image with in degrees" \
		RACENTR 0.712306 "right ascension of image center in hours" \
		RAWIDTH 0.667602 "right ascension image width in hours" \
		PXLWIDTH 6.05 "width of a pixel in microns" \
		PXLHIGHT 6.05 "height of a pixel in microns" \
		FOCUS 0.135 "telescope focal length" \
		BLUBB 4711 "test integer" \
		FOO mustbestring "test string"

.PHONY:	orion
orion:
	rm -f orion.fits
	./stack --debug --output=orion.fits orion/orion*.fits 2>&1 | tee stack.log
#	./stack --debug --output=orion.fits orion/orion1.fits orion/orion2.fits 2>&1 | tee stack.log
