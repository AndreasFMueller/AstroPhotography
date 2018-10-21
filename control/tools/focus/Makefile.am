#
# Makefile.am -- build the focus tools
#
# (c) 2018 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

bin_PROGRAMS = astrofocus 

astrofocus_SOURCES = astrofocus.cpp
astrofocus_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astrofocus_LDADD = -L$(top_builddir)/lib -lastro

test:	astrofocus
	./astrofocus --debug --method=fwhm --solver=max			\
		--center='(1364,1540)' --window=100x100 		\
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
		8321000 testdata/image-20181018-00011.fits
