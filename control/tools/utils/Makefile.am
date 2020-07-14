#
# Makefile.am -- build the utilities
#
# (c) 2018 Prof Dr Andreas Mueller, Hochschule Rapperswil
#

bin_PROGRAMS = astrotime astrohorizon astroephemeris astrosunmoon

noinst_HEADERS = 

noinst_PROGRAMS = 

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

test:	astrotime
	./astrotime --debug 9.7531 46.9246

testhorizon:	astrohorizon
	./astrohorizon --debug Theodolite_Log_2020.01.02_11.24.29.csv

testephemeris:	astroephemeris
	./astroephemeris --time='1989-01-01 00:00:00' \
		mercury venus mars jupiter saturn uranus neptune pluto

testsunmoon:	astrosunmoon
	./astrosunmoon --time='1989-01-01 00:00:00' sun moon


