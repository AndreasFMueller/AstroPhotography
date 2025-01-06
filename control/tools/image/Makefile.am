#
# Makefile.am -- build the image processing tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
# $Id$
#

bin_PROGRAMS = imageinfo makedark makeflat calibrate \
	transform clamp logscale imagestack fitsheader imagerepo rescale \
	gammacorrect convolve background crop radon radoni backprojection \
	colorbalance stars findtransform luminance unsharp color \
	colorclamp hdr destar jpg2fits png2fits nan rgb2xyz psf \
	deconvolve haar listnan

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

imagestack_SOURCES = imagestack.cpp
imagestack_DEPENDENCIES = $(top_builddir)/lib/libastro.la
imagestack_LDADD = -L$(top_builddir)/lib -lastro 

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

radoni_SOURCES = radoni.cpp
radoni_DEPENDENCIES = $(top_builddir)/lib/libastro.la
radoni_LDADD = -L$(top_builddir)/lib -lastro

backprojection_SOURCES = backprojection.cpp
backprojection_DEPENDENCIES = $(top_builddir)/lib/libastro.la
backprojection_LDADD = -L$(top_builddir)/lib -lastro

destar_SOURCES = destar.cpp
destar_DEPENDENCIES = $(top_builddir)/lib/libastro.la
destar_LDADD = -L$(top_builddir)/lib -lastro

jpg2fits_SOURCES = jpg2fits.cpp
jpg2fits_DEPENDENCIES = $(top_builddir)/lib/libastro.la
jpg2fits_LDADD = -L$(top_builddir)/lib -lastro

png2fits_SOURCES = png2fits.cpp
png2fits_DEPENDENCIES = $(top_builddir)/lib/libastro.la
png2fits_LDADD = -L$(top_builddir)/lib -lastro

nan_SOURCES = nan.cpp
nan_DEPENDENCIES = $(top_builddir)/lib/libastro.la
nan_LDADD = -L$(top_builddir)/lib -lastro

rgb2xyz_SOURCES = rgb2xyz.cpp
rgb2xyz_DEPENDENCIES = $(top_builddir)/lib/libastro.la
rgb2xyz_LDADD = -L$(top_builddir)/lib -lastro

psf_SOURCES = psf.cpp
psf_DEPENDENCIES = $(top_builddir)/lib/libastro.la
psf_LDADD = -L$(top_builddir)/lib -lastro

deconvolve_SOURCES = deconvolve.cpp
deconvolve_DEPENDENCIES = $(top_builddir)/lib/libastro.la
deconvolve_LDADD = -L$(top_builddir)/lib -lastro

haar_SOURCES = haar.cpp
haar_DEPENDENCIES = $(top_builddir)/lib/libastro.la
haar_LDADD = -L$(top_builddir)/lib -lastro

listnan_SOURCES = listnan.cpp
listnan_DEPENDENCIES = $(top_builddir)/lib/libastro.la
listnan_LDADD = -L$(top_builddir)/lib -lastro

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
orion:	imagestack
	rm -f orion.fits
	./imagestack --debug --output=orion.fits orion/orion*.fits 2>&1 | tee stack.log
#	./imagestack --debug --output=orion.fits orion/orion1.fits orion/orion2.fits 2>&1 | tee stack.log

test:	png2fits
	rm -f t.fits
	./png2fits --debug ~/t.png t.fits

psftest:	psf
	./psf --debug --radius=50 --crop stacked2.fits psf.fits 2>&1 | tee psf.log

deconvolvetest:	deconvolve
	./deconvolve --debug --iterations=20 --constrained --prefix=vc --psf=psf.fits --method=vancittert \
		small.fits deconvolved.fits


gausstest:	deconvolve
	./deconvolve --debug --iterations=20 --constrained --prefix=fast --psf=psf.fits --method=fastvancittert \
		luminance.fits gauss.fits

thaar:	haar
	./haar --debug --force jackie.fits jackie-wavelets.fits
	convert jackie-wavelets.fits jackie-wavelets.jpg
	./haar --debug --force --inverse jackie-wavelets.fits jackie-back.fits
	convert jackie-back.fits jackie-back.jpg

darkrepo = /usr/local/astrophoto/ferdinand/repositories/20241230
darktest:	makedark
	./makedark --debug --badpixels --stddev=5 --interpolate		\
		 --outfile=darktest.fits 				\
		$(darkrepo)/image-20241230-00014.fits			\
		$(darkrepo)/image-20241230-00015.fits			\
		$(darkrepo)/image-20241230-00016.fits			\
		$(darkrepo)/image-20241230-00017.fits			\
		$(darkrepo)/image-20241230-00018.fits			\
		$(darkrepo)/image-20241230-00094.fits			\
		$(darkrepo)/image-20241230-00095.fits			\
		$(darkrepo)/image-20241230-00096.fits			\
		$(darkrepo)/image-20241230-00097.fits			\
		$(darkrepo)/image-20241230-00098.fits			\
		$(darkrepo)/image-20241230-00099.fits
	./listnan darktest.fits
	./fitsheader display darktest.fits

