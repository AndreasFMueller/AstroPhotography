#
# Makefile.am -- build the focus tools
#
# (c) 2018 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

bin_PROGRAMS = astrofocus 

noinst_HEADERS = ImageWriter.h

noinst_PROGRAMS = maketestimages

astrofocus_SOURCES = ImageWriter.cpp astrofocus.cpp
astrofocus_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astrofocus_LDADD = -L$(top_builddir)/lib -lastro

maketestimages_SOURCES = maketestimages.cpp
maketestimages_DEPENDENCIES = $(top_builddir)/lib/libastro.la
maketestimages_LDADD = -L$(top_builddir)/lib -lastro

image:	astrofocus
	./astrofocus --debug --method=fwhm				\
		image testdata/test-002.fits tmp/eval-002.jpg

image2:	astrofocus
	./astrofocus --debug --method=fwhm				\
		--center='(1410,584)' --window=200x200 			\
		image testdata/image-20181018-00007.fits		\
		tmp/image-20181018-00007.png 2>&1 | tee test.log

test:	astrofocus
	./astrofocus --debug --method=fwhm --solver=			\
		--center='(1410,584)' --window=400x400 			\
		--prefix=tmp/test --format=png				\
		evaluate						\
		8319000 testdata/image-20181018-00001.fits 		\
		8319200 testdata/image-20181018-00002.fits 		\
		8319400 testdata/image-20181018-00003.fits 		\
		8319600 testdata/image-20181018-00004.fits 		\
		8319800 testdata/image-20181018-00005.fits 		\
		8320000 testdata/image-20181018-00006.fits 		\
		8320200 testdata/image-20181018-00007.fits 		\
		8320400 testdata/image-20181018-00008.fits 		\
		8320600 testdata/image-20181018-00009.fits 		\
		8320800 testdata/image-20181018-00010.fits 		\
		8321000 testdata/image-20181018-00011.fits		\
		2>&1 | tee test.log

full:	astrofocus
	./astrofocus --debug --method=fwhm --solver=			\
		--prefix=tmp/full --format=png				\
		evaluate						\
		8319000 testdata/image-20181018-00001.fits 		\
		8319200 testdata/image-20181018-00002.fits 		\
		8319400 testdata/image-20181018-00003.fits 		\
		8319600 testdata/image-20181018-00004.fits 		\
		8319800 testdata/image-20181018-00005.fits 		\
		8320000 testdata/image-20181018-00006.fits 		\
		8320200 testdata/image-20181018-00007.fits 		\
		8320400 testdata/image-20181018-00008.fits 		\
		8320600 testdata/image-20181018-00009.fits 		\
		8320800 testdata/image-20181018-00010.fits 		\
		8321000 testdata/image-20181018-00011.fits		\
		2>&1 | tee test.log

test2:	astrofocus
	./astrofocus --debug --method=fwhm --solver=abs			\
		--format=jpeg --prefix=tmp/eval				\
		evaluate						\
		0 testdata/test-000.fits				\
		1 testdata/test-001.fits				\
		2 testdata/test-002.fits				\
		3 testdata/test-003.fits				\
		4 testdata/test-004.fits				\
		5 testdata/test-005.fits				\
		6 testdata/test-006.fits				\
		7 testdata/test-007.fits				\
		8 testdata/test-008.fits				\
		9 testdata/test-009.fits				\
		10 testdata/test-010.fits				\
		2>&1 | tee test.log


images:	maketestimages
	rm -f testdata/test-*.fits
	./maketestimages --debug --width=512 --height=384 --stars=10	\
		--number=10 --prefix=testdata/test

solve:	astrofocus
	./astrofocus --debug --solver=abs solve				\
		45000      8.5187					\
		46000      6.6097					\
		47000      4.7758					\
		48000      3.1509					\
		49000      2.2469					\
		50000       2.858					\
		51000      4.3919					\
		52000      6.1975					\
		53000       8.095					\
		54000      10.032					\
		55000       11.99

process:	astrofocus
	./astrofocus --debug --method=fwhm --solve=abs --steps=10	\
		--exposure=10 --format=png --prefix=tmp/focus		\
		--center='(1410,584)' --window=400x400 			\
		--ccd=ccd:simulator/camera/ccd				\
		--focuser=focuser:simulator/focuser			\
		focus 45000 55000 2>&1 | tee test.log

process2:	astrofocus
	./astrofocus --debug --method=fwhm --solve=abs --steps=100	\
		--exposure=10 --format=png --prefix=tmp/focus2		\
		--center='(960,540)' --window=800x600 			\
		--ccd=ccd:simulator/camera/ccd				\
		--focuser=focuser:simulator/focuser			\
		focus 45000 55000 2>&1 | tee test.log

