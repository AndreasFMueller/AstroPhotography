#
# Makefile.am -- build the image processing tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

bin_PROGRAMS = imageinfo makedark makeflat calibrate \
	transform clamp logscale stack fitsheader imagerepo rescale \
	gammacorrect convolve background crop radon backprojection \
	colorbalance stars findtransform luminance unsharp color \
	colorclamp hdr

color_SOURCES = color.cpp
color_DEPENDENCIES = $(top_builddir)/lib/libastro.la
color_LDADD = -L$(top_builddir)/lib -lastro 

findtransform_SOURCES = findtransform.cpp
findtransform_DEPENDENCIES = $(top_builddir)/lib/libastro.la
findtransform_LDADD = -L$(top_builddir)/lib -lastro 

stars_SOURCES = stars.cpp
stars_DEPENDENCIES = $(top_builddir)/lib/libastro.la
stars_LDADD = -L$(top_builddir)/lib -lastro 

unsharp_SOURCES = unsharp.cpp
unsharp_DEPENDENCIES = $(top_builddir)/lib/libastro.la
unsharp_LDADD = -L$(top_builddir)/lib -lastro 

hdr_SOURCES = hdr.cpp
hdr_DEPENDENCIES = $(top_builddir)/lib/libastro.la
hdr_LDADD = -L$(top_builddir)/lib -lastro 

luminance_SOURCES = luminance.cpp
luminance_DEPENDENCIES = $(top_builddir)/lib/libastro.la
luminance_LDADD = -L$(top_builddir)/lib -lastro 

colorbalance_SOURCES = colorbalance.cpp
colorbalance_DEPENDENCIES = $(top_builddir)/lib/libastro.la
colorbalance_LDADD = -L$(top_builddir)/lib -lastro 

imageinfo_SOURCES = imageinfo.cpp
imageinfo_DEPENDENCIES = $(top_builddir)/lib/libastro.la
imageinfo_LDADD = -L$(top_builddir)/lib -lastro 

makedark_SOURCES = makedark.cpp
makedark_DEPENDENCIES = $(top_builddir)/lib/libastro.la
makedark_LDADD = -L$(top_builddir)/lib -lastro 

makeflat_SOURCES = makeflat.cpp
makeflat_DEPENDENCIES = $(top_builddir)/lib/libastro.la
makeflat_LDADD = -L$(top_builddir)/lib -lastro 

calibrate_SOURCES = calibrate.cpp
calibrate_DEPENDENCIES = $(top_builddir)/lib/libastro.la
calibrate_LDADD = -L$(top_builddir)/lib -lastro 

transform_SOURCES = transform.cpp
transform_DEPENDENCIES = $(top_builddir)/lib/libastro.la
transform_LDADD = -L$(top_builddir)/lib -lastro 

stack_SOURCES = stack.cpp
stack_DEPENDENCIES = $(top_builddir)/lib/libastro.la
stack_LDADD = -L$(top_builddir)/lib -lastro 

clamp_SOURCES = clamp.cpp
clamp_DEPENDENCIES = $(top_builddir)/lib/libastro.la
clamp_LDADD = -L$(top_builddir)/lib -lastro 

colorclamp_SOURCES = colorclamp.cpp
colorclamp_DEPENDENCIES = $(top_builddir)/lib/libastro.la
colorclamp_LDADD = -L$(top_builddir)/lib -lastro 

crop_SOURCES = crop.cpp
crop_DEPENDENCIES = $(top_builddir)/lib/libastro.la
crop_LDADD = -L$(top_builddir)/lib -lastro 

rescale_SOURCES = rescale.cpp
rescale_DEPENDENCIES = $(top_builddir)/lib/libastro.la
rescale_LDADD = -L$(top_builddir)/lib -lastro 

logscale_SOURCES = logscale.cpp
logscale_DEPENDENCIES = $(top_builddir)/lib/libastro.la
logscale_LDADD = -L$(top_builddir)/lib -lastro 

gammacorrect_SOURCES = gammacorrect.cpp
gammacorrect_DEPENDENCIES = $(top_builddir)/lib/libastro.la
gammacorrect_LDADD = -L$(top_builddir)/lib -lastro 

fitsheader_SOURCES = fitsheader.cpp
fitsheader_DEPENDENCIES = $(top_builddir)/lib/libastro.la
fitsheader_LDADD = -L$(top_builddir)/lib -lastro 

imagerepo_SOURCES = imagerepo.cpp
imagerepo_DEPENDENCIES = $(top_builddir)/lib/libastro.la
imagerepo_LDADD = -L$(top_builddir)/lib -lastro

convolve_SOURCES = convolve.cpp
convolve_DEPENDENCIES = $(top_builddir)/lib/libastro.la
convolve_LDADD = -L$(top_builddir)/lib -lastro

background_SOURCES = background.cpp
background_DEPENDENCIES = $(top_builddir)/lib/libastro.la
background_LDADD = -L$(top_builddir)/lib -lastro

radon_SOURCES = radon.cpp
radon_DEPENDENCIES = $(top_builddir)/lib/libastro.la
radon_LDADD = -L$(top_builddir)/lib -lastro

backprojection_SOURCES = backprojection.cpp
backprojection_DEPENDENCIES = $(top_builddir)/lib/libastro.la
backprojection_LDADD = -L$(top_builddir)/lib -lastro

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
