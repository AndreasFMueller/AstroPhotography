#
# Makefile.am -- build the utilities
#
# (c) 2018 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

bin_PROGRAMS = astrotime astrohorizon astroephemeris astrosunmoon astrojd

noinst_PROGRAMS = gridcalculator

noinst_HEADERS = 

astrojd_SOURCES = astrojd.cpp
astrojd_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astrojd_LDADD = -L$(top_builddir)/lib -lastro

astrotime_SOURCES = astrotime.cpp
astrotime_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astrotime_LDADD = -L$(top_builddir)/lib -lastro

astrohorizon_SOURCES = astrohorizon.cpp
astrohorizon_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astrohorizon_LDADD = -L$(top_builddir)/lib -lastro

astroephemeris_SOURCES = astroephemeris.cpp
astroephemeris_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astroephemeris_LDADD = -L$(top_builddir)/lib -lastro

astrosunmoon_SOURCES = astrosunmoon.cpp
astrosunmoon_DEPENDENCIES = $(top_builddir)/lib/libastro.la
astrosunmoon_LDADD = -L$(top_builddir)/lib -lastro

gridcalculator_SOURCES = gridcalculator.cpp
gridcalculator_DEPENDENCIES = $(top_builddir)/lib/libastro.la
gridcalculator_LDADD = -L$(top_builddir)/lib -lastro

test:	astrotime
	./astrotime --debug 9.7531 46.9246

testhorizon:	astrohorizon
	./astrohorizon --debug Theodolite_Log_2020.01.02_11.24.29.csv

testephemeris:	astroephemeris
	./astroephemeris --time='1989-01-01 00:00:00' \
		mercury venus mars jupiter saturn uranus neptune pluto

testsunmoon:	astrosunmoon
	./astrosunmoon --time='1989-01-01 00:00:00' sun moon

testgrid:	gridcalculator
	./gridcalculator --debug --center='+14:11:60.000 +32:30:00.000' \
		--resolution=100 --frame=1118x753 --pixels=100
