#
# Makefile.am -- build the image processing tools
#
# (c) 2012 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

bin_PROGRAMS = imageinfo makedark makeflat calibrate \
	transform clamp logscale imagestack fitsheader imagerepo rescale \
	gammacorrect convolve background crop radon radoni backprojection \
	colorbalance stars findtransform luminance unsharp color \
	colorclamp hdr destar jpg2fits png2fits nan rgb2xyz psf \
	deconvolve haar listnan abinspect

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

abinspect_SOURCES = abinspect.cpp
abinspect_DEPENDENCIES = $(top_builddir)/lib/libastro.la
abinspect_LDADD = -L$(top_builddir)/lib -lastro

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

biasrepo = /usr/local/astrophoto/ferdinand/repositories/20250103
biastest:	biastest.fits
biastest.fits:	makedark Makefile
	./makedark --debug --badpixels --absolute=100 			\
		 --outfile=biastest.fits 				\
		$(biasrepo)/image-20250103-00201.fits			\
		$(biasrepo)/image-20250103-00202.fits			\
		$(biasrepo)/image-20250103-00203.fits			\
		$(biasrepo)/image-20250103-00204.fits			\
		$(biasrepo)/image-20250103-00205.fits			\
		$(biasrepo)/image-20250103-00206.fits			\
		$(biasrepo)/image-20250103-00207.fits			\
		$(biasrepo)/image-20250103-00208.fits			\
		$(biasrepo)/image-20250103-00209.fits			\
		$(biasrepo)/image-20250103-00210.fits			\
		$(biasrepo)/image-20250103-00211.fits			\
		$(biasrepo)/image-20250103-00212.fits			\
		$(biasrepo)/image-20250103-00213.fits			\
		$(biasrepo)/image-20250103-00214.fits			\
		$(biasrepo)/image-20250103-00215.fits			\
		$(biasrepo)/image-20250103-00216.fits			\
		$(biasrepo)/image-20250103-00217.fits			\
		$(biasrepo)/image-20250103-00218.fits			\
		$(biasrepo)/image-20250103-00219.fits			\
		$(biasrepo)/image-20250103-00220.fits			\
		$(biasrepo)/image-20250103-00221.fits			\
		$(biasrepo)/image-20250103-00222.fits			\
		$(biasrepo)/image-20250103-00223.fits			\
		$(biasrepo)/image-20250103-00224.fits			\
		$(biasrepo)/image-20250103-00225.fits			\
		$(biasrepo)/image-20250103-00226.fits			\
		$(biasrepo)/image-20250103-00227.fits			\
		$(biasrepo)/image-20250103-00228.fits			\
		$(biasrepo)/image-20250103-00229.fits			\
		$(biasrepo)/image-20250103-00220.fits
	./listnan biastest.fits
	./fitsheader display biastest.fits

flatrepo = /usr/local/astrophoto/ferdinand/repositories/20250103
flattest:	makeflat biastest.fits
	./makeflat --debug --bias=biastest.fits 			\
		 --outfile=flattest.fits 				\
		$(flatrepo)/image-20250103-00111.fits			\
		$(flatrepo)/image-20250103-00112.fits			\
		$(flatrepo)/image-20250103-00113.fits			\
		$(flatrepo)/image-20250103-00114.fits			\
		$(flatrepo)/image-20250103-00115.fits			\
		$(flatrepo)/image-20250103-00116.fits			\
		$(flatrepo)/image-20250103-00117.fits			\
		$(flatrepo)/image-20250103-00118.fits			\
		$(flatrepo)/image-20250103-00119.fits			\
		$(flatrepo)/image-20250103-00120.fits			\
		$(flatrepo)/image-20250103-00121.fits			\
		$(flatrepo)/image-20250103-00122.fits			\
		$(flatrepo)/image-20250103-00123.fits			\
		$(flatrepo)/image-20250103-00124.fits			\
		$(flatrepo)/image-20250103-00125.fits			\
		$(flatrepo)/image-20250103-00126.fits			\
		$(flatrepo)/image-20250103-00127.fits			\
		$(flatrepo)/image-20250103-00128.fits			\
		$(flatrepo)/image-20250103-00129.fits			\
		$(flatrepo)/image-20250103-00130.fits			\
		$(flatrepo)/image-20250103-00131.fits			\
		$(flatrepo)/image-20250103-00132.fits			\
		$(flatrepo)/image-20250103-00133.fits			\
		$(flatrepo)/image-20250103-00134.fits			\
		$(flatrepo)/image-20250103-00135.fits			\
		$(flatrepo)/image-20250103-00136.fits			\
		$(flatrepo)/image-20250103-00137.fits			\
		$(flatrepo)/image-20250103-00138.fits			\
		$(flatrepo)/image-20250103-00139.fits			\
		$(flatrepo)/image-20250103-00140.fits
	./listnan flattest.fits
	./fitsheader display flattest.fits

calibraterepo = /usr/local/astrophoto/ferdinand/repositories/20250103
calibratetest:	calibrated.fits
calibrated.fits:	darktest.fits flattest.fits
	./calibrate --debug --interpolate --bayer			\
		--dark=darktest.fits --flat=flattest.fits		\
		$(calibraterepo)/image-20250103-00020.fits		\
		calibrated.fits

abtest:	abinspect
	./abinspect --debug --width=400 --height=300 jackie.fits blubb.fits
